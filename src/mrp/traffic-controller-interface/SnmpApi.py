from easysnmp import Session
import StandardMib
import EconoliteMib
import time

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
    
    # Get list of phases from "value":
    def getPhaseListFromValue(self,value:int):
        phasesStr = str(f'{value:08b}')[::-1]
        phaseList = []
        for i in range(len(phasesStr)):
            if phasesStr[i] == str(1):
                phaseList = phaseList + [i+1]
        return phaseList

    def omitVehPhases(self, value:int):
        if value == 0:
            self.snmpSet(StandardMib.PHASE_CONTROL_VEH_OMIT, value)
            print("Veh-OMIT cleared at time: " + str(time.time()))
        else:
            phases = self.getPhaseListFromValue(value)
            self.snmpSet(StandardMib.PHASE_CONTROL_VEH_OMIT, value)
            print("Veh-OMIT " + str(phases) + " at time: " + str(time.time()))

    def omitPedPhases(self, value:int):
        if value == 0:
            self.snmpSet(StandardMib.PHASE_CONTROL_PED_OMIT, value)
            print("Ped-OMIT cleared at time: " + str(time.time()))
        else:
            phases = self.getPhaseListFromValue(value)
            self.snmpSet(StandardMib.PHASE_CONTROL_PED_OMIT, value)
            print("Ped-OMIT " + str(phases) + " at time: " + str(time.time()))

    def holdPhases(self, value:int):
        if value == 0:
            self.snmpSet(StandardMib.PHASE_CONTROL_HOLD, value)
            print("HOLD cleared at time: " + str(time.time()))
        else:        
            phases = self.getPhaseListFromValue(value)
            self.snmpSet(StandardMib.PHASE_CONTROL_HOLD, value)
            print("HOLD " + str(phases) + " at time: " + str(time.time()))

    def forceOffPhases(self, value:int):
        phases = self.getPhaseListFromValue(value)
        self.snmpSet(StandardMib.PHASE_CONTROL_FORCEOFF, value)
        print("FORCEOFF " + str(phases) + " at time: " + str(time.time()))

    def callVehPhases(self, value:int):
        phases = self.getPhaseListFromValue(value)
        self.snmpSet(StandardMib.PHASE_CONTROL_VEHCALL, value)
        print("Veh-CALL " + str(phases) + " at time: " + str(time.time()))
    
    def callPedPhases(self, value:int):
        phases = self.getPhaseListFromValue(value)
        self.snmpSet(StandardMib.PHASE_CONTROL_PEDCALL, value)
        print("Ped-CALL " + str(phases) + " at time: " + str(time.time()))

    def enableSpat(self):
        self.snmpSet(EconoliteMib.asc3ViiMessageEnable, 6)

    def getCurrentHoldPhases(self):
        return self.snmpGet(StandardMib.PHASE_CONTROL_HOLD)

    def getCurrentVehOmitPhases(self):
        return self.snmpGet(StandardMib.PHASE_CONTROL_VEH_OMIT)

    def getCurrentPedOmitPhases(self):
        return self.snmpGet(StandardMib.PHASE_CONTROL_PED_OMIT)

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
    print(snmp.snmpSet("1.3.6.1.4.1.1206.4.2.1.1.5.1.2.1",1))


    