from easysnmp import Session
import StandardMib
import EconoliteMib

class SnmpApi:
    def __init__(self, targetDeviceCommInfo:tuple):
        self.targetDeviceIp, self.targetDevicePort = targetDeviceCommInfo
        self.session = Session(hostname=self.targetDeviceIp, community="public", version=1, remote_port=self.targetDevicePort)
    
    '''##############################################
                  SNMP Key Methods
    ##############################################'''

    def snmpGet(self, oid:str):
        result = self.session.get(oid).value
        return result

    def snmpSet(self, oid:str, value:int):
        self.session.set(oid, value, "int")

    '''##############################################
                  Phase Control Methods
    ##############################################'''
    
    def omitVehPhases(self, value:int):
        return self.snmpSet(StandardMib.PHASE_CONTROL_VEH_OMIT, value)
    
    def omitPedPhases(self, value:int):
        return self.snmpSet(StandardMib.PHASE_CONTROL_PED_OMIT, value)

    def holdPhases(self, value:int):
        return self.snmpSet(StandardMib.PHASE_CONTROL_HOLD, value)

    def forceOffPhases(self, value:int):
        return self.snmpSet(StandardMib.PHASE_CONTROL_FORCEOFF, value)

    def callVehPhases(self, value:int):
        return self.snmpSet(StandardMib.PHASE_CONTROL_VEHCALL, value)
    
    def callPedPhases(self, value:int):
        return self.snmpSet(StandardMib.PHASE_CONTROL_PEDCALL, value)

    def enableSpat(self):
        return self.snmpSet(EconoliteMib.asc3ViiMessageEnable, 6)

    def getNextPhases(self):
        nextPhases = int(self.snmpGet(StandardMib.PHASE_GROUP_STATUS_NEXT))
        return nextPhases

    '''##############################################
              Timing Plan Extraction Methods
    ##############################################'''

    def getActiveTimingPlanId(self):
        activeTimingPlanId = int(self.snmpGet(EconoliteMib.CUR_TIMING_PLAN))
        return activeTimingPlanId
    
    def getPhaseParameterPhaseNumber(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_PHASE_NUMBER) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterPedWalk(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_PEDWALK) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterPedClear(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_PEDCLEAR) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterMinGreen(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_MIN_GRN) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterPassage(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_PASSAGE) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))*0.1
        return phaseParameter

    def getPhaseParameterMaxGreen(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_MAX_GRN) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))
        return phaseParameter

    def getPhaseParameterYellowChange(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_YELLOW_CHANGE) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))*0.1
        return phaseParameter

    def getPhaseParameterRedClear(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_RED_CLR) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))*0.1
        return phaseParameter

    def getPhaseParameterRing(self):
        phaseParameter = [0 for x in range (0,8)]
        for i in range(0,8):
            oid = (StandardMib.PHASE_PARAMETERS_RING) + '.' + str(i+1)
            phaseParameter[i] = int(self.snmpGet(oid))
        return phaseParameter


'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    
    # Define controller's communication Info
    controllerIp = "10.12.6.17"
    controllerPort = 501
    controllerCommInfo = (controllerIp, controllerPort)
    
    # Create an opject of SnmpApi class
    snmp = SnmpApi(controllerCommInfo)
    
    # Print the id of the currently active timing plan in the controller
    print(snmp.snmpGet("1.3.6.1.4.1.1206.3.5.2.1.22.0"))
    