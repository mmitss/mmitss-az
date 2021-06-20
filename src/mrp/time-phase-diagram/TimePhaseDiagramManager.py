import time
import datetime
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
import shutil
import os


class TimePhaseDiagramManager:
    def __init__(self):
        pass

    def archiveOldDiagrams(self):
        for file in os.listdir(r'/nojournal/bin/performance-measurement-diagrams/time-phase-diagram'):
            if file.endswith(".jpg"):
                shutil.move("/nojournal/bin/performance-measurement-diagrams/time-phase-diagram/" +
                            file, "/nojournal/bin/performance-measurement-diagrams/time-phase-diagram/archive")
    

    def getParameters(self, cumulativeLeftCriticalPointsRing1, cumulativeRightCriticalPointsRing1, cumulativePhaseHeightInRing1, phaseSequenceInRing1,
                      cumulativeLeftCriticalPointsRing2, cumulativeRightCriticalPointsRing2, cumulativePhaseHeightInRing2, phaseSequenceInRing2,
                      ETA_EV, ETA_Duration_EV, requestedPhase_EV, vehicleType_EV, delay_EV,
                      ETA_Transit, ETA_Duration_Transit, requestedPhase_Transit, vehicleType_Transit, delay_Transit,
                      ETA_Truck, ETA_Duration_Truck, requestedPhase_Truck, vehicleType_Truck, delay_Truck,
                      ETA_Coordination, ETA_Duration_Coordination, requestedPhase_Coordination, vehicleType_Coordination, delay_Coordination,
                      ETA_DilemmaZone, ETA_Duration_DilemmaZone, requestedPhase_DilemmaZone, vehicleType_DilemmaZone, delay_DilemmaZone):

        self.cumulativeLeftCriticalPointsRing1,self.cumulativeRightCriticalPointsRing1,self.cumulativePhaseHeightInRing1, self.phaseSequenceInRing1 = cumulativeLeftCriticalPointsRing1, cumulativeRightCriticalPointsRing1, cumulativePhaseHeightInRing1, phaseSequenceInRing1
        self.cumulativeLeftCriticalPointsRing2, self.cumulativeRightCriticalPointsRing2, self.cumulativePhaseHeightInRing2, self.phaseSequenceInRing2 = cumulativeLeftCriticalPointsRing2,cumulativeRightCriticalPointsRing2, cumulativePhaseHeightInRing2, phaseSequenceInRing2
        self.ETA_EV, self.ETA_Duration_EV, self.requestedPhase_EV, self.vehicleType_EV, self.delay_EV = ETA_EV, ETA_Duration_EV, requestedPhase_EV, vehicleType_EV, delay_EV
        self.ETA_Transit, self.ETA_Duration_Transit, self.requestedPhase_Transit, self.vehicleType_Transit, self.delay_Transit = ETA_Transit, ETA_Duration_Transit, requestedPhase_Transit, vehicleType_Transit, delay_Transit
        self.ETA_Truck, self.ETA_Duration_Truck, self.requestedPhase_Truck, self.vehicleType_Truck, self.delay_Truck = ETA_Truck, ETA_Duration_Truck, requestedPhase_Truck, vehicleType_Truck, delay_Truck
        self.ETA_Coordination, self.ETA_Duration_Coordination, self.requestedPhase_Coordination, self.vehicleType_Coordination, self.delay_Coordination = ETA_Coordination, ETA_Duration_Coordination, requestedPhase_Coordination, vehicleType_Coordination, delay_Coordination
        self.ETA_DilemmaZone, self.ETA_Duration_DilemmaZone, self.requestedPhase_DilemmaZone, self.vehicleType_DilemmaZone, self.delay_DilemmaZone = ETA_DilemmaZone, ETA_Duration_DilemmaZone, requestedPhase_DilemmaZone, vehicleType_DilemmaZone, delay_DilemmaZone




    def timePhaseDiagramMethod(self, ringNo):
        self.archiveOldDiagrams()
        fig, ax1=plt.subplots(figsize = (18, 12))

        if ringNo == 'Ring1&2':
            color='tab:red'
            ax1.set_xlabel('Time (s)', fontsize = 24, fontweight = 'bold')
            ax1.set_ylabel('Ring 1', color = color,
                           fontsize = 28, fontweight = 'bold')
            ax1.plot(self.cumulativeLeftCriticalPointsRing1,
                     self.cumulativePhaseHeightInRing1, color=color, linewidth=4)
            ax1.plot(self.cumulativeRightCriticalPointsRing1,
                     self.cumulativePhaseHeightInRing1, color=color, linewidth=4)
            plt.xticks(np.arange(
                self.cumulativeRightCriticalPointsRing1[0], self.cumulativeRightCriticalPointsRing1[-1], 20), fontsize=24)
            # ax1.set_yticks(ticks=np.arange(
            #     self.cumulativePhaseHeightInRing1[0], self.cumulativePhaseHeightInRing1[-1], 10))
            ax1.set_yticks(self.cumulativePhaseHeightInRing1[0:-1])
            ax1.set_yticklabels(self.phaseSequenceInRing1)
            ax1.tick_params(axis='y', labelcolor=color, labelsize=18)
            for axis in ['top', 'bottom', 'left', 'right']:
                ax1.spines[axis].set_linewidth(4)

            ax1.scatter(self.cumulativeLeftCriticalPointsRing1, self.cumulativePhaseHeightInRing1,
                        marker='o', c="orange", linewidths=6, alpha=1.0, label='Left & Right Critical Points')
            ax1.scatter(self.cumulativeRightCriticalPointsRing1, self.cumulativePhaseHeightInRing1,
                        marker='o', c="orange", linewidths=6, alpha=1.0)

            # Ring2
            ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
            color = 'tab:blue'
            ax2.set_ylabel('Ring 2', color=color,
                           fontsize=28, fontweight='bold')
            ax2.plot(self.cumulativeLeftCriticalPointsRing2,
                     self.cumulativePhaseHeightInRing2, color=color, linewidth=4)
            ax2.plot(self.cumulativeRightCriticalPointsRing2,
                     self.cumulativePhaseHeightInRing2, color=color, linewidth=4)
            ax2.set_yticks(self.cumulativePhaseHeightInRing2[0:-1])
            ax2.set_yticklabels(self.phaseSequenceInRing2)
            ax2.tick_params(axis='y', labelcolor=color, labelsize=24)

            ax2.scatter(self.cumulativeLeftCriticalPointsRing2, self.cumulativePhaseHeightInRing2,
                        marker='o', c="orange", linewidths=6, alpha=1.0, label='Left & Right Critical Points')
            ax2.scatter(self.cumulativeRightCriticalPointsRing2, self.cumulativePhaseHeightInRing2,
                        marker='o', c="orange", linewidths=6, alpha=1.0)

            if len(self.phaseSequenceInRing1) > len(self.phaseSequenceInRing2):
                ax1.grid(color='black', linestyle='-', linewidth=2, axis='y')

            else:
                ax2.grid(color='black', linestyle='-', linewidth=2, axis='y')

        if(len(self.vehicleType_EV) > 0):
            requestedPhasePosition, requestedPhaseHeight = self.getRequestedPhasePositionAndHeight(
                self.requestedPhase_EV)

            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_EV[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_EV[i]
                h = requestedPhaseHeight[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='red', linewidth=2, label='EV Priority Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='red', linewidth=2))

        if(len(self.vehicleType_Transit) > 0):
            requestedPhasePosition, requestedPhaseHeight = self.getRequestedPhasePositionAndHeight(
                self.requestedPhase_Transit)

            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_Transit[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_Transit[i]
                h = requestedPhaseHeight[i]
                if i == 0:
                    # if(len(self.delay_Transit) > 0):
                    #     ax1.add_patch(matplotlib.patches.Rectangle(
                    #         (x, y), self.delay_Transit[i], 1, angle=0.0, color='red', linewidth=1))
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='green', linewidth=2, label='Transit Priority Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='green', linewidth=2))

        if(len(self.vehicleType_Truck) > 0):
            requestedPhasePosition, requestedPhaseHeight = self.getRequestedPhasePositionAndHeight(
                self.requestedPhase_Truck)

            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_Truck[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_Truck[i]
                h = requestedPhaseHeight[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='navy', linewidth=2, label='Truck Priority Request'))

                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='navy', linewidth=2))

        if(len(self.vehicleType_Coordination) > 0):
            requestedPhasePosition, requestedPhaseHeight = self.getRequestedPhasePositionAndHeight(
                self.requestedPhase_Coordination)

            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_Coordination[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_Coordination[i]
                h = requestedPhaseHeight[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='darkcyan', linewidth=2, label='Coordination Priority Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='darkcyan', linewidth=2))

        if(len(self.vehicleType_DilemmaZone) > 0):
            requestedPhasePosition, requestedPhaseHeight = self.getRequestedPhasePositionAndHeight(
                self.requestedPhase_DilemmaZone)

            for i in range(0, len(requestedPhasePosition)):
                x = self.ETA_DilemmaZone[i]
                y = requestedPhasePosition[i]
                z = self.ETA_Duration_DilemmaZone[i]
                h = requestedPhaseHeight[i]
                if i == 0:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='navy', linewidth=2, label='DilemmaZone Request'))
                else:
                    ax1.add_patch(matplotlib.patches.Rectangle(
                        (x, y), z, h, angle=0.0, color='navy', linewidth=2))

        ax1.legend(loc='upper right', bbox_to_anchor=(
            0.9, 1), prop={"size": 18})
        ax1.set_title("Time-Phase Diagram [" + str(time.time()) + " / " + str(
            datetime.datetime.now()) + "]", fontsize=20, fontweight='bold')
        fig.tight_layout()  # otherwise the right y-label is slightly clipped
        fileName = "/nojournal/bin/performance-measurement-diagrams/time-phase-diagram/time-phase-diagram" + \
            str(time.time())
        plt.savefig(fileName+'.jpg', bbox_inches='tight', dpi=300)

        # plt.show()
        
        print("[{}]".format(str(round(time.time(), 4))) + " " + "Generate Time-Phase Diagram")
        
    def getRequestedPhasePositionAndHeight(self, requestedPhaseList):
        requestedPhasePosition = []
        requestedPhaseHeight = []
        indexPosList = []

        for requestedPhase in requestedPhaseList:
            indexPosList.clear()
            if requestedPhase < 5:
                [indexPosList.append(phase) for phase in range(len(
                    self.phaseSequenceInRing1)) if self.phaseSequenceInRing1[phase] == requestedPhase]
                [requestedPhasePosition.append(
                    self.cumulativePhaseHeightInRing1[index]) for index in indexPosList]
                [requestedPhaseHeight.append(
                    self.cumulativePhaseHeightInRing1[index+1] - self.cumulativePhaseHeightInRing1[index]) for index in indexPosList]

            else:
                [indexPosList.append(phase) for phase in range(len(
                    self.phaseSequenceInRing2)) if self.phaseSequenceInRing2[phase] == requestedPhase]
                [requestedPhasePosition.append(
                    self.cumulativePhaseHeightInRing2[index]) for index in indexPosList]
                [requestedPhaseHeight.append(
                    self.cumulativePhaseHeightInRing2[index+1] - self.cumulativePhaseHeightInRing2[index]) for index in indexPosList]
    
        return requestedPhasePosition, requestedPhaseHeight
