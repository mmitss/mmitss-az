"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

TimeSpaceDiagramManager.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
The methods available from this class are the following:
- getParameters(): method to initialize the parameters and variables
- timeSpaceDiagramMethod: method to generate the time-space diagram based on the SPaT and BSM data
***************************************************************************************
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle


class TimeSpaceDiagramManager:
    def __init__(self):
        pass

    def getParameters(self, greenRectangleStartPoint, clearanceRectangleStartPoint, greenTime, clearanceTime, 
                    distance_Green, distance_Clearance, evTrajectoryTimePoint, evTrajectoryDistancePoint, 
                    transitTrajectoryTimePoint, transitTrajectoryDistancePoint, truckTrajectoryTimePoint, 
                    truckTrajectoryDistancePoint, carTrajectoryTimePoint, carTrajectoryDistancePoint, 
                    connectedVehicleTrajectoryTimePoint, connectedVehicleTrajectoryDistancePoint, textString, xAxisRange):
        """
        method to initialize the parameters and variables
        """
        self.greenRectangleStartPoint = greenRectangleStartPoint
        self.clearanceRectangleStartPoint = clearanceRectangleStartPoint
        self.greenTime = greenTime
        self.clearanceTime = clearanceTime
        self.distance_Green = distance_Green
        self.distance_Clearance = distance_Clearance
        self.evTrajectoryTimePoint, self.evTrajectoryDistancePoint = evTrajectoryTimePoint, evTrajectoryDistancePoint,
        self.transitTrajectoryTimePoint, self.transitTrajectoryDistancePoint = transitTrajectoryTimePoint, transitTrajectoryDistancePoint
        self.truckTrajectoryTimePoint, self.truckTrajectoryDistancePoint = truckTrajectoryTimePoint, truckTrajectoryDistancePoint
        self.carTrajectoryTimePoint, self.carTrajectoryDistancePoint = carTrajectoryTimePoint, carTrajectoryDistancePoint
        self.connectedVehicleTrajectoryTimePoint, self.connectedVehicleTrajectoryDistancePoint = connectedVehicleTrajectoryTimePoint, connectedVehicleTrajectoryDistancePoint
        self.textString = textString
        self.xAxisRange = xAxisRange

    def timeSpaceDiagramMethod(self):
        """
        method to generate the time-space diagram based on the SPaT and BSM data
        """
        fig, ax1 = plt.subplots()

        ax1.set_xlabel('Time (s)', fontsize=24, fontweight='bold')
        ax1.set_ylabel('Distance (m)', fontsize=24, fontweight='bold')
        max_x_limit = self.xAxisRange-100
        plt.xlim([0, max_x_limit])
        plt.ylim([0, max(self.distance_Green)+400])
        plt.xticks(np.arange(0, self.xAxisRange-75, 50), fontsize=24)
        ax1.tick_params(axis='y',  labelsize=18)
        for axis in ['top', 'bottom', 'left', 'right']:
            ax1.spines[axis].set_linewidth(4)
        # ax1.set_yticks(ticks=np.arange(0, 100, 20),fontsize = 24)
        #newYlabel = ['-400','0','395','810','1225']
        # plt.gca().set_yticklabels(newYlabel)
        # plt.yticks([])
        req_phase_length = len(self.greenRectangleStartPoint)
        for i in range(0, req_phase_length):
            x = self.greenRectangleStartPoint[i]
            y = self.distance_Green[i]
            ax1.add_patch(Rectangle(
                (x, y), self.greenTime[i], 30, angle=0.0, color='green', linewidth=2,))

        req_phase_length = len(self.clearanceRectangleStartPoint)
        for i in range(0, req_phase_length):
            x = self.clearanceRectangleStartPoint[i]
            y = self.distance_Clearance[i]
            ax1.add_patch(Rectangle(
                (x, y), self.clearanceTime[i], 30, angle=0.0, color='red', linewidth=2))


        if len(self.evTrajectoryTimePoint) > 0:
            ax1.scatter(self.evTrajectoryTimePoint, self.evTrajectoryDistancePoint, c="black",  linewidths=4,
                        marker=".",  edgecolor="none",  s=50, label='Connected Vehicles Trajectory', zorder=2)

        if len(self.transitTrajectoryTimePoint) > 0:
            ax1.scatter(self.transitTrajectoryTimePoint, self.transitTrajectoryDistancePoint, c="black",
                        linewidths=4, marker=".",  edgecolor="none",  s=50, label='Connected Vehicles Trajectory', zorder=2)

        if len(self.truckTrajectoryTimePoint) > 0:
            ax1.scatter(self.truckTrajectoryTimePoint, self.truckTrajectoryDistancePoint, c="black",
                        linewidths=4, marker=".",  edgecolor="none",  s=50, label='Connected Vehicles Trajectory', zorder=2)

        if len(self.carTrajectoryTimePoint) > 0:
            ax1.scatter(self.carTrajectoryTimePoint, self.carTrajectoryDistancePoint, c="black",  linewidths=4,
                        marker=".",  edgecolor="none",  s=50, label='Connected Vehicles Trajectory', zorder=2)

        if len(self.connectedVehicleTrajectoryTimePoint) > 0:
            ax1.scatter(self.connectedVehicleTrajectoryTimePoint, self.connectedVehicleTrajectoryDistancePoint, c="black",  linewidths=4,
                        marker=".",  edgecolor="none",  s=50, label='Connected Vehicles Trajectory', zorder=2)                

        ax1.legend(loc='upper right', prop={"size": 16})
        ax1.set_title("Time-Space Diagram", fontsize=20, fontweight='bold')
        fig.tight_layout()  # otherwise the right y-label is slightly clipped
        plt.grid(color='black', linestyle='-', linewidth=0.5)
        plt.show()
