import json
import numpy as np
from collections import Counter
from MmitssPhase import MmitssPhase

class ScheduleSpatTranslator:
    def __init__(self):
        self.reset()


    def reset(self):
        self.schedule = None
        self.clearanceTimes = None
        self.phaseRings = None

        self.phases = [MmitssPhase(phaseNo+1) for phaseNo in range(0,8)]

        self.omittedPhases = []

        self.gMaxEndTimes = [[],[]]
        self.gMinEndTimes = [[],[]]
        self.rMaxEndTimes = [[],[]]
        self.rMinEndTimes = [[],[]] 

    def get_gmax_end_times(self, phaseNo):
        return [int(command["commandStartTime"] * 10) for command in self.schedule if (command["commandType"]=="forceoff" and
                                                                                     command["commandPhase"]==phaseNo)]

    def get_gmin_end_times(self, phaseNo):
        return [int(command["commandEndTime"] * 10) for command in self.schedule if (command["commandType"]=="hold" and
                                                                                     command["commandPhase"]==phaseNo)]

    def get_rmax_end_times(self, phaseNo):        
        previousPhase = self.phases[phaseNo-1].previousPhaseNo
        previousPhaseClearanceTime = self.phases[phaseNo-1].previousPhaseClearanceTime
            
        return [int((command["commandStartTime"] + previousPhaseClearanceTime) * 10) for command in self.schedule if (command["commandType"]=="forceoff" and
                                                                                                          command["commandPhase"]==previousPhase)]

    def get_rmin_end_times(self, phaseNo):
        previousPhase = self.phases[phaseNo-1].previousPhaseNo
        previousPhaseClearanceTime = self.phases[phaseNo-1].previousPhaseClearanceTime
        
        return [int((command["commandEndTime"] + previousPhaseClearanceTime) * 10) for command in self.schedule if (command["commandType"]=="hold" and
                                                                                     command["commandPhase"]==previousPhase)]    

    def get_rminmax_end_times_cycle0(self):
        rminmaxEndTimes = [0 for phase in range(8)]

        phasesRing1 = [phaseIndex+1 for phaseIndex in range(8) if (self.phaseRings[phaseIndex]==1)]
        phasesRing2 = [phaseIndex+1 for phaseIndex in range(8) if (self.phaseRings[phaseIndex]==2)]
        
        # identify first hold start time for ring1:
        startTimeRing1 = min([command["commandStartTime"] for command in self.schedule if (command["commandPhase"] in phasesRing1 and
                                                                                           command["commandType"]=="hold")])

        startingPhaseRing1 = [command["commandPhase"] for command in self.schedule if (command["commandType"]=="hold" and  
                                                                                       np.isclose(command["commandStartTime"],startTimeRing1) and 
                                                                                       command["commandPhase"] in phasesRing1)][0]

        rminmaxEndTimes[startingPhaseRing1-1] = int(startTimeRing1*10)

        # identify first hold start time for ring1:
        startTimeRing2 = min([command["commandStartTime"] for command in self.schedule if (command["commandPhase"] in phasesRing2 and
                                                                                           command["commandType"]=="hold")])

        startingPhaseRing2 = [command["commandPhase"] for command in self.schedule if (command["commandType"]=="hold" and  
                                                                                                np.isclose(command["commandStartTime"],startTimeRing2) and 
                                                                                       command["commandPhase"] in phasesRing2)][0]

        rminmaxEndTimes[startingPhaseRing2-1] = int(startTimeRing2*10)

        return rminmaxEndTimes

        
    
        

    def get_omitted_phases(self):
        npList = list(np.unique(np.array([command["commandPhase"] for command in self.schedule if command["commandType"]=="omit_veh"])))
        return [int(element) for element in npList]
    
    def get_previous_phase_and_clearance_time(self, phaseNo:int):
        # Check if the phase exists in the schedule
        if not any(command["commandPhase"] == phaseNo for command in self.schedule):
            return False, False
        else:
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
        

    def construct_initial_spat_table(self, schedule:dict, clearanceTimes:list, phaseRings:list):
        # Parse arguments
        self.reset()
        self.schedule = schedule["Schedule"]
        self.clearanceTimes = clearanceTimes
        self.phaseRings = phaseRings
        
        # Adjust phase rings if any of them are missing
        for phaseIndex in range(len(phaseRings)):
            if phaseRings[phaseIndex] == 0:
                if phaseIndex < 4: phaseRings[phaseIndex] = 1 
                else: phaseRings[phaseIndex] = 2

        # Identify ommitted phases
        self.omittedPhases = self.get_omitted_phases()
        
        # Identify previous phases
        for phase in self.phases:
            # Get omitted phases and their omit end times:
            if phase.phaseNo in self.omittedPhases:
                phase.omit = True
        
            if phase.omit == False:
                # Get previous phases and their clearance times
                phase.previousPhaseNo, phase.previousPhaseClearanceTime = self.get_previous_phase_and_clearance_time(phase.phaseNo)

                if (phase.previousPhaseNo == False and phase.previousPhaseClearanceTime == False):
                    phase.initialGMaxTimeToEnd = [False,False]
                    phase.initialGMinTimeToEnd = [False,False]
                    phase.initialRMaxTimeToEnd = [False,False]
                    phase.initialRMinTimeToEnd = [False,False]
                    #phase.omit = True
                    self.omittedPhases += [phase.phaseNo]
                else:

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
            
    def get_schedule_spat_translation_json(self, schedule:dict, clearanceTimes:list, phaseRings:list):
        try:
            self.construct_initial_spat_table(schedule, clearanceTimes, phaseRings)
            
            spatTranslationDict = dict({
                "MsgType": "ScheduleSpatTranslation",
                "OmittedPhases": self.omittedPhases,
                "GreenStates":
                {
                    "Cycle1":
                    {
                        "MaxEndTime": self.gMaxEndTimes[0],
                        "MinEndTime": self.gMinEndTimes[0],
                    },
                    "Cycle2":
                    {
                        "MaxEndTime": self.gMaxEndTimes[1],
                        "MinEndTime": self.gMinEndTimes[1],
                    }
                },
                "RedStates":
                {
                    "Cycle1":
                    {
                        "MaxEndTime": self.rMaxEndTimes[0],
                        "MinEndTime": self.rMinEndTimes[0]
                    },
                    "Cycle2":
                    {
                        "MaxEndTime": self.rMaxEndTimes[1],
                        "MinEndTime": self.rMinEndTimes[1]
                    },
                    "Cycle0":
                    {
                        "MinMaxEndTime": self.get_rminmax_end_times_cycle0()
                    }
                }
            })

            return json.dumps(spatTranslationDict)
        except Exception as e:
            print(e)

if __name__=="__main__":
    import json
    import time

    phaseRings = [1,1,1,1,2,2,2,2]
    clearanceTimes = [5,5,0,5,5,5,0,5]
    sst = ScheduleSpatTranslator()

    with open("test/schedule1.json", 'r') as fp:
        scheduleSpatTranslationJson = sst.get_schedule_spat_translation_json(json.load(fp), clearanceTimes, phaseRings)
        print(scheduleSpatTranslationJson)
    
    with open("test/schedule2.json", 'r') as fp:
        scheduleSpatTranslationJson = sst.get_schedule_spat_translation_json(json.load(fp), clearanceTimes, phaseRings)
        print(scheduleSpatTranslationJson)