import json
import numpy as np
from collections import Counter
from MmitssPhase import MmitssPhase

UNKNOWN = 36001

class MmitssScheduleProcessor:
    def __init__(self, schedule:dict, clearanceTimes:list, phaseRings:list):
        self.isActive = True
        self.schedule = schedule["Schedule"]
        self.clearanceTimes = clearanceTimes
        self.phaseRings = phaseRings

        self.phases = [MmitssPhase(phaseNo+1) for phaseNo in range(0,8)]

        self.gMaxEndTimes = [[],[]]
        self.gMinEndTimes = [[],[]]
        self.rMaxEndTimes = [[],[]]
        self.rMinEndTimes = [[],[]]        

        self.construct_initial_spat_table()



    def construct_initial_spat_table(self):

        # Identify ommitted phases
        omittedPhases = self.get_omitted_phases()
        
        # Identify previous phases
        for phase in self.phases:
            # Get omitted phases and their omit end times:
            if phase.phaseNo in omittedPhases:
                phase.omit = True
        
            if phase.omit == False:
                # Get previous phases and their clearance times
                phase.previousPhaseNo, phase.previousPhaseClearanceTime = self.get_previous_phase_and_clearance_time(phase.phaseNo)

                # Get initial MaxEndTime when in Green
                phase.initialGMaxTimeToEnd = self.get_gmax_end_times(phase.phaseNo)

                # Get initial MinEndTime when in Green
                phase.initialGMinTimeToEnd = self.get_gmin_end_times(phase.phaseNo)

                # Get initial MaxEndTime when in Red
                phase.initialRMaxTimeToEnd = self.get_rmax_end_times(phase.phaseNo)

                # Get initial MinEndTime when in Red
                phase.initialRMinTimeToEnd = self.get_rmin_end_times(phase.phaseNo)

            for cycle in range(2):
                self.gMaxEndTimes[cycle] = [phase.initialGMaxTimeToEnd[cycle] for phase in self.phases]
                self.gMinEndTimes[cycle] = [phase.initialGMinTimeToEnd[cycle] for phase in self.phases]
                self.rMaxEndTimes[cycle] = [phase.initialRMaxTimeToEnd[cycle] for phase in self.phases]
                self.rMinEndTimes[cycle] = [phase.initialRMinTimeToEnd[cycle] for phase in self.phases]

        

    def get_gmax_end_times(self, phaseNo):
        return [command["commandStartTime"] for command in self.schedule if (command["commandType"]=="forceoff" and
                                                                                     command["commandPhase"]==phaseNo)]

    def get_gmin_end_times(self, phaseNo):
        return [command["commandEndTime"] for command in self.schedule if (command["commandType"]=="hold" and
                                                                                     command["commandPhase"]==phaseNo)]

    def get_rmax_end_times(self, phaseNo):        
        previousPhase = self.phases[phaseNo-1].previousPhaseNo
        previousPhaseClearanceTime = self.phases[phaseNo-1].previousPhaseClearanceTime
            
        return [(command["commandStartTime"]+previousPhaseClearanceTime) for command in self.schedule if (command["commandType"]=="forceoff" and
                                                                                                          command["commandPhase"]==previousPhase)]

    def get_rmin_end_times(self, phaseNo):
        previousPhase = self.phases[phaseNo-1].previousPhaseNo
        previousPhaseClearanceTime = self.phases[phaseNo-1].previousPhaseClearanceTime
        
        return [(command["commandEndTime"]+previousPhaseClearanceTime) for command in self.schedule if (command["commandType"]=="forceoff" and
                                                                                     command["commandPhase"]==previousPhase)]        
        

    def get_omitted_phases(self):
        return list(np.unique(np.array([command["commandPhase"] for command in self.schedule if command["commandType"]=="omit_veh"])))
    
    def get_previous_phase_and_clearance_time(self, phaseNo:int):
        phaseRing = self.phaseRings[phaseNo-1]
        otherPhasesInRing = [phaseIndex+1 for phaseIndex in range(8) if (self.phaseRings[phaseIndex]==phaseRing and phaseIndex+1 != phaseNo)]
        phaseLastHoldStartTime =  max([command["commandStartTime"] for 
                        command in self.schedule if (command["commandType"] == "hold" and 
                                                     command["commandPhase"]==phaseNo)])
        
        # identify the phases in the same ring that have hold start time less than or equal to the subject phase's first hold start time:
        previousHoldsInRing = [command for command in self.schedule if (command["commandType"]=="hold" and 
                                                                   command["commandStartTime"]<phaseLastHoldStartTime and 
                                                                   command["commandPhase"] in otherPhasesInRing)]

        penultimateHoldStartTime = max([command["commandStartTime"] for command in previousHoldsInRing])

        previousPhase = [command["commandPhase"] for command in previousHoldsInRing if np.isclose(command["commandStartTime"],penultimateHoldStartTime)][0]

        previousPhaseClearanceTime = self.clearanceTimes[previousPhase-1]

        return previousPhase, previousPhaseClearanceTime      
        


if __name__=="__main__":
    import json
    import time

    with open("test/schedule_nonev.json", 'r') as fp:
        phaseRings = [1,1,1,1,2,2,2,2]
        clearanceTimes = [10,9,8,7,6,5,4,3]
        startTime = time.time()
        msp = MmitssScheduleProcessor(json.load(fp), clearanceTimes, phaseRings)
    pass