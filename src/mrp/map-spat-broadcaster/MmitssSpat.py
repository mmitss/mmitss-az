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

        self.phaseClearanceTimes = [0 for phase in range(8)]
        self.previousPhase = dict({1:4, 2:1, 3:2, 4:3, 5:8, 6:5, 7:6, 8:7})
        
        self.vehMinEndTimeList = None
        self.vehMaxEndTimeList = None

        self.startingPhases = [0,0]

        

    def extract_local_phase_control_schedule(self, phaseControlSchedule):
