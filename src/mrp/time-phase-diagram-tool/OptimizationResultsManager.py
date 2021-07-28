import numpy as np
from TimePhaseDiagramManager import TimePhaseDiagramManager


class OptimizationResultsManager:
    def __init__(self):
        self.fileName = '/nojournal/bin/OptimizationResults.txt'
        self.timePhaseDiagramManager = TimePhaseDiagramManager()
        
        self.startingPhase1, self.startingPhase2 = 0, 0
        self.init1, self.init2 = 0.0, 0.0

    def createParamters(self):
        self.phaseSequenceInRing1, self.phaseSequenceInRing2, self.phaseHeightInRing1, self.phaseHeightInRing2 = ([
        ] for i in range(4))
        self.leftCriticalPointsRing1, self.rightCriticalPointsRing1,  self.leftCriticalPointsRing2, self.rightCriticalPointsRing2 = ([
        ] for i in range(4))
        self.cumulativePhaseSequenceInRing1, self.cumulativePhaseSequenceInRing2, self.cumulativePhaseHeightInRing1, self.cumulativePhaseHeightInRing2 = ([
        ] for i in range(4))
        self.cumulativeLeftCriticalPointsRing1, self.cumulativeRightCriticalPointsRing1, self.cumulativeLeftCriticalPointsRing2, self.cumulativeRightCriticalPointsRing2 = ([
        ] for i in range(4))
        self.ETA_EV, self.ETA_Duration_EV, self.requestedPhase_EV, self.vehicleType_EV, self.delay_EV = ([
        ] for i in range(5))
        self.ETA_Transit, self.ETA_Duration_Transit, self.requestedPhase_Transit, self.vehicleType_Transit, self.delay_Transit = ([
        ] for i in range(5))
        self.ETA_Truck, self.ETA_Duration_Truck, self.requestedPhase_Truck, self.vehicleType_Truck, self.delay_Truck = ([
        ] for i in range(5))
        self.ETA_Coordination, self.ETA_Duration_Coordination, self.requestedPhase_Coordination, self.vehicleType_Coordination, self.delay_Coordination = ([
        ] for i in range(5))
        self.ETA_DilemmaZone, self.ETA_Duration_DilemmaZone, self.requestedPhase_DilemmaZone, self.vehicleType_DilemmaZone, self.delay_DilemmaZone = ([
        ] for i in range(5))
        self.phaseDurationList = []
    def readOptimizationResultsFile(self):
        """
        Method to read the OptimizationResults.txt file line by line.
        """
        requiredLineNo = 0
        self.createParamters()
        
        self.optimizationResultsFile = open(self.fileName, 'r')
        for i, line in enumerate(self.optimizationResultsFile):
            if i == 0:
                startingPhase1, startingPhase2 = line.split()
                self.startingPhase1, self.startingPhase2 = int(
                    startingPhase1), int(startingPhase2)

            elif i == 1:
                init1, init2, elapsedGreen1, elapsedGreen2 = line.split()
                self.init1, self.init2 = float(init1), float(init2)

            elif i == 2:
                self.processPhaseDuration(
                    line, self.leftCriticalPointsRing1, self.leftCriticalPointsRing2)
                self.processPhaseSequence()

            elif i == 3:
                self.processPhaseDuration(
                    line, self.leftCriticalPointsRing1, self.leftCriticalPointsRing2)
                self.processPhaseSequence()

            elif i == 4:
                self.processPhaseDuration(
                    line, self.leftCriticalPointsRing1, self.leftCriticalPointsRing2)
                self.processPhaseSequence()

            elif i == 5:
                self.processPhaseDuration(
                    line, self.rightCriticalPointsRing1, self.rightCriticalPointsRing2)

            elif i == 6:
                self.processPhaseDuration(
                    line, self.rightCriticalPointsRing1, self.rightCriticalPointsRing2)

            elif i == 7:
                self.processPhaseDuration(
                    line, self.rightCriticalPointsRing1, self.rightCriticalPointsRing2)

            elif i == 14:
                noOfRequest = int(line)
                requiredLineNo = 15 + noOfRequest
                break
        self.optimizationResultsFile.close()
        self.getPriorityRequests(requiredLineNo)
        
        # print("[{}]".format(str(round(time.time(), 4))) + " " + "critical points are ", self.leftCriticalPointsRing1, self.rightCriticalPointsRing1,
        #       self.leftCriticalPointsRing2, self.rightCriticalPointsRing2)

        self.getCummulativeValues()
        self.createTimePhaseDiagram()

    def processPhaseDuration(self, line, criticalPointList1, criticalPointList2):
        list1, list2 = ([] for i in range(2))
        phaseDurationOfP1, phaseDurationOfP2, phaseDurationOfP3, phaseDurationOfP4, phaseDurationOfP5, phaseDurationOfP6, phaseDurationOfP7, phaseDurationOfP8 = line.split()

        self.phaseDurationList = [float(phaseDurationOfP1), float(phaseDurationOfP2), float(phaseDurationOfP3), float(phaseDurationOfP4),
                                  float(phaseDurationOfP5), float(phaseDurationOfP6), float(phaseDurationOfP7), float(phaseDurationOfP8)]

        [list1.append(value) for index, value in enumerate(
            self.phaseDurationList) if value > 0.0 and index < 4]
        [list2.append(value) for index, value in enumerate(
            self.phaseDurationList) if value > 0.0 and index >= 4]

        [criticalPointList1.append(value) for value in list1 if len(
            list1) > 0 and len(list2) > 0]
        [criticalPointList2.append(value) for value in list2 if len(
            list1) > 0 and len(list2) > 0]

    def processPhaseSequence(self):
        phasesInRing1, phasesInRing2 = ([] for i in range(2))

        [phasesInRing1.append(index+1) for index, value in enumerate(
            self.phaseDurationList) if value > 0.0 and index < 4]
        [phasesInRing2.append(index+1) for index, value in enumerate(
            self.phaseDurationList) if value > 0.0 and index >= 4]

        if len(phasesInRing1) > 0 and len(phasesInRing2) > 0:
            self.phaseSequenceInRing1.extend(phasesInRing1)
            self.phaseSequenceInRing2.extend(phasesInRing2)

            self.processPhaseHeight(phasesInRing1, phasesInRing2)

    def processPhaseHeight(self, phasesInRing1, phasesInRing2):
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
        self.cumulativeLeftCriticalPointsRing1 = np.cumsum(
            self.leftCriticalPointsRing1)
        self.cumulativeRightCriticalPointsRing1 = np.cumsum(
            self.rightCriticalPointsRing1)
        self.cumulativeLeftCriticalPointsRing2 = np.cumsum(
            self.leftCriticalPointsRing2)
        self.cumulativeRightCriticalPointsRing2 = np.cumsum(
            self.rightCriticalPointsRing2)

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

        self.cumulativePhaseHeightInRing1 = np.insert(
            self.cumulativePhaseHeightInRing1, 0, 0.0)
        self.cumulativePhaseHeightInRing2 = np.insert(
            self.cumulativePhaseHeightInRing2, 0, 0.0)
        self.cumulativeLeftCriticalPointsRing1 = np.insert(
            self.cumulativeLeftCriticalPointsRing1, 0, 0.0)
        self.cumulativeRightCriticalPointsRing1 = np.insert(
            self.cumulativeRightCriticalPointsRing1, 0, 0.0)
        self.cumulativeLeftCriticalPointsRing2 = np.insert(
            self.cumulativeLeftCriticalPointsRing2, 0, 0.0)
        self.cumulativeRightCriticalPointsRing2 = np.insert(
            self.cumulativeRightCriticalPointsRing2, 0, 0.0)


        # print("\n[{}]".format(str(round(time.time(), 4))) + " " + "Cumulative critical points are ", self.cumulativeLeftCriticalPointsRing1, self.cumulativeRightCriticalPointsRing1,
        #       self.cumulativeLeftCriticalPointsRing2, self.cumulativeRightCriticalPointsRing2)

    def getPriorityRequests(self, requiredLineNo):
        """
        Method to get the priority requests information (vehile type, ETA etc.) from the OptimizationResults.txt 
        """
        self.optimizationResultsFile = open(self.fileName, 'r')
        for i, line in enumerate(self.optimizationResultsFile):

            if i in range(15, requiredLineNo):
                reqPhase, earliestArrival, latestArrival, delay, vehicleClass = line.split()
                if int(vehicleClass) == 1:
                    self.requestedPhase_EV.append(int(reqPhase))
                    self.ETA_EV.append(float(latestArrival) - 4.0)
                    self.ETA_EV.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_EV.append(4.0)
                    self.ETA_Duration_EV.append(4.0)
                    self.vehicleType_EV.append(int(vehicleClass))
                    self.delay_EV.append(float(delay))

                elif int(vehicleClass) == 2:
                    self.requestedPhase_Transit.append(int(reqPhase))
                    self.ETA_Transit.append(float(latestArrival) - 4.0)
                    self.ETA_Transit.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_Transit.append(4.0)
                    self.ETA_Duration_Transit.append(4.0)
                    self.vehicleType_Transit.append(int(vehicleClass))
                    if (float(delay) > 0):
                        self.delay_Transit.append(
                            float(delay) - (float(latestArrival) - float(earliestArrival) - 4.0))

                elif int(vehicleClass) == 3:
                    self.requestedPhase_Truck.append(int(reqPhase))
                    self.ETA_Truck.append(float(latestArrival) - 4.0)
                    self.ETA_Truck.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_Truck.append(4.0)
                    self.ETA_Duration_Truck.append(4.0)
                    self.vehicleType_Truck.append(int(vehicleClass))
                    if (float(delay) > 0):
                        self.delay_Truck.append(
                            float(delay) - (float(latestArrival) - float(earliestArrival) - 4.0))

                elif int(vehicleClass) == 4:
                    self.requestedPhase_DilemmaZone.append(int(reqPhase))
                    self.ETA_DilemmaZone.append(float(latestArrival) - 4.0)
                    self.ETA_DilemmaZone.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_DilemmaZone.append(4.0)
                    self.ETA_Duration_DilemmaZone.append(4.0)
                    self.vehicleType_DilemmaZone.append(int(vehicleClass))
                    self.delay_DilemmaZone.append(float(delay))

                elif int(vehicleClass) == 5:
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

    def createTimePhaseDiagram(self):
        self.timePhaseDiagramManager.getParameters(self.cumulativeLeftCriticalPointsRing1, self.cumulativeRightCriticalPointsRing1, self.cumulativePhaseHeightInRing1, self.phaseSequenceInRing1,
                                                   self.cumulativeLeftCriticalPointsRing2, self.cumulativeRightCriticalPointsRing2, self.cumulativePhaseHeightInRing2, self.phaseSequenceInRing2,
                                                   self.ETA_EV, self.ETA_Duration_EV, self.requestedPhase_EV, self.vehicleType_EV, self.delay_EV,
                                                   self.ETA_Transit, self.ETA_Duration_Transit, self.requestedPhase_Transit, self.vehicleType_Transit, self.delay_Transit,
                                                   self.ETA_Truck, self.ETA_Duration_Truck, self.requestedPhase_Truck, self.vehicleType_Truck, self.delay_Truck,
                                                   self.ETA_Coordination, self.ETA_Duration_Coordination, self.requestedPhase_Coordination, self.vehicleType_Coordination, self.delay_Coordination,
                                                   self.ETA_DilemmaZone, self.ETA_Duration_DilemmaZone, self.requestedPhase_DilemmaZone, self.vehicleType_DilemmaZone, self.delay_DilemmaZone)
        self.timePhaseDiagramManager.timePhaseDiagramMethod("Ring1&2")


'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    optimizationResultsManager = OptimizationResultsManager()
    optimizationResultsManager.readOptimizationResultsFile()