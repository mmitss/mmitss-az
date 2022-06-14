"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

signal-coordination-request-generator.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This is a wrapper module for generating time-space diagram. It performs
following functions:
(1) computes horizontal range of the diagram
(2) creates an instance of SpatDataManager class and calls manageSpatData() method to process all the SPaT files
(3) creates an instance of BSMDataManager class and calls manageBsmData() method to process all the BSM files
(4) creates an instance of TimeSpaceDiagramManager class and calls timeSpaceDiagramMethod() method to generate the time-space diagram
***************************************************************************************
"""

import json 
from SpatDataManager import SpatDataManager
from BsmDataManager import BsmDataManager
from TimeSpaceDiagramManager import TimeSpaceDiagramManager


def main():   
    # Read the config file into a json object:
    configFile = open("configuration.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()

    xAxisRange = config["EndTimeOfDiagram"] - config["StartTimeOfDiagram"]

    spatDataManager = SpatDataManager(config)
    bsmDataManager = BsmDataManager(config)
    timeSpaceDiagramManager = TimeSpaceDiagramManager()

    """
    Process SPaT files
    """ 
    greenTimeStartPoint, clearanceTimeStartPoint, greenTime, clearanceTime, intersectionDistance_Green, intersectionDistance_Clearance = spatDataManager.manageSpatData()

    """
    Process BSM files
    """
    
    evTrajectoryTimePoint, evTrajectoryDistancePoint, transitTrajectoryTimePoint, transitTrajectoryDistancePoint, truckTrajectoryTimePoint, truckTrajectoryDistancePoint, carTrajectoryTimePoint, carTrajectoryDistancePoint, connectedVehicleTrajectoryTimePoint, connectedVehicleTrajectoryDistancePoint = bsmDataManager.manageBsmData()

    """
    Plot Time-Space Diagram
    """
    timeSpaceDiagramManager.getParameters(greenTimeStartPoint, clearanceTimeStartPoint,
                     greenTime, clearanceTime, intersectionDistance_Green, intersectionDistance_Clearance, evTrajectoryTimePoint, evTrajectoryDistancePoint, transitTrajectoryTimePoint, transitTrajectoryDistancePoint, truckTrajectoryTimePoint, truckTrajectoryDistancePoint, carTrajectoryTimePoint, carTrajectoryDistancePoint, connectedVehicleTrajectoryTimePoint, connectedVehicleTrajectoryDistancePoint, "Time-Space Diagram", xAxisRange)

    timeSpaceDiagramManager.timeSpaceDiagramMethod()
    
if __name__ == "__main__":
    main()