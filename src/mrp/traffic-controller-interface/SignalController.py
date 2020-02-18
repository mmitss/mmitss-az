import datetime
from apscheduler.schedulers.background import BackgroundScheduler
from SnmpApi import SnmpApi
import StandardMib
import EconoliteMib

class SignalController:
    def __init__(self, signalControllerIP:str, signalControllerSnmpPort:int, vendorId:int):
        self.signalControllerCommInfo = (signalControllerIP, signalControllerSnmpPort)
        self.snmpApi = SnmpApi(self.signalControllerCommInfo)
        self.commandScheduler = BackgroundScheduler()
        self.commandScheduler.start()
        self.commandId = 65534
        self.vendorId = vendorId
        self.vendorMib = 0
        
        # Import vendor specific Mib. vendorId:vendor mapping => 0:Econolite
        if self.vendorId == 0:
            self.vendorMib = EconoliteMib

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

    def addCommandToSchedule(self, commandType, phasesTotal, secondsFromNow):
        if self.commandId > 65534:
            self.commandId = 0
        self.commandId = self.commandId + 1
        if (commandType <= 1 or commandType > 7):
            return False
        elif commandType == 2:
            self.commandScheduler.add_job(self.omitVehPhases, args = [phasesTotal], trigger = 'date', run_date=(datetime.datetime.now()+datetime.timedelta(seconds=secondsFromNow)), id = str(self.commandId))
            return self.commandId
        elif commandType == 3:
            self.commandScheduler.add_job(self.omitPedPhases, args = [phasesTotal], trigger = 'date', run_date=(datetime.datetime.now()+datetime.timedelta(seconds=secondsFromNow)), id = str(self.commandId))
            return self.commandId
        elif commandType == 4:
            self.commandScheduler.add_job(self.holdPhases, args = [phasesTotal], trigger = 'date', run_date=(datetime.datetime.now()+datetime.timedelta(seconds=secondsFromNow)), id = str(self.commandId))
            return self.commandId
        elif commandType == 5:
            self.commandScheduler.add_job(self.forceOffPhases, args = [phasesTotal], trigger = 'date', run_date=(datetime.datetime.now()+datetime.timedelta(seconds=secondsFromNow)), id = str(self.commandId))
            return self.commandId
        elif commandType == 6:
            self.commandScheduler.add_job(self.callVehPhases, args = [phasesTotal], trigger = 'date', run_date=(datetime.datetime.now()+datetime.timedelta(seconds=secondsFromNow)), id = str(self.commandId))
            return self.commandId
        elif commandType == 7:
            self.commandScheduler.add_job(self.callPedPhases, args = [phasesTotal], trigger = 'date', run_date=(datetime.datetime.now()+datetime.timedelta(seconds=secondsFromNow)), id = str(self.commandId))
            return self.commandId

    def removeCommandFromSchedule(self, commandId:str):
        self.commandScheduler.remove_job(commandId)
        
    def stopCommandScheduler(self):
        self.commandScheduler.remove_all_jobs()
        self.commandScheduler.shutdown()
        return True

    # Add methods for:
    # Getting active schedule
    # 
    




