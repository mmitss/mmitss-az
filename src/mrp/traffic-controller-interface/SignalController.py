"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

SignalController.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This class encapsulates all methods that interacti with the signal controller. The 
methods available from this class facilitate the following:
(1) Enable broadcast of SPAT blob
(2) Control phases in the signal controller
(3) Send information about current and next phases to the requestor via a combination 
of calls to Snmp::getValue funtion and listening for information from MapSpatBroadcaster
(4) Store and maintain the current active timing plan information.

***************************************************************************************
"""

import datetime
from threading import current_thread
import time
import socket
import json
import StandardMib
import importlib.util
import Command
from Snmp import Snmp
from bitstring import BitArray
from Logger import Logger

class SignalController:
    """
    provides methods to interact with the signal controller.
    """

    def __init__(self, logger:Logger): # Check if there exists an NTCIP object to read it through SNMP.
        self.logger = logger
        # Read the config file and store the configuration in an JSON object:
        configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
        config = (json.load(configFile))

        # Close the config file:
        configFile.close()

        # Create a tuple to store the communication address of the traffic signal controller:
        signalControllerIp = config["SignalController"]["IpAddress"]
        self.logger.write("Signal Controller IP Address: " + signalControllerIp)

        signalControllerNtcipPort = config["SignalController"]["NtcipPort"]
        self.logger.write("Signal Controller NTCIP Port: {}".format(signalControllerNtcipPort))
        
        self.signalController_commInfo = (signalControllerIp, signalControllerNtcipPort)

        self.vendor = config["SignalController"]["Vendor"].lower()

        self.timingPlanMibName = (config["SignalController"]["TimingPlanMib"])
        timingPlanMibFileName = self.timingPlanMibName

        if self.timingPlanMibName.lower() != "standardmib":
            # Read the filename of the Mib that can be used to extract timing plan parameters.
            spec = importlib.util.spec_from_file_location(self.timingPlanMibName, timingPlanMibFileName)
            self.timingPlanMib = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(self.timingPlanMib)
        else: self.timingPlanMib = StandardMib

        self.currentHolds = BitArray([False for phase in range(8)])
        self.currentForceoffs = BitArray([False for phase in range(8)])
        self.currentVehOmits = BitArray([False for phase in range(8)])
        self.currentPedOmits = BitArray([False for phase in range(8)])
        self.currentVehCalls = BitArray([False for phase in range(8)])
        self.currentPedCalls = BitArray([False for phase in range(8)])

        # Read the arguments into local variables:
        self.snmp = Snmp()
        self.timingPlanUpdateInterval_sec = config["SignalController"]["TimingPlanUpdateInterval_sec"]
        
        # Get NTCIP Unit Backup Time:
        ntcipBackupTimeOid = StandardMib.NTCIP_UNIT_BACKUP_TIME
        if self.vendor == "maxtime":
            ntcipBackupTimeOid = ntcipBackupTimeOid[:-2]
        
        self.logger.write("Waiting for connection with the M_SnmpEngine...")
        self.ntcipBackupTime_sec = int(self.snmp.getValue(ntcipBackupTimeOid))
        self.logger.write("Connection with M_SnmpEngine is verified")
        self.logger.write("NTCIP Unit Backup Time: {} seconds".format(self.ntcipBackupTime_sec))

        # Create a tuple to store the communication address of the socket that listens for Current and Next Phases info sent by the MapSpatBroadcaster:
        mrpIp = config["HostIp"]
        currPhaseListenerPort = config["PortNumber"]["TrafficControllerCurrPhaseListener"]
        self.currPhaseListenerAddress = (mrpIp, currPhaseListenerPort)

        # Create a tuple to store the address of consumer of the timing plan.
        mapSpatBroadcasterPort = config["PortNumber"]["PrioritySolver"]
        self.mapSpatBroadcasterAddress = (mrpIp, mapSpatBroadcasterPort)
        
        solverPort = config["PortNumber"]["PrioritySolver"]
        self.solverAddress = (mrpIp, solverPort)

        # Create a tuple to store the address of timing plan sender
        timingPlanSenderPort = config["PortNumber"]["TrafficControllerTimingPlanSender"]
        self.timingPlanSenderAddress = (mrpIp, timingPlanSenderPort)

        # Initialize current timing plan ID and corresponding JSON string
        self.currentTimingPlanId = 0
        self.currentTimingPlanJson = ""
        
        # Read the currently active timing plan and store it into a JSON string
        self.updateAndSendActiveTimingPlan()

        self.phaseControls = dict({
                                        1:{
                                            "name":"VEH_CALL",
                                            "oid": StandardMib.PHASE_CONTROL_VEHCALL,
                                            "status": self.currentVehCalls,
                                        },
                                        2:{
                                            "name":"PED_CALL",
                                            "oid": StandardMib.PHASE_CONTROL_PEDCALL,
                                            "status": self.currentPedCalls
                                        },
                                        3:{
                                            "name":"FORCEOFF",
                                            "oid": StandardMib.PHASE_CONTROL_FORCEOFF,
                                            "status": self.currentForceoffs
                                        },
                                        4:{
                                            "name":"HOLD",
                                            "oid": StandardMib.PHASE_CONTROL_HOLD,
                                            "status": self.currentHolds
                                        },
                                        5:{
                                            "name":"VEH_OMIT",
                                            "oid": StandardMib.PHASE_CONTROL_VEH_OMIT,
                                            "status": self.currentVehOmits
                                        },
                                        6:{
                                            "name":"PED_OMIT",
                                            "oid": StandardMib.PHASE_CONTROL_PED_OMIT,
                                            "status": self.currentPedOmits
                                        },
                                    })

        self.specialFunctionLocalStatus = self.getSpecialFunctionControllerStatus()
        
    ######################## Definition End: __init__(self, snmp:Snmp) ########################

    def getSpecialFunctionControllerStatus(self):
        # Get total number of available special functions:
        numTotalSpecialFunctionsOid = StandardMib.TOTAL_SPECIAL_FUNCTIONS 
        if self.vendor == "maxtime":
            numTotalSpecialFunctionsOid = numTotalSpecialFunctionsOid[:-2]
        
        totalSpecialFunctions = int(self.snmp.getValue(numTotalSpecialFunctionsOid))        
        specialFunctionStatusDict = {}
        for specialFunctionIndex in range(totalSpecialFunctions):
            specialFunctionId = specialFunctionIndex+1
            specialFunctionStatusDict[specialFunctionId] = int(self.snmp.getValue(StandardMib.SPECIAL_FUNCTION + "." + str(specialFunctionId)))
        
        return specialFunctionStatusDict


    def updateSpecialFunctionLocalStatus(self, functionId:int, status:bool):
        self.specialFunctionLocalStatus[functionId] = status


    def setSpecialFunctionControllerStatus(self, functionId:int):        
        
        partOid = StandardMib.SPECIAL_FUNCTION
        fullOid = partOid + "." + str(functionId)
        status = self.specialFunctionLocalStatus[functionId]
        self.snmp.setValue(fullOid, int(status))
        if status == True:
            self.logger.write("Special Function " + str(functionId) + " is active")
        else: self.logger.write("Special Function " + str(functionId) + " is deactivated")

    def resetAllPhaseControls(self):
        
        self.phaseControls.get(Command.CALL_VEH_PHASES)["status"] = BitArray(False for phase in range(8))
        self.phaseControls.get(Command.CALL_PED_PHASES)["status"] = BitArray(False for phase in range(8))
        self.phaseControls.get(Command.FORCEOFF_PHASES)["status"] = BitArray(False for phase in range(8))
        self.phaseControls.get(Command.HOLD_VEH_PHASES)["status"] = BitArray(False for phase in range(8))
        self.phaseControls.get(Command.OMIT_PED_PHASES)["status"] = BitArray(False for phase in range(8))
        self.phaseControls.get(Command.OMIT_VEH_PHASES)["status"] = BitArray(False for phase in range(8))

    def getPhaseControl(self, control:int) -> int:
        """
        returns the integer representation of bit-string denoting which phases are currently executing 
        the action.
        
        Arguments:
        ----------
            (1) Action ID:
                    The action can be chosen from the command class. Available actions are:
                    CALL_VEH_PHASES, CALL_PED_PHASES, FORCEOFF_PHASES, HOLD_VEH_PHASES, OMIT_VEH_PHASES, 
                    OMIT_PED_PHASES. The mapping of actions to integer is available in Command class. 
        
        Returns:
        --------
            the value (representing the bit-string that denotes the phases that are executing the required action) 
            obtained from the snmp.getValue(action:int) function.
        """

        # Create a dummy command to get the enumerations corresponding to each phase control action
        

        if control == Command.CALL_VEH_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_VEHCALL))
        elif control == Command.CALL_PED_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_PEDCALL))
        elif control == Command.FORCEOFF_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_FORCEOFF))
        elif control == Command.HOLD_VEH_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_HOLD))
        elif control == Command.OMIT_VEH_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_VEH_OMIT))
        elif control == Command.OMIT_PED_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_PED_OMIT))
        else: value = False
        
        return value
    

    def setPhaseControl(self, control:int, action:bool, phases:list, scheduleReceiptTime:int):
        phaseControlDict = self.phaseControls.get(control)
        if ((len(phases) == 0) or (len(phases) == 1 and phases[0] == 0)) :
            phaseControlDict["status"] = BitArray(False for phase in range(8))
            phaseList = []
            groupInt = phaseControlDict["status"].uint
            self.snmp.setValue(phaseControlDict["oid"], groupInt)
            self.logger.write(phaseControlDict["name"] + " " + str(phaseList) + "(Integer={})".format(groupInt) + ". Time elapsed since the receipt of the new schedule: " + str(round((time.time() - scheduleReceiptTime),1)) + " seconds")
            
        
        else:
            currentStatus = phaseControlDict["status"]
            for phase in phases:
                phaseControlDict["status"][-phase] = action # What does "-" sign do? It reverses the bitstring.
            
            groupInt = phaseControlDict["status"].uint
            self.snmp.setValue(phaseControlDict["oid"], groupInt)
            phaseList = self.snmp.getPhaseListFromBitArray(phaseControlDict["status"])
            self.logger.write(phaseControlDict["name"] + " " + str(phaseList) + " (Value={})".format(groupInt) + ". Time elapsed since the receipt of the new schedule: " + str(round((time.time() - scheduleReceiptTime),1)) + " seconds")
            

    ######################## Definition End: phaseControl(self, action, phases) ########################

    def sendCurrentAndNextPhasesDict(self, requesterAddress:tuple):
        """
        sends a dictionary containing lists of current and next phases to the requestor

        This function does three tasks:
        (1) Open a UDP socket at a port where MapSpatBroadcaster sends the information about current phases.
        (2) If the current phase is not in Green state (i.e. either red or yellow), this function calls the 
        getNextPhaseDict function. Else, next phases are set to 0.
        (3) Formulates a json string comprising the information about current phases and next phases and sends it
        to the requestor.

        Arguments:
        ----------
            (1) requestorAddress: 
                    This is a tuple containing IP address and port of the client 
                    who request the information about current and next phases.
        """


        def getNextPhasesDict() -> dict:
            """
            requests the "next phases" in the controller through an Snmp::getValue method.
                        
            Returns:
            --------
                A dictionary containing the list of next phases.
            """

            nextPhasesInt = int(self.snmp.getValue(StandardMib.PHASE_GROUP_STATUS_NEXT))
            nextPhasesStr = str(f'{(nextPhasesInt):08b}')[::-1]
            
            nextPhasesList = [0,0]
            # Identify first next phase
            for i in range(0,8):
                if nextPhasesStr[i] == "1":
                    nextPhasesList[0] = i+1
                    break
            # Identify second next phase
            for i in range(0,8):
                if nextPhasesStr[i] == "1":
                    nextPhasesList[1] = i+1

            if nextPhasesList[0] == nextPhasesList[1]:
                nextPhasesDict= {"nextPhases":[nextPhasesList[0]]}
            else:
                nextPhasesDict= {"nextPhases":[nextPhasesList[0], nextPhasesList[1]]}
            self.logger.write("Current next phases are " + str(nextPhasesList))
            return nextPhasesDict
        
        def getPhasesCallsDict() -> dict:
            """
            requests the "veh and ped calls" in the controller through an Snmp::getValue method.
                        
            Returns:
            --------
                A dictionary containing the list of next phases.
            """
            vehCallsList = []
            vehCallsInt = int(self.snmp.getValue(StandardMib.VEHICLE_CALLS))
            vehCallsStr = str(f'{(vehCallsInt):08b}')[::-1]
            for i in range(0,8):
                if vehCallsStr[i]=="1":
                    vehCallsList = vehCallsList + [i+1]
            
            pedCallsList = []
            pedCallsInt = int(self.snmp.getValue(StandardMib.PEDESTRIAN_CALLS))
            pedCallsStr = str(f'{(pedCallsInt):08b}')[::-1]
            for i in range(0,8):
                if pedCallsStr[i]=="1":
                    pedCallsList = pedCallsList + [i+1]

            phaseCallsDict = {"vehicleCalls": vehCallsList, "pedestrianCalls": pedCallsList}
            
            return phaseCallsDict

        ######################## Definition End: getNextPhasesDict() ########################

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(self.currPhaseListenerAddress)
        data, addr = s.recvfrom(1024)
        self.logger.write("Received CurrPhaseStatus from MapSpatBroadcaster: " + str(data.decode()))

        currentPhasesDict = json.loads(data.decode())

        if ((currentPhasesDict["currentPhases"][0]["State"]=="green") and (currentPhasesDict["currentPhases"][1]["State"]=="green")):
            nextPhasesDict= {"nextPhases":[0]}
        else:
            nextPhasesDict = getNextPhasesDict()

        if ((currentPhasesDict["currentPhases"][0]["State"]=="green") or (currentPhasesDict["currentPhases"][1]["State"]=="green")):
            phaseCallsDict = getPhasesCallsDict()
        else: 
            phaseCallsDict = {"vehicleCalls":[], "pedestrianCalls":[]}


        currentAndNextPhasesDict = currentPhasesDict
        currentAndNextPhasesDict["MsgType"] = "CurrNextPhaseStatus"
        currentAndNextPhasesDict["nextPhases"] = nextPhasesDict["nextPhases"]
        currentAndNextPhasesDict["vehicleCalls"] = phaseCallsDict["vehicleCalls"]
        currentAndNextPhasesDict["totalVehicleCalls"] = len(phaseCallsDict["vehicleCalls"])
        currentAndNextPhasesDict["pedestrianCalls"] = phaseCallsDict["pedestrianCalls"]
        currentAndNextPhasesDict["totalPedestrianCalls"] = len(phaseCallsDict["pedestrianCalls"])

        currentAneNextPhasesJson = json.dumps(currentAndNextPhasesDict)
        
        s.sendto(currentAneNextPhasesJson.encode(), requesterAddress)
        self.logger.write("Sent curr and NextPhasestatus to priority-request-solver:" + str(currentAneNextPhasesJson))
        s.close()
    ######################## Definition End: sendCurrentAndNextPhasesDict(self, currPhaseListenerAddress:tuple, requesterAddress:tuple): ########################

    def updateAndSendActiveTimingPlan(self):
        """
        updates the active timing plan and sends the active timing plan to PriorityRequestSolver and MapSpatBroadcaster.

        For Econolite signal controllers, this function first checks the ID of currently active timing plan, through Snmp::getValue function.
        If the ID does not match the ID of the timing plan currently stored in the class attribute, 
        the function updates the timing plan stored in the class attribute, via different calls to Snmp::getValue function.

        For other signal controllers using the standard MIB, this function does the same task but with different OIDs. 
        Note that for in the standard MIB, we can get information only about the timing plan # 1.
        """
        
        if self.vendor.lower()=="econolite" and self.timingPlanMibName.lower()=="/nojournal/bin/econolitemib.py": # This block is valid for Econolite only:
            def getActiveTimingPlanId():
                activeTimingPlanId = int(self.snmp.getValue(self.timingPlanMib.CUR_TIMING_PLAN))
                return activeTimingPlanId
            
            def getPhaseParameterPhaseNumber(activeTimingPlanId:int):
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_PHASE_NUMBER) + '.' + str(activeTimingPlanId) + "." + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterPedWalk(activeTimingPlanId:int):
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_PEDWALK) + '.' + str(activeTimingPlanId) + "." + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterPedClear(activeTimingPlanId:int):
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_PEDCLEAR) + '.' + str(activeTimingPlanId) + "." + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterMinGreen(activeTimingPlanId:int):
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_MIN_GRN) + '.' + str(activeTimingPlanId) + "." + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterPassage(activeTimingPlanId:int):
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_PASSAGE) + '.' + str(activeTimingPlanId) + "." + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))*0.1
                return phaseParameter

            def getPhaseParameterMaxGreen(activeTimingPlanId:int):
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_MAX_GRN) + '.' + str(activeTimingPlanId) + "." + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterYellowChange(activeTimingPlanId:int):
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_YELLOW_CHANGE) + '.' + str(activeTimingPlanId) + "." + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))*0.1
                return phaseParameter

            def getPhaseParameterRedClear(activeTimingPlanId:int):
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_RED_CLR) + '.' + str(activeTimingPlanId) + "." + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))*0.1
                return phaseParameter

            def getPhaseParameterRing():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (StandardMib.PHASE_PARAMETERS_RING) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            activeTimingPlanId = getActiveTimingPlanId()
            if activeTimingPlanId != self.currentTimingPlanId:
                self.currentTimingPlanId = activeTimingPlanId
                activeTimingPlan =  dict({
                                    "MsgType": "ActiveTimingPlan",
                                    "TimingPlan":{
                                    "NoOfPhase": 8,
                                    "PhaseNumber": getPhaseParameterPhaseNumber(activeTimingPlanId),
                                    "PedWalk": getPhaseParameterPedWalk(activeTimingPlanId),
                                    "PedClear": getPhaseParameterPedClear(activeTimingPlanId),
                                    "MinGreen": getPhaseParameterMinGreen(activeTimingPlanId),
                                    "Passage": getPhaseParameterPassage(activeTimingPlanId),
                                    "MaxGreen": getPhaseParameterMaxGreen(activeTimingPlanId),
                                    "YellowChange": getPhaseParameterYellowChange(activeTimingPlanId),
                                    "RedClear": getPhaseParameterRedClear(activeTimingPlanId),
                                    "PhaseRing": getPhaseParameterRing(),
                }})
                self.currentTimingPlanJson =  json.dumps(activeTimingPlan)
                s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                s.bind(self.timingPlanSenderAddress)
                s.sendto((self.currentTimingPlanJson).encode(), self.solverAddress)
                s.sendto((self.currentTimingPlanJson).encode(), self.mapSpatBroadcasterAddress)
                s.close()
                self.logger.write("Detected a new timing plan - updated the local timing plan and sent to PriorityRequestSolver and MapSpatBroadcaster")
                self.logger.write(self.currentTimingPlanJson)

        else:
            def getPhaseParameterPhaseNumber():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_PHASE_NUMBER) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterPedWalk():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_PEDWALK) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterPedClear():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_PEDCLEAR) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterMinGreen():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_MIN_GRN) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterPassage():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_PASSAGE) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))*0.1
                return phaseParameter

            def getPhaseParameterMaxGreen():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_MAX_GRN) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            def getPhaseParameterYellowChange():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_YELLOW_CHANGE) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))*0.1
                return phaseParameter

            def getPhaseParameterRedClear():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (self.timingPlanMib.PHASE_PARAMETERS_RED_CLR) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))*0.1
                return phaseParameter

            def getPhaseParameterRing():
                phaseParameter = [0 for x in range (0,8)]
                for i in range(0,8):
                    oid = (StandardMib.PHASE_PARAMETERS_RING) + str(i+1)
                    phaseParameter[i] = int(self.snmp.getValue(oid))
                return phaseParameter

            activeTimingPlan =  dict({
                                "MsgType": "ActiveTimingPlan",
                                "TimingPlan":{
                                "NoOfPhase": 8,
                                "PhaseNumber": getPhaseParameterPhaseNumber(),
                                "PedWalk": getPhaseParameterPedWalk(),
                                "PedClear": getPhaseParameterPedClear(),
                                "MinGreen": getPhaseParameterMinGreen(),
                                "Passage": getPhaseParameterPassage(),
                                "MaxGreen": getPhaseParameterMaxGreen(),
                                "YellowChange": getPhaseParameterYellowChange(),
                                "RedClear": getPhaseParameterRedClear(),
                                "PhaseRing": getPhaseParameterRing(),
            }})
            self.currentTimingPlanJson =  json.dumps(activeTimingPlan)
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.bind(self.timingPlanSenderAddress)
            s.sendto((self.currentTimingPlanJson).encode(), self.solverAddress)
            s.sendto((self.currentTimingPlanJson).encode(), self.mapSpatBroadcasterAddress)
            s.close()
            self.logger.write("Detected a new timing plan - updated the local timing plan and sent to PriorityRequestSolver and MapSpatBroadcaster")
            self.logger.write(self.currentTimingPlanJson)
            
    ######################## Definition End: updateActiveTimingPlan(self) ########################


'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":

    logger = Logger(True, False, "speedway-mountain")

    # Create an object of SignalController class
    controller = SignalController(logger)
    
    print(controller.ntcipBackupTime_sec)
