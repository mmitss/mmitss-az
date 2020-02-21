import datetime
from SnmpApi import SnmpApi
import socket
import json
import Command

class SignalController:
    def __init__(self, signalControllerCommInfo:tuple):
        # Communication Parameters
        self.signalControllerCommInfo = signalControllerCommInfo
        self.snmp = SnmpApi(self.signalControllerCommInfo)
               
        self.currentTimingPlanId = 0
        self.currentTimingPlanJson = ""

    '''##############################################
                  Information Extraction
    ##############################################'''

    def getNextPhasesDict(self):
        nextPhasesStr = str(f'{(self.snmp.getNextPhases()):08b}')[::-1]
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

    
    # _TODO_: Refinement and testing is required
    def updateActiveTimingPlan(self):
        
        currentTimingPlanId = self.snmp.getActiveTimingPlanId()

        if (currentTimingPlanId != self.currentTimingPlanId):
            self.currentTimingPlanId = currentTimingPlanId
            activeTimingPlan =  dict({"TimingPlan":{
                                "PhaseNumber": self.snmp.getPhaseParameterPhaseNumber(),
                                "PedWalk": self.snmp.getPhaseParameterPedWalk(),
                                "PedClear": self.snmp.getPhaseParameterPedClear(),
                                "MinGreen": self.snmp.getPhaseParameterMinGreen(),
                                "Passage": self.snmp.getPhaseParameterPassage(),
                                "MaxGreen": self.snmp.getPhaseParameterMaxGreen(),
                                "YellowChange": self.snmp.getPhaseParameterYellowChange(),
                                "RedClear": self.snmp.getPhaseParameterRedClear(),
                                "PhaseRing": self.snmp.getPhaseParameterRing(),
            }})
            self.currentTimingPlanJson =  activeTimingPlan


'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    # Define controller's communication Info
    controllerIp = "10.12.6.17"
    controllerPort = 501
    controllerCommInfo = (controllerIp, controllerPort)

    # Create an object of SignalController class
    controller = SignalController(controllerCommInfo)
    controller.updateActiveTimingPlan()
    print(controller.currentTimingPlanJson)





