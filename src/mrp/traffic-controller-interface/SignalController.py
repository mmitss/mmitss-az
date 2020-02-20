import datetime
from apscheduler.schedulers.background import BackgroundScheduler
from SnmpApi import SnmpApi
import StandardMib
import EconoliteMib
import socket
import json
import Command

class SignalController:
    def __init__(self, signalControllerIP:str, signalControllerSnmpPort:int, vendorId:int, ntcipBackupTime_Sec):
        # Communication Parameters
        self.signalControllerCommInfo = (signalControllerIP, signalControllerSnmpPort)
        self.snmpApi = SnmpApi(self.signalControllerCommInfo)
        
        # NTCIP Backup Time
        self.ntcipBackupTime_Sec = ntcipBackupTime_Sec
        

        # Scheduler global parameters
        self.commandId = 65534
        self.commandScheduler = [0,0]

        # Scheduler object for Ring 1
        self.commandScheduler[0] = BackgroundScheduler() 
        self.commandScheduler[0].start()

        # Scheduler object for Ring 2
        self.commandScheduler[1] = BackgroundScheduler()
        self.commandScheduler[1].start()
        
        # Vendor ID and MIB
        self.vendorId = vendorId
        self.vendorMib = 0
    
        # Import vendor specific Mib. vendorId:vendor mapping => 0:Econolite
        if self.vendorId == 0:
            self.vendorMib = EconoliteMib


    '''##############################################
                  Information Extraction
    ##############################################'''

    def getNextPhases(self):
        nextPhases = int(self.snmpApi.snmpGet(StandardMib.PHASE_GROUP_STATUS_NEXT))
        return nextPhases

    def getNextPhasesDict(self):
        nextPhasesStr = str(f'{(self.getNextPhases()):08b}')[::-1]
        nextPhases = [0,0]
        # Identify first next phase
        for i in range(0,8):
            if nextPhasesStr[i] == "1":
                nextPhases[0] = i+1
                break
        # Identify first next phase
        for i in range(0,8):
            if nextPhasesStr[i] == "1":
                nextPhases[1] = i+1

        if nextPhases[0] == nextPhases[1]:
            nextPhasesDict= {"nextPhases":[nextPhases[0]]}
        else:
            nextPhasesDict= {"nextPhases":[nextPhases[0], nextPhases[1]]}

        return nextPhasesDict

    def sendCurrentAndNextPhasesDict(self, currPhaseListenerAddress:tuple, requesterAddress:tuple):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(currPhaseListenerAddress)
        data, addr = s.recvfrom(1024)
        currentPhasesDict = json.loads(data.decode())

        if ((currentPhasesDict["currentPhases"][0]["State"]==6) and (currentPhasesDict["currentPhases"][1]["State"]==6)):
            nextPhasesDict= {"nextPhases":[0]}
        else:
            nextPhasesDict = self.getNextPhasesDict()

        currentAndNextPhasesDict = currentPhasesDict
        currentAndNextPhasesDict["nextPhases"] = nextPhasesDict["nextPhases"]
        currentAneNextPhasesJson = json.dumps(currentAndNextPhasesDict)
        
        s.sendto(currentAneNextPhasesJson.encode(), requesterAddress)
        s.close()

    '''##############################################
                  Phase Control Methods
    ##############################################'''
    
    def omitVehPhases(self, value:int):
        return self.snmpApi.snmpSet(StandardMib.PHASE_CONTROL_VEH_OMIT, value)
    
    def omitPedPhases(self, value:int):
        return self.snmpApi.snmpSet(StandardMib.PHASE_CONTROL_PED_OMIT, value)

    def holdPhases(self, value:int):
        return self.snmpApi.snmpSet(StandardMib.PHASE_CONTROL_HOLD, value)

    def forceOffPhases(self, value:int):
        return self.snmpApi.snmpSet(StandardMib.PHASE_CONTROL_FORCEOFF, value)

    def callVehPhases(self, value:int):
        return self.snmpApi.snmpSet(StandardMib.PHASE_CONTROL_VEHCALL, value)
    
    def callPedPhases(self, value:int):
        return self.snmpApi.snmpSet(StandardMib.PHASE_CONTROL_PEDCALL, value)

    def enableSpat(self):
        return self.snmpApi.snmpSet(EconoliteMib.asc3ViiMessageEnable, 6)

    '''##############################################
              Timing Plan Extraction Methods
    ##############################################'''

    def getActiveTimingPlanId(self):
        activeTimingPlanId = int(self.snmpApi.snmpGet(self.vendorMib.CUR_TIMING_PLAN))
        return activeTimingPlanId
    
    def getPhaseParameterPhaseNumber(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_PHASE_NUMBER) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterPedWalk(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_PEDWALK) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterPedClear(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_PEDCLEAR) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterMinGreen(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_MIN_GRN) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterPassage(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_PASSAGE) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))*0.1
        return phaseParameter

    def getPhaseParameterMaxGreen(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_MAX_GRN) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterYellowChange(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_YELLOW_CHANGE) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))*0.1
        return phaseParameter

    def getPhaseParameterRedClear(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_RED_CLR) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))*0.1
        return phaseParameter

    def getPhaseParameterRing(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_RING) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpApi.snmpGet(oid))
        return phaseParameter

    def getActiveTimingPlan(self):
        activeTimingPlan =  dict({"TimingPlan":{
                            "PhaseNumber": self.getPhaseParameterPhaseNumber(),
                            "PedWalk": self.getPhaseParameterPedWalk(),
                            "PedClear": self.getPhaseParameterPedClear(),
                            "MinGreen": self.getPhaseParameterMinGreen(),
                            "Passage": self.getPhaseParameterPassage(),
                            "MaxGreen": self.getPhaseParameterMaxGreen(),
                            "YellowChange": self.getPhaseParameterYellowChange(),
                            "RedClear": self.getPhaseParameterRedClear(),
                            "PhaseRing": self.getPhaseParameterRing(),
        }})
        return activeTimingPlan

    '''##############################################
                    Scheduler Methods
    ##############################################'''

    def addCommandToSchedule(self, ring, commandObject:Command):
        if self.commandId > 65534:
            self.commandId = 0
        self.commandId = self.commandId + 1

        if (commandObject.commandType <= 1 or commandObject.commandType > 7):
            return False
        
        # Omit vehicle phases
        elif commandObject.commandType == 2:

            self.commandScheduler[ring].add_job(self.omitVehPhases, args = [commandObject.phaseInt], 
                    trigger = 'interval', 
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                    id = str(self.commandId))
            return self.commandId
        
        # Omit pedestrian phases
        elif commandObject.commandType == 3:
            self.commandScheduler[ring].add_job(self.omitPedPhases, args = [commandObject.phaseInt], 
                    trigger = 'interval',
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                    id = str(self.commandId))
            return self.commandId

        # Hold vehicle phases
        elif commandObject.commandType == 4:
            self.commandScheduler[ring].add_job(self.holdPhases, args = [commandObject.phaseInt], 
                    trigger = 'interval', 
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Forceoff vehicle phases
        elif commandObject.commandType == 5:
            self.commandScheduler[ring].add_job(self.forceOffPhases, args = [commandObject.phaseInt], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Call vehicle phases
        elif commandObject.commandType == 6:
            self.commandScheduler[ring].add_job(self.callVehPhases, args = [commandObject.phaseInt], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Call pedestrian phases
        elif commandObject.commandType == 7:
            self.commandScheduler[ring].add_job(self.callPedPhases, args = [commandObject.phaseInt], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId

    def stopCommandScheduler(self):
        for schedulerIndex in range (0,2):
            self.commandScheduler[schedulerIndex].remove_all_jobs()
            self.commandScheduler[schedulerIndex].shutdown()
        return True

    # Add methods for:
    # Getting active schedule
    # 
    




