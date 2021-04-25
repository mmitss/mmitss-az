import json
from Spat import Spat
from Ntcip1202v2Blob import Ntcip1202v2Blob

class MmitssSpat(Spat):
    def __init__(self):
        super().__init__()
        
        self.UNKNOWN_MIN_END_TIME = 36001 # Refer SAEJ2735/2016 Page 202 of 267
        self.UNKNOWN_MAX_END_TIME = 36001 # Refer SAEJ2735/2016 Page 202 of 267

        self.isScheduleActive = False
        self.isScheduleJustReceived = False

        self.scheduledHolds = None
        self.scheduledForceoffs = None
        self.greenPhaseIndices = None

        self.vehMinEndTimeList = None
        self.vehMaxEndTimeList = None

        self.phaseClearanceTimes = [0 for phase in range(8)]

        self.previousPhase = dict({1:4, 2:1, 3:2, 4:3, 5:8, 6:5, 7:6, 8:7})
        self.startingPhases = [0,0]

    def extract_local_phase_control_schedule(self, phaseControlSchedule:json):
        scheduledHolds = phaseControlSchedule["ScheduledActivePhaseControls"]["Holds"]
        self.scheduledHolds = scheduledHolds

        scheduledForceoffs = phaseControlSchedule["ScheduledActivePhaseControls"]["ForceOffs"]
        self.scheduledForceoffs = scheduledForceoffs

        self.isScheduleActive = True
        self.isScheduleJustReceived = True

    def update_current_phase_status(self, spatBlob:Ntcip1202v2Blob):
        vehCurrStateList = super().getVehCurrStateList(spatBlob)
        self.greenPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "green"]
        self.yellowPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "yellow"]


    def update_local_phase_control_schedule(self):
        
        if self.isScheduleJustReceived == False:
            # Update scheduled holds:
            if self.scheduledHolds != None:
                for index in range (len(self.scheduledHolds)):
                    if (self.scheduledHolds[index] >= 0):
                        self.scheduledHolds[index] = self.scheduledHolds[index] - 1

                if all(x < 0 for x in self.scheduledHolds):
                        self.scheduledHolds = None

            # Update scheduled Forceoffs:
            if self.scheduledForceoffs != None:
                for index in range(len(self.scheduledForceoffs)):
                    if (self.scheduledForceoffs[index] >= 0):
                        self.scheduledForceoffs[index] = self.scheduledForceoffs[index] - 1
                
                if all(x < 0 for x in self.scheduledForceoffs):
                    self.scheduledForceoffs = None

            # Update status of local phase control schedule:
            if ((self.scheduledHolds == None) and (self.scheduledForceoffs == None)):
                self.isScheduleActive = False

        else: self.isScheduleJustReceived = False


    def getVehMinEndTimeList(self, spatBlob):
        controllerVehMinEndTimeList = super().getVehMinEndTimeList(spatBlob)
        updatedVehMinEndTimeList =  [self.UNKNOWN_MIN_END_TIME for phase in range(8)]

        # Consider effect of holds:
        if self.scheduledHolds == None:
            if self.scheduledForceoffs != None:
                for greenPhaseIndex in self.greenPhaseIndices:
                    updatedVehMinEndTimeList[greenPhaseIndex] = 0

        else:
            for yellowPhaseIndex in self.yellowPhaseIndices:
                updatedVehMinEndTimeList[yellowPhaseIndex] = controllerVehMinEndTimeList[yellowPhaseIndex]            
            
            for greenPhaseIndex in self.greenPhaseIndices:
                if (self.scheduledHolds[greenPhaseIndex] >= 0): # If the hold is active
                    updatedVehMinEndTimeList[greenPhaseIndex] = self.scheduledHolds[greenPhaseIndex]

        self.vehMinEndTimeList = updatedVehMinEndTimeList
        return updatedVehMinEndTimeList

    def getVehMaxEndTimeList(self, spatBlob):        
        controllerVehMaxEndTimeList = super().getVehMaxEndTimeList(spatBlob)
        updatedVehMaxEndTimeList =  [self.UNKNOWN_MAX_END_TIME for phase in range(8)]

        # Consider effect of Forceoffs:
        if self.scheduledForceoffs == None:
            updatedVehMaxEndTimeList = controllerVehMaxEndTimeList

        else:
            for yellowPhaseIndex in self.yellowPhaseIndices:
                updatedVehMaxEndTimeList[yellowPhaseIndex] = controllerVehMaxEndTimeList[yellowPhaseIndex]

            for greenPhaseIndex in self.greenPhaseIndices:
                if (self.scheduledForceoffs[greenPhaseIndex] >= 0):
                    updatedVehMaxEndTimeList[greenPhaseIndex] = self.scheduledForceoffs[greenPhaseIndex]

        self.vehMaxEndTimeList = updatedVehMaxEndTimeList
        return updatedVehMaxEndTimeList

