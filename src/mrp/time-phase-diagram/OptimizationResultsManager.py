import json
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
from numpy.testing._private.utils import requires_memory


class OptimizationResultsManager:
    def __init__(self):

        self.phaseSequenceInRing1, self.phaseSequenceInRing2, self.phaseLengthInRing1, self.phaseLengthInRing2 = ([] for i in range(4))
        self.leftCriticalPointsRing1, self.rightCriticalPointsRing1,  self.leftCriticalPointsRing2, self.rightCriticalPointsRing2 = ([] for i in range(4))
        self.cumulativePhaseSequenceInRing1, self.cumulativePhaseSequenceInRing2, self.cumulativePhaseLengthInRing1, self.cumulativePhaseLengthInRing2 = ([] for i in range(4))
        self.cumulativeLeftCriticalPointsRing1, self.cumulativeRightCriticalPointsRing1, self.cumulativeLeftCriticalPointsRing2, self.cumulativeRightCriticalPointsRing2 = ([] for i in range(4))
        self.ETA_EV, self.ETA_Duration_EV, self.requestedPhase_EV, self.vehicleType_EV = ([] for i in range(4))
        self.ETA_Transit, self.ETA_Duration_Transit, self.requestedPhase_Transit, self.vehicleType_Transit = ([] for i in range(4))
        self.ETA_Truck, self.ETA_Duration_Truck, self.requestedPhase_Truck, self.vehicleType_Truck = ([] for i in range(4))
        self.ETA_Coordination, self.ETA_Duration_Coordination, self.requestedPhase_Coordination, self.vehicleType_Coordination = ([] for i in range(4))
        self.ETA_DilemmaZone, self.ETA_Duration_DilemmaZone, self.requestedPhase_DilemmaZone, self.vehicleType_DilemmaZone = ([] for i in range(4))
        self.phaseDurationList = []
        self.startingPhase1, self.startingPhase2 = 0, 0
        self.init1, self.init2 = 0.0, 0.0

    def readOptimizationResultsFile(self, fileName):
        requiredLineNo =0
        self.optimizationResultsFile = open(fileName, 'r')
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
        self.getPriorityRequests(fileName, requiredLineNo)

        print("critical points are ", self.leftCriticalPointsRing1, self.rightCriticalPointsRing1,
              self.leftCriticalPointsRing2, self.rightCriticalPointsRing2)
        self.getCummulativeValues()

    def getPriorityRequests(self, fileName, requiredLineNo):
        self.optimizationResultsFile = open(fileName, 'r')
        for i, line in enumerate(self.optimizationResultsFile):

            if i in range(15, requiredLineNo):
                reqPhase, earliestArrival, latestArrival, delay, vehicleClass = line.split()
                if int(vehicleClass) == 1:
                    self.requestedPhase_EV.append(int(reqPhase))
                    self.ETA_EV.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_EV.append(4.0)
                    self.vehicleType_EV.append(int(vehicleClass))
                
                elif int(vehicleClass) == 2:
                    self.requestedPhase_Transit.append(int(reqPhase))
                    self.ETA_Transit.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_Transit.append(4.0)
                    self.vehicleType_Transit.append(int(vehicleClass))
                    
                elif int(vehicleClass) == 3:
                    self.requestedPhase_Truck.append(int(reqPhase))
                    self.ETA_Truck.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_Truck.append(4.0)
                    self.vehicleType_Truck.append(int(vehicleClass))
                    
                elif int(vehicleClass) == 4:
                    self.requestedPhase_Coordination.append(int(reqPhase))
                    self.ETA_Coordination.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_Coordination.append(4.0)
                    self.vehicleType_Coordination.append(int(vehicleClass))
                    
                elif int(vehicleClass) == 5:
                    self.requestedPhase_DilemmaZone.append(int(reqPhase))
                    self.ETA_DilemmaZone.append(float(latestArrival) - 4.0)
                    self.ETA_Duration_DilemmaZone.append(4.0)
                    self.vehicleType_DilemmaZone.append(int(vehicleClass))
        
    def processPhaseDuration(self, line, list1, list2):
        phaseDurationOfP1, phaseDurationOfP2, phaseDurationOfP3, phaseDurationOfP4, phaseDurationOfP5, phaseDurationOfP6, phaseDurationOfP7, phaseDurationOfP8 = line.split()

        self.phaseDurationList = [float(phaseDurationOfP1), float(phaseDurationOfP2), float(phaseDurationOfP3), float(phaseDurationOfP4),
                                  float(phaseDurationOfP5), float(phaseDurationOfP6), float(phaseDurationOfP7), float(phaseDurationOfP8)]

        [list1.append(value) for index, value in enumerate(
            self.phaseDurationList) if value > 0.0 and index < 4]
        [list2.append(value) for index, value in enumerate(
            self.phaseDurationList) if value > 0.0 and index > 4]
        # print("Phase Duration List is: ", list1, list2)

    def processPhaseSequence(self):
        phasesInRing1, phasesInRing2 = ([] for i in range(2))

        [phasesInRing1.append(index+1) for index, value in enumerate(
            self.phaseDurationList) if value > 0.0 and index < 4]
        [phasesInRing2.append(index+1) for index, value in enumerate(
            self.phaseDurationList) if value > 0.0 and index > 4]

        self.phaseSequenceInRing1.extend(phasesInRing1)
        self.phaseSequenceInRing2.extend(phasesInRing2)

        # print("Phase sequences are: ", self.phaseSequenceInRing1, self.phaseSequenceInRing2)
        self.processPhaseLength(phasesInRing1, phasesInRing2)

    def processPhaseLength(self, phasesInRing1, phasesInRing2):
        P11, P12, P21, P22 = ([] for i in range(4))
        # P11Length, P12Length, P21Length, P22Length  = ([] for i in range(4))
        phaseLengthDictionary = {}

        [P11.append(index+1)for index, value in enumerate(self.phaseDurationList)
         if value > 0.0 and index < 2]
        [P12.append(index+1)for index, value in enumerate(self.phaseDurationList)
         if value > 0.0 and index >= 2 and index < 4]
        [P21.append(index+1)for index, value in enumerate(self.phaseDurationList)
         if value > 0.0 and index >= 4 and index < 6]
        [P22.append(index+1)for index, value in enumerate(self.phaseDurationList)
         if value > 0.0 and index >= 6 and index < 8]

        # print("Phases in per ring-barrier group are: " ,P11, P12, P21, P22)

        if (len(P11) == len(P21)):
            for index in range(len(P11)):
                if len(P11) > 0:
                    phaseLengthDictionary[str(P11[index])] = 10

            for index in range(len(P21)):
                if len(P21) > 0:
                    phaseLengthDictionary[str(P21[index])] = 10

        elif (len(P11) < len(P21)):
            for index in range(len(P11)):
                if len(P11) > 0:
                    phaseLengthDictionary[str(P11[index])] = 20

            for index in range(len(P21)):
                if len(P21) > 0:
                    phaseLengthDictionary[str(P21[index])] = 10

        elif (len(P11) > len(P21)):
            for index in range(len(P11)):
                if len(P11) > 0:
                    phaseLengthDictionary[str(P11[index])] = 10

            for index in range(len(P21)):
                if len(P21) > 0:
                    phaseLengthDictionary[str(P21[index])] = 20

        if (len(P12) == len(P22)):
            for index in range(len(P12)):
                if len(P12) > 0:
                    phaseLengthDictionary[str(P12[index])] = 10

            for index in range(len(P22)):
                if len(P22) > 0:
                    phaseLengthDictionary[str(P22[index])] = 10

        elif (len(P12) < len(P22)):
            for index in range(len(P12)):
                if len(P12) > 0:
                    phaseLengthDictionary[str(P12[index])] = 20
            for index in range(len(P22)):
                if len(P22) > 0:
                    phaseLengthDictionary[str(P22[index])] = 10

        elif (len(P12) > len(P22)):
            for index in range(len(P12)):
                if len(P12) > 0:
                    phaseLengthDictionary[str(P12[index])] = 10
            for index in range(len(P22)):
                if len(P22) > 0:
                    phaseLengthDictionary[str(P22[index])] = 20

        # print("Phase Length Dictionary is: ", phaseLengthDictionary)

        for phase in phasesInRing1:
            for key, value in phaseLengthDictionary.items():
                if int(key) == phase:
                    self.phaseLengthInRing1.append(value)

        for phase in phasesInRing2:
            for key, value in phaseLengthDictionary.items():
                if int(key) == phase:
                    self.phaseLengthInRing2.append(value)

    def getCummulativeValues(self):

        self.cumulativePhaseLengthInRing1 = np.cumsum(self.phaseLengthInRing1)
        self.cumulativePhaseLengthInRing2 = np.cumsum(self.phaseLengthInRing2)
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
        
        self.cumulativePhaseLengthInRing1 = np.insert(
            self.cumulativePhaseLengthInRing1, 0, 0.0)
        self.cumulativePhaseLengthInRing2 = np.insert(
            self.cumulativePhaseLengthInRing2, 0, 0.0)
        self.cumulativeLeftCriticalPointsRing1 = np.insert(
            self.cumulativeLeftCriticalPointsRing1, 0, 0.0)
        self.cumulativeRightCriticalPointsRing1 = np.insert(
            self.cumulativeRightCriticalPointsRing1, 0, 0.0)
        self.cumulativeLeftCriticalPointsRing2 = np.insert(
            self.cumulativeLeftCriticalPointsRing2, 0, 0.0)
        self.cumulativeRightCriticalPointsRing2 = np.insert(
            self.cumulativeRightCriticalPointsRing2, 0, 0.0)
        
        print("Cunnulative critical points are ", self.cumulativeLeftCriticalPointsRing1, self.cumulativeRightCriticalPointsRing1,
              self.cumulativeLeftCriticalPointsRing2, self.cumulativeRightCriticalPointsRing2)               

        self.timePhaseDiagramMethod("Ring1&2")

    
    def timePhaseDiagramMethod(self, ringNo):

        fig, ax1 = plt.subplots()

        if ringNo == 'Ring1&2':
            color = 'tab:red'
            ax1.set_xlabel('Time (s)', fontsize=24, fontweight='bold')
            ax1.set_ylabel('Ring 1', color=color,
                           fontsize=28, fontweight='bold')
            ax1.plot(self.cumulativeLeftCriticalPointsRing1,
                     self.cumulativePhaseLengthInRing1, color=color, linewidth=4)
            ax1.plot(self.cumulativeRightCriticalPointsRing1,
                     self.cumulativePhaseLengthInRing1, color=color, linewidth=4)
            plt.xticks(np.arange(
                self.cumulativeRightCriticalPointsRing1[0], self.cumulativeRightCriticalPointsRing1[-1], 20), fontsize=24)
            # ax1.set_yticks(ticks=np.arange(
            #     self.cumulativePhaseLengthInRing1[0], self.cumulativePhaseLengthInRing1[-1], 10))
            ax1.set_yticks(self.cumulativePhaseLengthInRing1[0:-1])
            ax1.set_yticklabels(self.phaseSequenceInRing1)
            ax1.tick_params(axis='y', labelcolor=color, labelsize=18)
            for axis in ['top', 'bottom', 'left', 'right']:
                ax1.spines[axis].set_linewidth(4)

            ax1.scatter(self.cumulativeLeftCriticalPointsRing1, self.cumulativePhaseLengthInRing1,
                        marker='o', c="orange", linewidths=4, alpha=1.0, label='Left & Right Critical Points')
            ax1.scatter(self.cumulativeRightCriticalPointsRing1, self.cumulativePhaseLengthInRing1,
                        marker='o', c="orange", linewidths=4, alpha=1.0)

            # Ring2
            ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
            color = 'tab:blue'
            ax2.set_ylabel('Ring 2', color=color,
                           fontsize=28, fontweight='bold')
            ax2.plot(self.cumulativeLeftCriticalPointsRing2,
                     self.cumulativePhaseLengthInRing2, color=color, linewidth=4)
            ax2.plot(self.cumulativeRightCriticalPointsRing2,
                     self.cumulativePhaseLengthInRing2, color=color, linewidth=4)
            ax2.set_yticks(self.cumulativePhaseLengthInRing2[0:-1])
            ax2.set_yticklabels(self.phaseSequenceInRing2)
            ax2.tick_params(axis='y', labelcolor=color, labelsize=24)

            ax2.scatter(self.cumulativeLeftCriticalPointsRing2, self.cumulativePhaseLengthInRing2,
                        marker='o', c="orange", linewidths=4, alpha=1.0, label='Left & Right Critical Points')
            ax2.scatter(self.cumulativeRightCriticalPointsRing2, self.cumulativePhaseLengthInRing2,
                        marker='o', c="orange", linewidths=4, alpha=1.0)

            if len(self.phaseSequenceInRing1) > len(self.phaseSequenceInRing2):
                ax1.grid(color='black', linestyle='-', linewidth=2, axis='y')

            else:
                ax2.grid(color='black', linestyle='-', linewidth=2, axis='y')
                         
        
        if(len(self.vehicleType_EV)>0):
            self.ETA_EV.extend(self.ETA_EV*1)
            self.ETA_Duration_EV.extend(self.ETA_Duration_EV*1)
            requestedPhasePosition = self.getRequestedPhasePosition(self.requestedPhase_EV)
            
            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_EV[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_EV[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='red', linewidth=2, label='EV Priority Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='red', linewidth=2))
        
        if(len(self.vehicleType_Transit)>0):
            self.ETA_Transit.extend(self.ETA_Transit*1)
            self.ETA_Duration_Transit.extend(self.ETA_Duration_Transit*1)
            requestedPhasePosition = self.getRequestedPhasePosition(self.requestedPhase_Transit)
            
            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_Transit[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_Transit[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='green', linewidth=2, label='Transit Priority Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='green', linewidth=2))            
        
        if(len(self.vehicleType_Truck)>0):
            self.ETA_Truck.extend(self.ETA_Truck*1)
            self.ETA_Duration_Truck.extend(self.ETA_Duration_Truck*1)
            requestedPhasePosition = self.getRequestedPhasePosition(self.requestedPhase_Truck)
            
            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_Truck[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_Truck[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='navy', linewidth=2, label='Truck Priority Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='navy', linewidth=2))
        
        if(len(self.vehicleType_Coordination)>0):
            self.ETA_Coordination.extend(self.ETA_Coordination*1)
            self.ETA_Duration_Coordination.extend(self.ETA_Duration_Coordination*1)
            requestedPhasePosition = self.getRequestedPhasePosition(self.requestedPhase_Coordination)
            
            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_Coordination[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_Coordination[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='orange', linewidth=2, label='Coordination Priority Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='orange', linewidth=2))
        
        if(len(self.vehicleType_DilemmaZone)>0):
            self.ETA_DilemmaZone.extend(self.ETA_DilemmaZone*1)
            self.ETA_Duration_DilemmaZone.extend(self.ETA_Duration_DilemmaZone*1)
            requestedPhasePosition = self.getRequestedPhasePosition(self.requestedPhase_DilemmaZone)
            
            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_DilemmaZone[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_DilemmaZone[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='navy', linewidth=2, label='DilemmaZone Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, 10, angle=0.0, color='navy', linewidth=2))    
        
        ax1.legend(loc='upper right', bbox_to_anchor=(
            0.9, 1), prop={"size": 18})
        fig.tight_layout()  # otherwise the right y-label is slightly clipped

        plt.show()


    def getRequestedPhasePosition(self, requestedPhaseList):
        requestedPhasePosition = []      
        indexPosList = []
        for requestedPhase in requestedPhaseList:
            indexPosList.clear()
            if requestedPhase < 5:
                [indexPosList.append(phase) for phase in range(len(self.phaseSequenceInRing1)) if self.phaseSequenceInRing1[phase] == requestedPhase]
                [requestedPhasePosition.append(self.cumulativePhaseLengthInRing1[index]) for index in indexPosList]
            
            else:
                [indexPosList.append(phase) for phase in range(len(self.phaseSequenceInRing2)) if self.phaseSequenceInRing2[phase] == requestedPhase]
                [requestedPhasePosition.append(self.cumulativePhaseLengthInRing2[index]) for index in indexPosList]
                
                
        return requestedPhasePosition
'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    optimizationResultsManager = OptimizationResultsManager()
    optimizationResultsManager.readOptimizationResultsFile(
        '/nojournal/bin/OptimizationResults.txt')
