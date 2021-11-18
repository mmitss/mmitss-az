"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

BsmDataManager.py
Created by: Debashis Das
University of Arizona
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
The methods available from this class are the following:
- getBsmConfigData(): method to get config data for the BSM files
- manageBsmData(): method to store vehicle trajectories based on their vehicle type
- processBsmFile(BsmFile, desiredSignaGroup, distance): method to process each BSM file to obtain vehicle trajectories
- findUniqueVehicleIdInDataFrame(bsmDf, desiredSignaGroup): method to compute unique vehicle ID that are travelling along the desired signal group
- getVehicleTrajectory(uniqueVehicleId, dataframe, distance): method to obtain vehicle trajectory for a specific vehicle ID
***************************************************************************************
"""

import pandas as pd


class BsmDataManager:
    def __init__(self, config):
        self.config = config
        self.startTime = 0.0
        self.endTime = 0.0
        self.fileDirectoryList_BSM = []
        self.intersectionDistanceApart = []
        self.signalGroup = []

    def getBsmConfigData(self):
        """
        method to get config data for the BSM files
        """
        [self.fileDirectoryList_BSM.append(
            fileDirectory)for fileDirectory in self.config["BsmFileDirectory"]]
        [self.intersectionDistanceApart.append(
            distance) for distance in self.config["IntersectionDistance"]]
        [self.signalGroup.append(phase)
                                 for phase in self.config["DeisredSignalGroup"]]
        self.startTime = self.config["StartTimeOfDiagram"]
        self.endTime = self.config["EndTimeOfDiagram"]

    def manageBsmData(self):
        """
        method to store vehicle trajectories based on their vehicle type
        """
        self.getBsmConfigData()
        evTrajectoryTimePoint, evTrajectoryDistancePoint = (
            [] for i in range(2))
        transitTrajectoryTimePoint, transitTrajectoryDistancePoint = (
            [] for i in range(2))
        truckTrajectoryTimePoint, truckTrajectoryDistancePoint = (
            [] for i in range(2))
        carTrajectoryTimePoint, carTrajectoryDistancePoint = (
            [] for i in range(2))
        connectedVehicleTrajectoryTimePoint, connectedVehicleTrajectoryDistancePoint = (
            [] for i in range(2))

        for index, bsmFile in enumerate(self.fileDirectoryList_BSM):
            desiredSignaGroup = self.signalGroup[index]
            vehicleTrajectoryTime, vehicleTrajectoryDistance, vehicleType = self.processBsmFile(
                bsmFile, desiredSignaGroup, self.intersectionDistanceApart[index])

            [evTrajectoryTimePoint.append(element) for index, element in enumerate(
                vehicleTrajectoryTime) if vehicleType[index] == "EmergencyVehicle"]
            [evTrajectoryDistancePoint.append(element) for index,  element in enumerate(
                vehicleTrajectoryDistance) if vehicleType[index] == "EmergencyVehicle"]

            [transitTrajectoryTimePoint.append(element) for index, element in enumerate(
                vehicleTrajectoryTime) if vehicleType[index] == "Transit"]
            [transitTrajectoryDistancePoint.append(element) for index,  element in enumerate(
                vehicleTrajectoryDistance) if vehicleType[index] == "Transit"]

            [truckTrajectoryTimePoint.append(element) for index, element in enumerate(
                vehicleTrajectoryTime) if vehicleType[index] == "Truck"]
            [truckTrajectoryDistancePoint.append(element) for index,  element in enumerate(
                vehicleTrajectoryDistance) if vehicleType[index] == "Truck"]

            [carTrajectoryTimePoint.append(element) for index, element in enumerate(
                vehicleTrajectoryTime) if vehicleType[index] == "Car"]
            [carTrajectoryDistancePoint.append(element) for index,  element in enumerate(
                vehicleTrajectoryDistance) if vehicleType[index] == "Car"]

            [connectedVehicleTrajectoryTimePoint.append(element) for index, element in enumerate(
                vehicleTrajectoryTime) if vehicleType[index] == 0]
            [connectedVehicleTrajectoryDistancePoint.append(element) for index,  element in enumerate(
                vehicleTrajectoryDistance) if vehicleType[index] == 0]

        return evTrajectoryTimePoint, evTrajectoryDistancePoint, transitTrajectoryTimePoint, transitTrajectoryDistancePoint, truckTrajectoryTimePoint, truckTrajectoryDistancePoint, carTrajectoryTimePoint, carTrajectoryDistancePoint, connectedVehicleTrajectoryTimePoint, connectedVehicleTrajectoryDistancePoint

    def processBsmFile(self, BsmFile, desiredSignaGroup, distance):
        """
        modifies the dataframe when vehicle is on insideintersectionbox or outbound lane
        call the findUniqueVehicleIdInDataFrame() and getVehicleTrajectory() methods to get the vehicle trajectories
        """
        vehicleTrajectoryTimePoint = []
        vehicleTrajectoryDistancePoint = []
        vehicleType = []
        currentSignalGroup = 0

        dataframe = pd.read_csv(BsmFile)
        """
        Data Collector don't know current signal group information if a vehicle is not on inBound lane.
        Following section of the code is to deal with that situation.
        """
        for idx, row in dataframe.loc[:].iterrows():
            
            if row['current_signal_group'] == 0:
               dataframe['current_signal_group'][idx] = currentSignalGroup
               dataframe['trajectory_signal_group'][idx] = currentSignalGroup
            else:
                currentSignalGroup = row['current_signal_group']

        # dataframe.to_csv(BsmFile)
        """ end of the section """
                
        dataframe = dataframe.loc[dataframe["trajectory_signal_group"] == desiredSignaGroup]
        uniqueVehicleId = self.findUniqueVehicleIdInDataFrame(dataframe, desiredSignaGroup)

        for vehicleId in uniqueVehicleId:
            vehicleTrajectoryTimeList, vehicleTrajectoryDistanceList, vehicleTypeList = self.getVehicleTrajectory(
                vehicleId, dataframe,  distance)

            [vehicleTrajectoryTimePoint.append(element)
            for element in vehicleTrajectoryTimeList]

            [vehicleTrajectoryDistancePoint.append(
                element) for element in vehicleTrajectoryDistanceList]

            [vehicleType.append(element) for element in vehicleTypeList]

        return vehicleTrajectoryTimePoint, vehicleTrajectoryDistancePoint, vehicleType

    def findUniqueVehicleIdInDataFrame(self, bsmDf, desiredSignaGroup):
        """
        This method computes unique vehicle ID that are travelling along the desired signal group
        """
        uniqueVehicleId = []

        if not bsmDf.empty:
            for idx, row in bsmDf.loc[:].iterrows():
                if row['temporaryId'] not in uniqueVehicleId and row['trajectory_signal_group'] == desiredSignaGroup and row['timestamp_posix'] >= self.startTime and row['timestamp_posix'] <= self.endTime:
                    uniqueVehicleId.append(row['temporaryId'])

        return uniqueVehicleId


    def getVehicleTrajectory(self, uniqueVehicleId, dataframe, distance):
        """
        method to obtain vehicle trajectory for a specific vehicle ID from the dataframe
        if a vehicle is on inbound lane and trajectory time stamp is between the start time and end time of the diagram generation time, store that trajectory data
        if a vehicle is on intersection box or outbound lane and trajectory time stamp is between the start time and end time of the diagram generation time, 
            - consider 'dist_to_stopbar' column data to calutate the trajectory distance (calculate the difference between total distance along the path and inbound distance along the path)        
        """
        vehicleTrajectoryTimeList = []
        vehicleTrajectoryDistanceList = []
        vehicleTypeList = []
        inboundDistanceAlongPath = 0.0
        cumulativeTime = 0.0
        currentStateTime = self.startTime
        previousStartTime = currentStateTime
        temporaryDistance = 0.0
        bsmDf = dataframe.loc[dataframe["temporaryId"] == uniqueVehicleId]

        for idx, row in bsmDf.loc[:].iterrows():

            if row['position_on_map'] == "inbound" and row['timestamp_posix'] >= self.startTime and row['timestamp_posix'] <= self.endTime and row['timestamp_posix'] - previousStartTime >= 1.0:
                temporaryDistance = distance - row['dist_to_stopbar']
                vehicleTrajectoryDistanceList.append(temporaryDistance)
                inboundDistanceAlongPath = row['distance_along_path']
                cumulativeTime += row['timestamp_posix'] - previousStartTime
                vehicleTrajectoryTimeList.append(cumulativeTime)
                vehicleTypeList.append(row['type'])
                previousStartTime = row['timestamp_posix']

            elif (row['position_on_map'] == "insideIntersectionBox" or row['position_on_map'] == "outbound") and row['timestamp_posix'] >= self.startTime and row['timestamp_posix'] <= self.endTime and row['timestamp_posix'] - previousStartTime >= 1.0:
                vehicleTrajectoryDistanceList.append(
                    temporaryDistance + (row['distance_along_path'] - inboundDistanceAlongPath))
                cumulativeTime += row['timestamp_posix'] - previousStartTime
                vehicleTrajectoryTimeList.append(cumulativeTime)
                vehicleTypeList.append(row['type'])
                previousStartTime = row['timestamp_posix']

        return vehicleTrajectoryTimeList, vehicleTrajectoryDistanceList, vehicleTypeList
