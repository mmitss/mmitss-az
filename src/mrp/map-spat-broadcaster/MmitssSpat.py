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

        self.greenPhaseIndices = None
        self.yellowPhaseIndices = None
        self.redPhaseIndices = None

        self.holds_cycle1 = None
        self.holds_cycle2 = None
        
        self.forceoffs_cycle1 = None
        self.forceoffs_cycle2 = None

        self.currentCycle = [0 for phase in range(0)]

        self.scheduledHolds = None
        self.scheduledForceoffs = None

        self.vehMinEndTimeList = None
        self.vehMaxEndTimeList = None

        self.phaseClearanceTimes = [0 for phase in range(8)]
        self.previousPhase = dict({1:4, 2:1, 3:2, 4:3, 5:8, 6:5, 7:6, 8:7})

    def update_current_phase_status(self, spatBlob:Ntcip1202v2Blob):
        vehCurrStateList = super().getVehCurrStateList(spatBlob)
        self.greenPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "green"]
        self.yellowPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "yellow"]
        self.redPhaseIndices =  [phaseIndex for phaseIndex, phaseStatus in enumerate(vehCurrStateList) if phaseStatus == "red"]

    def extract_local_phase_control_schedule(self, phaseControlSchedule):
        self.holds_cycle1 = phaseControlSchedule["ScheduledActivePhaseControls"]["Cycle1"]["Holds"]
        self.holds_cycle2 = phaseControlSchedule["ScheduledActivePhaseControls"]["Cycle2"]["Holds"]
        self.scheduledForceoffs = self.holds_cycle1
        
        self.forceoffs_cycle1 = phaseControlSchedule["ScheduledActivePhaseControls"]["Cycle1"]["ForceOffs"]
        self.forceoffs_cycle2 = phaseControlSchedule["ScheduledActivePhaseControls"]["Cycle2"]["ForceOffs"]
        self.scheduledForceoffs = self.forceoffs_cycle1

        self.currentCycle_holds = [1 for phase in range(8)]
        self.currentCycle_forceoffs = [1 for phase in range(8)]

        self.isScheduleJustReceived = True
        self.isScheduleActive = True
        
    def update_local_phase_control_schedule(self):
        
        if self.isScheduleJustReceived == False:
            # Update scheduled holds:
            if self.scheduledHolds != None:
                for index in range (len(self.scheduledHolds)):
                    if (self.scheduledHolds[index] >= 0):
                        self.scheduledHolds[index] = self.scheduledHolds[index] - 1
                    elif self.currentCycle_holds == 1:
                        self.currentCycle_holds == 2
                        self.scheduledHolds[index] = self.holds_cycle2[index]

            # Update scheduled Forceoffs:
            if self.scheduledForceoffs != None:
                for index in range(len(self.scheduledForceoffs)):
                    if (self.scheduledForceoffs[index] >= 0):
                        self.scheduledForceoffs[index] = self.scheduledForceoffs[index] - 1
                    elif self.currentCycle_forceoffs == 1:
                        self.currentCycle_forceoffs = 2
                        self.scheduledForceoffs[index] = self.forceoffs_cycle2[index]




            if all(x < 0 for x in self.scheduledHolds):
                    self.scheduledHolds = None                

            if all(x < 0 for x in self.scheduledForceoffs):
                self.scheduledForceoffs = None

            # Update status of local phase control schedule:
            if ((self.scheduledHolds == None) and (self.scheduledForceoffs == None)):
                self.isScheduleActive = False

        else: self.isScheduleJustReceived = False                   

    