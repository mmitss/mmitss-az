import json
from Spat import Spat
from Ntcip1202v2Blob import Ntcip1202v2Blob

class MmitssSpat(Spat):
    def __init__(self):
        super().__init__()
        
        self.UNKNOWN_MIN_END_TIME = 36001 # Refer SAEJ2735/2016 Page 202 of 267
        self.UNKNOWN_MAX_END_TIME = 36001 # Refer SAEJ2735/2016 Page 202 of 267

        self.isScheduleActive = False
        self.scheduledHolds = None
        self.scheduledForceoffs = None
        self.greenPhaseIndices = None

        self.vehMinEndTimeList = None
        self.vehMaxEndTimeList = None

    def extract_local_phase_control_schedule(self, phaseControlSchedule:json):
        self.scheduledHolds = phaseControlSchedule["ScheduledActivePhaseControls"]["Holds"]
        self.scheduledForceoffs = phaseControlSchedule["ScheduledActivePhaseControls"]["Forceoffs"]
        self.isScheduleActive = True

    def update_current_phase_status(self, spatBlob:Ntcip1202v2Blob):
        vehCurrStateList = super().getVehCurrStateList(spatBlob)
        self.greenPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "green"]

    def update_local_phase_control_schedule(self):
        
        # Update scheduled holds:
        if self.scheduledHolds != None:
            for index in range (len(self.scheduledHolds)):
                if (self.scheduledHolds[index][0] > 0):
                    self.scheduledHolds[index][0] = self.scheduledHolds[index][0] - 1
                else:
                    self.scheduledHolds[index][0] = -1
                
                if (self.scheduledHolds[index][1] > 0):
                    self.scheduledHolds[index][1] = self.scheduledHolds[index][1] - 1
                else:
                    self.scheduledHolds[index][1] = -1

            if all(x[0] < 0 for x in self.scheduledHolds) and all(x[0] < 0 for x in self.scheduledHolds):
                    self.scheduledHolds = None

        # Update scheduled Forceoffs:
        if self.scheduledForceoffs != None:
            for index in range(len(self.scheduledForceoffs)):
                if (self.scheduledForceoffs[index] > 0):
                    self.scheduledForceoffs[index] = self.scheduledForceoffs[index] - 1
                else:
                    self.scheduledForceoffs[index] = -1
            
            if all(x < 0 for x in self.scheduledForceoffs):
                self.scheduledForceoffs = None

        # Update status of local phase control schedule:
        if ((self.scheduledHolds == None) and (self.scheduledForceoffs == None)):
            self.isScheduleActive = False

    def getVehMinEndTimeList(self, spatBlob):
        updatedVehMinEndTimeList =  [self.UNKNOWN_MIN_END_TIME for phase in range(8)]

        # Consider effect of holds:
        if self.scheduledHolds != None:
            for greenPhaseIndex in self.greenPhaseIndices:
                if ((self.scheduledHolds[greenPhaseIndex][0] <= 0) and (self.scheduledHolds[greenPhaseIndex][1] > 0)): # If the hold is active
                    updatedVehMinEndTimeList[greenPhaseIndex] = self.scheduledHolds[greenPhaseIndex][1]

        self.vehMinEndTimeList = updatedVehMinEndTimeList
        return updatedVehMinEndTimeList

    def getVehMaxEndTimeList(self, spatBlob):
        updatedVehMaxTimeList =  [self.UNKNOWN_MAX_END_TIME for phase in range(8)]

        # Consider effect of Forceoffs:
        if self.scheduledForceoffs != None:
            for greenPhaseIndex in self.greenPhaseIndices:
                if (self.scheduledForceoffs[greenPhaseIndex][0] > 0):
                    updatedVehMaxEndTimeList[greenPhaseIndex] = self.scheduledForceoffs[greenPhaseIndex][1]
        
        self.vehMaxEndTimeList = updatedVehMaxTimeList
        return updatedVehMaxTimeList