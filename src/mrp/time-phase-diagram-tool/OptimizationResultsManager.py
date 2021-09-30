"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

OptimizationResultsManager.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
The methods available from this class are the following:
- createParamters(): Method to create the variables
- readOptimizationResultsFile():
- processPhaseDuration():
- processPhaseHeight():
- getCummulativeValue():
- getPriorityRequests():
- generateTimePhaseDiagram(): 
***************************************************************************************
"""

import numpy as np
from TimePhaseDiagramManager import TimePhaseDiagramManager

EV = 1
TRANSIT = 2
TRUCK = 3
DILEMMAZONE = 4
COORDINATION = 5

class OptimizationResultsManager:
    def __init__(self):
        self.fileName = '/nojournal/bin/OptimizationResults.txt'
        self.timePhaseDiagramManager = TimePhaseDiagramManager()
        
        self.startingPhase1, self.startingPhase2 = 0, 0
        self.init1, self.init2 = 0.0, 0.0

    def createParamters(self):
        self.phaseSequenceInRing1, self.phaseSequenceInRing2, self.phaseHeightInRing1, self.phaseHeightInRing2 = ([] for i in range(4))
        self.leftCriticalPointsRing1, self.rightCriticalPointsRing1,  self.leftCriticalPointsRing2, self.rightCriticalPointsRing2 = ([] for i in range(4))
        self.cumulativePhaseSequenceInRing1, self.cumulativePhaseSequenceInRing2, self.cumulativePhaseHeightInRing1, self.cumulativePhaseHeightInRing2 = ([] for i in range(4))
        self.cumulativeLeftCriticalPointsRing1, self.cumulativeRightCriticalPointsRing1, self.cumulativeLeftCriticalPointsRing2, self.cumulativeRightCriticalPointsRing2 = ([] for i in range(4))
        self.ETA_EV, self.ETA_Duration_EV, self.requestedPhase_EV, self.vehicleType_EV, self.delay_EV = ([] for i in range(5))
        self.ETA_Transit, self.ETA_Duration_Transit, self.requestedPhase_Transit, self.vehicleType_Transit, self.delay_Transit = ([] for i in range(5))
        self.ETA_Truck, self.ETA_Duration_Truck, self.requestedPhase_Truck, self.vehicleType_Truck, self.delay_Truck = ([] for i in range(5))
        self.ETA_Coordination, self.ETA_Duration_Coordination, self.requestedPhase_Coordination, self.vehicleType_Coordination, self.delay_Coordination = ([] for i in range(5))
        self.ETA_DilemmaZone, self.ETA_Duration_DilemmaZone, self.requestedPhase_DilemmaZone, self.vehicleType_DilemmaZone, self.delay_DilemmaZone = ([] for i in range(5))
        self.phaseDurationList = []
        
    def readOptimizationResultsFile(self):
        """
        Method to read the OptimizationResults.txt file line by line
        This method computes the phase duration of each phase for different cycles
        """
        requiredLineNo = 0
        self.createParamters()
        
        self.optimizationResultsFile = open(self.fileName, 'r')
        
        for lineIndex, line in enumerate(self.optimizationResultsFile):
            if lineIndex == 0:
                startingPhase1, startingPhase2 = line.split()
                self.startingPhase1, self.startingPhase2 = int(startingPhase1), int(startingPhase2)

            elif lineIndex == 1:
                init1, init2, elapsedGreen1, elapsedGreen2 = line.split()
                self.init1, self.init2 = float(init1), float(init2)

            elif lineIndex == 2:
                self.processPhaseDuration(line, self.leftCriticalPointsRing1, self.leftCriticalPointsRing2)
                self.processPhaseSequence()

            elif lineIndex == 3:
                self.processPhaseDuration(line, self.leftCriticalPointsRing1, self.leftCriticalPointsRing2)
                self.processPhaseSequence()

            elif lineIndex == 4:
                self.processPhaseDuration(line, self.leftCriticalPointsRing1, self.leftCriticalPointsRing2)
                self.processPhaseSequence()

            elif lineIndex == 5:
                self.processPhaseDuration(line, self.rightCriticalPointsRing1, self.rightCriticalPointsRing2)

            elif lineIndex == 6:
                self.processPhaseDuration(line, self.rightCriticalPointsRing1, self.rightCriticalPointsRing2)

            elif lineIndex == 7:
                self.processPhaseDuration(line, self.rightCriticalPointsRing1, self.rightCriticalPointsRing2)

            elif lineIndex == 14:
                noOfRequest = int(line)
                requiredLineNo = 15 + noOfRequest
                break
        
        self.optimizationResultsFile.close()
        self.getPriorityRequests(requiredLineNo)
        
        self.getCummulativeValues()
        self.generateTimePhaseDiagram()

    def processPhaseDuration(self, line, criticalPointList1, criticalPointList2):
        """
        Method to compute the phase duration of each phase per ring
        """
        list1, list2 = ([] for i in range(2))
        
        phaseDurationOfP1, phaseDurationOfP2, phaseDurationOfP3, phaseDurationOfP4, phaseDurationOfP5, phaseDurationOfP6, phaseDurationOfP7, phaseDurationOfP8 = line.split()

        self.phaseDurationList = [float(phaseDurationOfP1), float(phaseDurationOfP2), float(phaseDurationOfP3), float(phaseDurationOfP4),
                                  float(phaseDurationOfP5), float(phaseDurationOfP6), float(phaseDurationOfP7), float(phaseDurationOfP8)]

        [list1.append(value) for index, value in enumerate(self.phaseDurationList) if value > 0.0 and index < 4]
        [list2.append(value) for index, value in enumerate(self.phaseDurationList) if value > 0.0 and index >= 4]

        [criticalPointList1.append(value) for value in list1 if len(list1) > 0 and len(list2) > 0]
        [criticalPointList2.append(value) for value in list2 if len(list1) > 0 and len(list2) > 0]

    def processPhaseSequence(self):
        """
        Method to compute the phase duration of each phase per ring for each cycle
        If phase duration of a phase is greater than zero, that phase is active for that cycle. 
        For example- if phase durations of ring 1 for cycle k are[0.00, 38.90, 8.00, 47.00], phase 2,3,4 are active for cycle k in ring 1
        """
        phasesInRing1, phasesInRing2 = ([] for i in range(2))

        [phasesInRing1.append(index+1) for index, value in enumerate(self.phaseDurationList) if value > 0.0 and index < 4]
        [phasesInRing2.append(index+1) for index, value in enumerate(self.phaseDurationList) if value > 0.0 and index >= 4]

        if len(phasesInRing1) > 0 and len(phasesInRing2) > 0:
            self.phaseSequenceInRing1.extend(phasesInRing1)
            self.phaseSequenceInRing2.extend(phasesInRing2)

            self.processPhaseHeight(phasesInRing1, phasesInRing2)

    def processPhaseHeight(self, phasesInRing1, phasesInRing2):
        """
        Method to compute the y-axis height of each phase per cycle in the diagram. 
        For example if phases 1, and 2 are active in ring1 and phase 5 is only active in ring 2 for a cycle, the y-axis height for phase 5 will be the sum of the height of phase 2 and 5
        The height adjusment logic is done based on each ring-barrier group.
        """
        P11, P12, P21, P22 = ([] for i in range(4))
        phaseHeightDictionary = {}

        [P11.append(index+1)for index, value in enumerate(self.phaseDurationList)
         if value > 0.0 and index < 2]
        [P12.append(index+1)for index, value in enumerate(self.phaseDurationList)
         if value > 0.0 and index >= 2 and index < 4]
        [P21.append(index+1)for index, value in enumerate(self.phaseDurationList)
         if value > 0.0 and index >= 4 and index < 6]
        [P22.append(index+1)for index, value in enumerate(self.phaseDurationList)
         if value > 0.0 and index >= 6 and index < 8]

        if (len(P11) == len(P21)):
            for index in range(len(P11)):
                if len(P11) > 0:
                    phaseHeightDictionary[str(P11[index])] = 10

            for index in range(len(P21)):
                if len(P21) > 0:
                    phaseHeightDictionary[str(P21[index])] = 10

        elif (len(P11) < len(P21)):
            for index in range(len(P11)):
                if len(P11) > 0:
                    phaseHeightDictionary[str(P11[index])] = 20

            for index in range(len(P21)):
                if len(P21) > 0:
                    phaseHeightDictionary[str(P21[index])] = 10

        elif (len(P11) > len(P21)):
            for index in range(len(P11)):
                if len(P11) > 0:
                    phaseHeightDictionary[str(P11[index])] = 10

            for index in range(len(P21)):
                if len(P21) > 0:
                    phaseHeightDictionary[str(P21[index])] = 20

        if (len(P12) == len(P22)):
            for index in range(len(P12)):
                if len(P12) > 0:
                    phaseHeightDictionary[str(P12[index])] = 10

            for index in range(len(P22)):
                if len(P22) > 0:
                    phaseHeightDictionary[str(P22[index])] = 10

        elif (len(P12) < len(P22)):
            for index in range(len(P12)):
                if len(P12) > 0:
                    phaseHeightDictionary[str(P12[index])] = 20
            for index in range(len(P22)):
                if len(P22) > 0:
                    phaseHeightDictionary[str(P22[index])] = 10

        elif (len(P12) > len(P22)):
            for index in range(len(P12)):
                if len(P12) > 0:
                    phaseHeightDictionary[str(P12[index])] = 10
            for index in range(len(P22)):
                if len(P22) > 0:
                    phaseHeightDictionary[str(P22[index])] = 20

        for phase in phasesInRing1:
            for key, value in phaseHeightDictionary.items():
                if int(key) == phase:
                    self.phaseHeightInRing1.append(value)

        for phase in phasesInRing2:
            for key, value in phaseHeightDictionary.items():
                if int(key) == phase:
                    self.phaseHeightInRing2.append(value)

    def getCummulativeValues(self):
        """
        Compute the cumulative values for different lists
        Adjust the phase duration for positive init value (if starting phases are on yellow change or red clearance interval)
        """
        self.cumulativePhaseHeightInRing1 = np.cumsum(self.phaseHeightInRing1)
        self.cumulativePhaseHeightInRing2 = np.cumsum(self.phaseHeightInRing2)
        self.cumulativeLeftCriticalPointsRing1 = np.cumsum(self.leftCriticalPointsRing1)
        self.cumulativeRightCriticalPointsRing1 = np.cumsum(self.rightCriticalPointsRing1)
        self.cumulativeLeftCriticalPointsRing2 = np.cumsum(self.leftCriticalPointsRing2)
        self.cumulativeRightCriticalPointsRing2 = np.cumsum(self.rightCriticalPointsRing2)

        if(self.init1 > 0):
            for index, value in enumerate(self.cumulativeLeftCriticalPointsRing1):
                self.cumulativeLeftCriticalPointsRing1[index] = value + self.init1
            for index, value in enumerate(self.cumulativeRightCriticalPointsRing1):
                self.cumulativeRightCriticalPointsRing1[index] = value + self.init1

        if(self.init2 > 0):
            for index, value in enumerate(self.cumulativeLeftCriticalPointsRing2):
                self.cumulativeLeftCriticalPointsRing2[index] = value + self.init2
            for index, value in enumerate(self.cumulativeRightCriticalPointsRing2):
                self.cumulativeRightCriticalPointsRing2[index] = value + self.init2

        self.cumulativePhaseHeightInRing1 = np.insert(self.cumulativePhaseHeightInRing1, 0, 0.0)
        self.cumulativePhaseHeightInRing2 = np.insert(self.cumulativePhaseHeightInRing2, 0, 0.0)
        self.cumulativeLeftCriticalPointsRing1 = np.insert(self.cumulativeLeftCriticalPointsRing1, 0, 0.0)
        self.cumulativeRightCriticalPointsRing1 = np.insert(self.cumulativeRightCriticalPointsRing1, 0, 0.0)
        self.cumulativeLeftCriticalPointsRing2 = np.insert(self.cumulativeLeftCriticalPointsRing2, 0, 0.0)
        self.cumulativeRightCriticalPointsRing2 = np.insert(self.cumulativeRightCriticalPointsRing2, 0, 0.0)

    def getPriorityRequests(self, requiredLineNo):
        """
        Method to get the priority requests information (vehile type, ETA etc.) from the OptimizationResults.txt 
        The optimization model generates optimal schedule for two cycles. Thus, priority requests are displayed for two cycle is the diagram.
        The ETA and ETA Duration are appended twice in their respective list.
        """
        self.optimizationResultsFile = open(self.fileName, 'r')
        for i, line in enumerate(self.optimizationResultsFile):
            if i in range(15, requiredLineNo):
                reqPhase, earliestArrival, latestArrival, delay, vehicleClass = line.split()
                if int(vehicleClass) == EV:
                    self.requestedPhase_EV.append(int(reqPhase))
                    self.ETA_EV.append(float(latestArrival) - 4.0)
                    self.ETA_EV.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_EV.append(4.0)
                    self.ETA_Duration_EV.append(4.0)
                    self.vehicleType_EV.append(int(vehicleClass))
                    self.delay_EV.append(float(delay))

                elif int(vehicleClass) == TRANSIT:
                    self.requestedPhase_Transit.append(int(reqPhase))
                    self.ETA_Transit.append(float(latestArrival) - 4.0)
                    self.ETA_Transit.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_Transit.append(4.0)
                    self.ETA_Duration_Transit.append(4.0)
                    self.vehicleType_Transit.append(int(vehicleClass))
                    if (float(delay) > 0):
                        self.delay_Transit.append(
                            float(delay) - (float(latestArrival) - float(earliestArrival) - 4.0))

                elif int(vehicleClass) == TRUCK:
                    self.requestedPhase_Truck.append(int(reqPhase))
                    self.ETA_Truck.append(float(latestArrival) - 4.0)
                    self.ETA_Truck.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_Truck.append(4.0)
                    self.ETA_Duration_Truck.append(4.0)
                    self.vehicleType_Truck.append(int(vehicleClass))
                    if (float(delay) > 0):
                        self.delay_Truck.append(
                            float(delay) - (float(latestArrival) - float(earliestArrival) - 4.0))

                elif int(vehicleClass) == DILEMMAZONE:
                    self.requestedPhase_DilemmaZone.append(int(reqPhase))
                    self.ETA_DilemmaZone.append(float(latestArrival) - 4.0)
                    self.ETA_DilemmaZone.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_DilemmaZone.append(4.0)
                    self.ETA_Duration_DilemmaZone.append(4.0)
                    self.vehicleType_DilemmaZone.append(int(vehicleClass))
                    self.delay_DilemmaZone.append(float(delay))

                elif int(vehicleClass) == COORDINATION:
                    self.requestedPhase_Coordination.append(int(reqPhase))
                    self.ETA_Coordination.append(float(earliestArrival))
                    self.ETA_Coordination.append(float(earliestArrival))
                    self.ETA_Duration_Coordination.append(
                        float(latestArrival) - float(earliestArrival))
                    self.ETA_Duration_Coordination.append(
                        float(latestArrival) - float(earliestArrival))
                    self.vehicleType_Coordination.append(int(vehicleClass))
                    self.delay_Coordination.append(float(delay))

        self.optimizationResultsFile.close()

    def generateTimePhaseDiagram(self):
        """
        Method to call necessary functions to generate time-phase diagram for optimal solution
        """
        self.timePhaseDiagramManager.getParameters(self.cumulativeLeftCriticalPointsRing1, self.cumulativeRightCriticalPointsRing1, self.cumulativePhaseHeightInRing1, self.phaseSequenceInRing1,
                                                   self.cumulativeLeftCriticalPointsRing2, self.cumulativeRightCriticalPointsRing2, self.cumulativePhaseHeightInRing2, self.phaseSequenceInRing2,
                                                   self.ETA_EV, self.ETA_Duration_EV, self.requestedPhase_EV, self.vehicleType_EV, self.delay_EV,
                                                   self.ETA_Transit, self.ETA_Duration_Transit, self.requestedPhase_Transit, self.vehicleType_Transit, self.delay_Transit,
                                                   self.ETA_Truck, self.ETA_Duration_Truck, self.requestedPhase_Truck, self.vehicleType_Truck, self.delay_Truck,
                                                   self.ETA_Coordination, self.ETA_Duration_Coordination, self.requestedPhase_Coordination, self.vehicleType_Coordination, self.delay_Coordination,
                                                   self.ETA_DilemmaZone, self.ETA_Duration_DilemmaZone, self.requestedPhase_DilemmaZone, self.vehicleType_DilemmaZone, self.delay_DilemmaZone)
        
        self.timePhaseDiagramManager.timePhaseDiagramMethodForOptimalSolution("Ring1&2")
        
'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    optimizationResultsManager = OptimizationResultsManager()
    optimizationResultsManager.readOptimizationResultsFile()