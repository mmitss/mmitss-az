import time
from Ntcip1202v2Blob import Ntcip1202v2Blob
from Spat import Spat


UNKNOWN = 36001
RED = "red"
GREEN = "green"
YELLOW = "yellow"
PERMISSIVE = "permissive_yellow"
LEFT_TURNS = [1,3,5,7]

class MmitssSpat(Spat):
    def __init__(self, splitPhasesDict:dict):
        super().__init__()
        self.reset()
        self.splitPhasesDict = splitPhasesDict
        self.splitPhasesList = self.get_left_turn_split_phases(splitPhasesDict)



    def get_left_turn_split_phases(self, splitPhasesDict):
        splitPhasesList = [0 for phase in range(8)]

        for phaseIndex in range(8):
            splitPhaseJsonKey = str(phaseIndex+1)
            if splitPhaseJsonKey in splitPhasesDict:
                splitPhasesList[phaseIndex] = splitPhasesDict[splitPhaseJsonKey]

        return splitPhasesList


    def reset(self):
        
        # Reset all internal variable
        self.isActive = False
        
        self.greenPhaseIndices = []
        self.yellowPhaseIndices = []
        self.redPhaseIndices = []

        self.omittedPhases = []

        self.gMaxEndTimes_cycle1 = []
        self.gMaxEndTimes_cycle2 = []
        
        self.gMinEndTimes_cycle1 = []
        self.gMinEndTimes_cycle2 = []
        
        self.rMaxEndTimes_cycle1 = []
        self.rMaxEndTimes_cycle2 = []
        
        self.rMinEndTimes_cycle1 = []
        self.rMinEndTimes_cycle2 = []

        self.servedAtleastOnce = []

        self.prevTimestamp = 0
        self.timestep = 0

        self.yellowStartPhaseIndices = []

    def initialize(self, scheduleSpatTranslation:dict):

        self.reset()
        self.isActive = True

        self.omittedPhases = scheduleSpatTranslation["OmittedPhases"]

        self.gMaxEndTimes_cycle1 = scheduleSpatTranslation["GreenStates"]["Cycle1"]["MaxEndTime"]
        self.gMaxEndTimes_cycle2 = scheduleSpatTranslation["GreenStates"]["Cycle2"]["MaxEndTime"]
        
        self.gMinEndTimes_cycle1 = scheduleSpatTranslation["GreenStates"]["Cycle1"]["MinEndTime"]
        self.gMinEndTimes_cycle2 = scheduleSpatTranslation["GreenStates"]["Cycle2"]["MinEndTime"]
        
        self.rMaxEndTimes_cycle1 = scheduleSpatTranslation["RedStates"]["Cycle1"]["MaxEndTime"]
        self.rMaxEndTimes_cycle2 = scheduleSpatTranslation["RedStates"]["Cycle2"]["MaxEndTime"]
        
        self.rMinEndTimes_cycle1 = scheduleSpatTranslation["RedStates"]["Cycle1"]["MinEndTime"]
        self.rMinEndTimes_cycle2 = scheduleSpatTranslation["RedStates"]["Cycle2"]["MinEndTime"]

        self.rMinMaxEndTimes_cycle0 = scheduleSpatTranslation["RedStates"]["Cycle0"]["MinMaxEndTime"]

        self.prevTimestamp = int(time.time() * 10)

        self.firstBlob = True
        

    def update(self, spatBlob:Ntcip1202v2Blob):
        currentTime = int(time.time() * 10)
        self.timestep = currentTime - self.prevTimestamp
        self.prevTimestamp = currentTime
        
        def deduct_timestep(dSecond):
            return max(dSecond-self.timestep, 0)

        if self.firstBlob == True:
            currentStates = spatBlob.getVehCurrState()
            for phaseIndex in range(len(currentStates)):
                if currentStates[phaseIndex]==YELLOW:
                    self.yellowStartPhaseIndices += [phaseIndex]
        
            self.firstBlob = False

        else:
            if len(self.yellowStartPhaseIndices) != 0:
                currentStates = spatBlob.getVehCurrState()
                indicesToPop = []
                for phaseIndex in self.yellowStartPhaseIndices:
                    if currentStates[phaseIndex] != YELLOW:
                        indicesToPop += [phaseIndex]

                for index in indicesToPop:
                    self.yellowStartPhaseIndices.remove(index)

        # Update current states of vehicle phases
        vehCurrStateList = super().getVehCurrStateList(spatBlob)
        self.greenPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == GREEN]
        self.redPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == RED]
        self.yellowPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == YELLOW]
        self.permissivePhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == PERMISSIVE]
        for phaseIndex in self.yellowPhaseIndices:
            if phaseIndex not in self.yellowStartPhaseIndices:
                if phaseIndex+1 not in self.servedAtleastOnce:
                    self.servedAtleastOnce += [phaseIndex+1]

        # Update internal variables
        self.gMaxEndTimes_cycle1 = list(map(deduct_timestep, self.gMaxEndTimes_cycle1))
        self.gMaxEndTimes_cycle2 = list(map(deduct_timestep, self.gMaxEndTimes_cycle2))
        
        self.gMinEndTimes_cycle1 = list(map(deduct_timestep, self.gMinEndTimes_cycle1))
        self.gMinEndTimes_cycle2 = list(map(deduct_timestep, self.gMinEndTimes_cycle2))
        
        self.rMaxEndTimes_cycle1 = list(map(deduct_timestep, self.rMaxEndTimes_cycle1))
        self.rMaxEndTimes_cycle2 = list(map(deduct_timestep, self.rMaxEndTimes_cycle2))
        
        self.rMinEndTimes_cycle1 = list(map(deduct_timestep, self.rMinEndTimes_cycle1))
        self.rMinEndTimes_cycle2 = list(map(deduct_timestep, self.rMinEndTimes_cycle2))

        self.rMinMaxEndTimes_cycle0 = list(map(deduct_timestep, self.rMinMaxEndTimes_cycle0))

        # If schedule is completed, mark it as inactive
        if not(any(self.gMaxEndTimes_cycle1) or any(self.gMaxEndTimes_cycle2) or
               any(self.gMinEndTimes_cycle1) or any(self.gMinEndTimes_cycle2) or
               any(self.rMaxEndTimes_cycle1) or any(self.rMaxEndTimes_cycle2) or
               any(self.rMinEndTimes_cycle1) or any(self.rMinEndTimes_cycle2)):
               
            self.isActive = False



    def getVehMinEndTimeList(self, spatBlob:Ntcip1202v2Blob):
        vehMinEndTimeList = super().getVehMinEndTimeList(spatBlob)
        try:
            for phaseIndex in range((8)):
                if phaseIndex+1 not in self.omittedPhases: # Do the following only for non-omitted phases
                    if phaseIndex in self.greenPhaseIndices:
                        # Substitute with the value from gMinEndTimes_cycle1 if it is not yet served
                        if phaseIndex+1 not in self.servedAtleastOnce:
                            vehMinEndTimeList[phaseIndex] = self.gMinEndTimes_cycle1[phaseIndex]
                        
                        # Substitute with the value from gMinEndTimes_cycle2 if it is already served
                        else: vehMinEndTimeList[phaseIndex] = self.gMinEndTimes_cycle2[phaseIndex]

                    elif phaseIndex in self.redPhaseIndices:
                        # Substitute with the value from rMinEndTimes_cycle1 if it is not yet served
                        if phaseIndex+1 not in self.servedAtleastOnce:
                            if self.rMinMaxEndTimes_cycle0[phaseIndex] > 0:
                                vehMinEndTimeList[phaseIndex] = self.rMinMaxEndTimes_cycle0[phaseIndex]
                            else:
                                vehMinEndTimeList[phaseIndex] = self.rMinEndTimes_cycle1[phaseIndex]
                else: vehMinEndTimeList[phaseIndex] = UNKNOWN

            # Adjust permissive_left turn phases
            for phase in LEFT_TURNS:
                phaseIndex = phase - 1
                splitPhaseIndex = self.splitPhasesList[phaseIndex]-1
                if phaseIndex+1 not in self.omittedPhases:
                    if (phaseIndex in self.permissivePhaseIndices):
                        vehMinEndTimeList[phaseIndex] = vehMinEndTimeList[splitPhaseIndex]
                    elif ((phaseIndex in self.yellowPhaseIndices) and (splitPhaseIndex in self.yellowPhaseIndices)):
                        vehMinEndTimeList[phaseIndex] = vehMinEndTimeList[splitPhaseIndex]
        except:
            pass
        return vehMinEndTimeList

    def getVehMaxEndTimeList(self, spatBlob:Ntcip1202v2Blob):
        vehMaxEndTimeList = super().getVehMaxEndTimeList(spatBlob)
        try:
            for phaseIndex in range((8)):
                if phaseIndex+1 not in self.omittedPhases: # Do the following only for non-omitted phases
                    if phaseIndex in self.greenPhaseIndices:
                        # Substitute with the value from gMaxEndTimes_cycle1 if it is not yet served
                        if phaseIndex+1 not in self.servedAtleastOnce:
                            vehMaxEndTimeList[phaseIndex] = self.gMaxEndTimes_cycle1[phaseIndex]
                        
                        # Substitute with the value from gMaxEndTimes_cycle2 if it is already served
                        else: vehMaxEndTimeList[phaseIndex] = self.gMaxEndTimes_cycle2[phaseIndex]

                    elif phaseIndex in self.redPhaseIndices:
                        # Substitute with the value from rMaxEndTimes_cycle1 if it is not yet served
                        if phaseIndex+1 not in self.servedAtleastOnce:
                            if self.rMinMaxEndTimes_cycle0[phaseIndex] > 0:
                                vehMaxEndTimeList[phaseIndex] = self.rMinMaxEndTimes_cycle0[phaseIndex]                        
                            else:
                                vehMaxEndTimeList[phaseIndex] = self.rMaxEndTimes_cycle1[phaseIndex]
                        
                        # Substitute with the value from rMaxEndTimes_cycle2 if it is already served
                        else: vehMaxEndTimeList[phaseIndex] = self.rMaxEndTimes_cycle2[phaseIndex]
                else: vehMaxEndTimeList[phaseIndex] = UNKNOWN
            
            # Adjust permissive_left turn phases
            for phase in LEFT_TURNS:
                phaseIndex = phase - 1
                splitPhaseIndex = self.splitPhasesList[phaseIndex]-1
                if phaseIndex+1 not in self.omittedPhases:
                    if (phaseIndex in self.permissivePhaseIndices):
                        vehMaxEndTimeList[phaseIndex] = vehMaxEndTimeList[splitPhaseIndex]
                    elif ((phaseIndex in self.yellowPhaseIndices) and (splitPhaseIndex in self.yellowPhaseIndices)):
                        vehMaxEndTimeList[phaseIndex] = vehMaxEndTimeList[splitPhaseIndex]
        except:
            pass

        return vehMaxEndTimeList

if __name__=="__main__":
    import json

    filename = "test/scheduleSpatTranslation_nonev.json"

    with open(filename, 'r') as fp:
        scheduleSpatTranslation = json.load(fp)

    mmitssSpat = MmitssSpat()

    mmitssSpat.initialize(scheduleSpatTranslation)
    mmitssSpat.update()
    mmitssSpat.deactivate()
    pass


