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
import time
import socket
import json
import StandardMib
import EconoliteMib
from Command import Command
from Snmp import Snmp


class SignalController:
    """
    SignalController class encapsulates all methods that interact with the signal controller. 
    Arguments: (1) An object of Snmp class, (2) Timing plan update interval (seconds), and
    (3) ntcipBackupTime (seconds)
    For example: asc = SignalController(snmp, 10)
    """
    def __init__(self, snmp:Snmp, timingPlanUpdateInterval_sec:int, ntcipBackupTime_sec:int):

        # Communication Parameters
        self.snmp = snmp
        
        self.currentTimingPlanId = 0
        self.currentTimingPlanJson = ""

        self.timingPlanUpdateInterval_sec = timingPlanUpdateInterval_sec
        self.ntcipBackupTime_sec = ntcipBackupTime_sec
        
        self.enableSpatBroadcast()
        self.updateActiveTimingPlan()

    ######################## Definition End: __init__(self, snmp:Snmp) ########################

    def enableSpatBroadcast(self):
        """
        SignalController::enableSpatBroadcast function enables the broadcasting of SPAT blob 
        from the signal controller. Signal controller broadcasts the SPAT blob to a particular
        port (default 6053) of the set server IP. This configuration can be set in MM-1-5-1
        in the controller menu. This function requires no arguments.
        """
        self.snmp.setValue(EconoliteMib.asc3ViiMessageEnable, 6)
        print("SPAT Broadcast Set Successfully!")
    ######################## Definition End: enableSpatBroadcast(self) ########################
    
    
    def getPhaseControl(self, action:int):
        command = Command(0,0,0,0)
        if action == command.CALL_VEH_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_VEHCALL))
        elif action == command.CALL_PED_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_PEDCALL))
        elif action == command.FORCEOFF_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_FORCEOFF))
        elif action == command.HOLD_VEH_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_HOLD))
        elif action == command.OMIT_VEH_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_VEH_OMIT))
        elif action == command.OMIT_PED_PHASES:
            value = int(self.snmp.getValue(StandardMib.PHASE_CONTROL_PED_OMIT))
        
        return value

    def setPhaseControl(self, action:int, phases:int):
        """
        SignalController::phaseControl funtion is responsible for setting the NTCIP phase control
        in the signal controller. 
        Arguments: 
        (1) Action:
        The action can be chosen from the command class. Available actions are:
        CALL_VEH_PHASES, CALL_PED_PHASES, FORCEOFF_PHASES, HOLD_VEH_PHASES, OMIT_VEH_PHASES, OMIT_PED_PHASES. 
        The mapping of actions to integer is available in Command class. 
        (2) Phases:
        The phases parameter is an integer representation of a bitstring for which a phase control needs 
        to be applied. For example, for placing a vehicle call on all phases, the bit string would be (11111111),
        whose integer representation is 255.
        """
        command = Command(0,0,0,0)
        phasesStr = str(f'{phases:08b}')[::-1]
        phaseList = []
        for i in range(len(phasesStr)):
            if phasesStr[i] == str(1):
                phaseList = phaseList + [i+1]
        
        if action == command.CALL_VEH_PHASES:
            self.snmp.setValue(StandardMib.PHASE_CONTROL_VEHCALL, phases)
            print("Veh-CALL " + str(phaseList) + " at time: " + str(time.time()))
        
        elif action == command.CALL_PED_PHASES:
            self.snmp.setValue(StandardMib.PHASE_CONTROL_PEDCALL, phases)
            print("Ped-CALL " + str(phaseList) + " at time: " + str(time.time()))

        elif action == command.FORCEOFF_PHASES:
            self.snmp.setValue(StandardMib.PHASE_CONTROL_FORCEOFF, phases)
            print("FORCEOFF " + str(phaseList) + " at time: " + str(time.time()))

        elif action == command.HOLD_VEH_PHASES:
            self.snmp.setValue(StandardMib.PHASE_CONTROL_HOLD, phases)
            print("HOLD " + str(phaseList) + " at time: " + str(time.time()))

        elif action == command.OMIT_VEH_PHASES:
            self.snmp.setValue(StandardMib.PHASE_CONTROL_VEH_OMIT, phases)
            print("Veh-OMIT " + str(phaseList) + " at time: " + str(time.time()))

        elif action == command.OMIT_PED_PHASES:
            self.snmp.setValue(StandardMib.PHASE_CONTROL_PED_OMIT, phases)
            print("Ped-OMIT " + str(phaseList) + " at time: " + str(time.time()))                        
        else: 
            print("Invalid action requested")
    ######################## Definition End: phaseControl(self, action, phases) ########################

    def sendCurrentAndNextPhasesDict(self, currPhaseListenerAddress:tuple, requesterAddress:tuple):
        """
        This function does three tasks:
        (1) Open a UDP socket at a port where MapSpatBroadcaster sends the information about current phases.
        (2) If the current phase is not in Green state (i.e. either red or yellow), this function calls the 
        getNextPhaseDict function. Else, next phases are set to 0.
        (3) Formulates a json string comprising the information about current phases and next phases and sends it
        to the requestor.

        Arguments:
        (1) currPhaseListenerAddress:
        This is a tuple containing IP address and port where MapSpatBroadcaster sends to information about current phases
        (2) requestorAddress: 
        This is a tuple containing IP address and port of the client who request the information about current and 
        next phases.
        """
        def getNextPhasesDict():
            """
            getNextPhaseDict funtion resquests the "next phases" in the controller through an Snmp::getValue method.
            The function requires no arguments.
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

            return nextPhasesDict
        ######################## Definition End: getNextPhasesDict() ########################

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(currPhaseListenerAddress)
        data, addr = s.recvfrom(1024)

        currentPhasesDict = json.loads(data.decode())

        if ((currentPhasesDict["currentPhases"][0]["State"]==6) and (currentPhasesDict["currentPhases"][1]["State"]==6)):
            nextPhasesDict= {"nextPhases":[0]}
        else:
            nextPhasesDict = getNextPhasesDict()

        currentAndNextPhasesDict = currentPhasesDict
        currentAndNextPhasesDict["nextPhases"] = nextPhasesDict["nextPhases"]
        currentAneNextPhasesJson = json.dumps(currentAndNextPhasesDict)
        
        s.sendto(currentAneNextPhasesJson.encode(), requesterAddress)
        s.close()
    ######################## Definition End: sendCurrentAndNextPhasesDict(self, currPhaseListenerAddress:tuple, requesterAddress:tuple): ########################

    def updateActiveTimingPlan(self):
        """
        SignalController::updateActiveTimingPlan function first checks the ID of currently active timing plan, through Snmp::getValue function.
        If the ID does not match the ID of the timing plan currently stored in the class attribute, 
        the function updates the timing plan stored in the class attribute, via different calls to Snmp::getValue function.
        This function requires no arguments.
        """
        currentTimingPlanId = int(self.snmp.getValue(EconoliteMib.CUR_TIMING_PLAN))

        if (currentTimingPlanId != self.currentTimingPlanId):
            self.currentTimingPlanId = currentTimingPlanId

            phaseNumber = [0 for phase in range(0,8)]
            for i in range(0,8):
                oid = (EconoliteMib.PHASE_PARAMETERS_PHASE_NUMBER) + '.' + str(currentTimingPlanId) + "." + str(i+1)
                phaseNumber[i] = int(self.snmp.getValue(oid))

            pedWalk = [0 for phase in range(0,8)]
            for i in range(0,8):
                oid = (EconoliteMib.PHASE_PARAMETERS_PEDWALK) + '.' + str(currentTimingPlanId) + "." + str(i+1)
                pedWalk[i] = int(self.snmp.getValue(oid))

            pedClear = [0 for phase in range(0,8)]
            for i in range(0,8):
                oid = (EconoliteMib.PHASE_PARAMETERS_PEDCLEAR) + '.' + str(currentTimingPlanId) + "." + str(i+1)
                pedClear[i] = int(self.snmp.getValue(oid))
            
            minGreen = [0 for phase in range(0,8)]
            for i in range(0,8):
                oid = (EconoliteMib.PHASE_PARAMETERS_MIN_GRN) + '.' + str(currentTimingPlanId) + "." + str(i+1)
                minGreen[i] = int(self.snmp.getValue(oid)) 
            
            passage = [0 for phase in range(0,8)]
            for i in range(0,8):
                oid = (EconoliteMib.PHASE_PARAMETERS_PASSAGE) + '.' + str(currentTimingPlanId) + "." + str(i+1)
                passage[i] = int(self.snmp.getValue(oid))*0.1
            
            maxGreen = [0 for phase in range(0,8)]
            for i in range(0,8):
                oid = (EconoliteMib.PHASE_PARAMETERS_MAX_GRN) + '.' + str(currentTimingPlanId) + "." + str(i+1)
                maxGreen[i] = int(self.snmp.getValue(oid))
            
            yellowChange = [0 for phase in range(0,8)]
            for i in range(0,8):
                oid = (EconoliteMib.PHASE_PARAMETERS_YELLOW_CHANGE) + '.' + str(currentTimingPlanId) + "." + str(i+1)
                yellowChange[i] = int(self.snmp.getValue(oid))*0.1
            
            redClear = [0 for phase in range(0,8)]
            for i in range(0,8):
                oid = (EconoliteMib.PHASE_PARAMETERS_RED_CLR) + '.' + str(currentTimingPlanId) + "." + str(i+1)
                redClear[i] = int(self.snmp.getValue(oid))*0.1

            activeTimingPlan =  dict({"TimingPlan":{
                                "PhaseNumber": phaseNumber,
                                "PedWalk": pedWalk,
                                "PedClear": pedClear,
                                "MinGreen": minGreen,
                                "Passage": passage,
                                "MaxGreen": maxGreen,
                                "YellowChange": yellowChange,
                                "RedClear": redClear
            }})

            self.currentTimingPlanJson =  json.dumps(activeTimingPlan)
            print("Current Timing Plan Updated")
        else: print("Timing plan is not changed. Hence, not updated")

    ######################## Definition End: updateActiveTimingPlan(self) ########################


'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    # Define controller's communication Info
    controllerIp = "10.12.6.17"
    controllerPort = 501
    controllerCommInfo = (controllerIp, controllerPort)

    snmp = Snmp(controllerCommInfo)
    # Create an object of SignalController class
    controller = SignalController(snmp, 10, 5)
    controller.setPhaseControl(1,3)

