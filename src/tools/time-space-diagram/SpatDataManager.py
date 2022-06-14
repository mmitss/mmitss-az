"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

SpatDataManager.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
The methods available from this class are the following:
- getSpatConfigData(): method to get config data for the SPaT files
- manageSpatData(): method to manage list for green time start point, clearance time start point, green time, clearance time, intersection distance for green time, intersection distance for clearance 
- processSpatFile(spatFile, distance): method to process all the SPaT files sequentially
- getStartAndEndTimeIndex(dataframe): method to get index for the start time and end time of the diagram
- modifyPhaseStatusForDesiredSignalGroup(dataframe): method to convert  the phase state from string to numeric 
- getGreenAndClearanceTime(dataframe, startTimeIndex, endTimeIndex, distance): method to get list for green rectangle start point, green rectangle time, clearance rectangle start point, clearance rectangle time
***************************************************************************************
"""

import pandas as pd

class SpatDataManager:
    def __init__(self, config):
        self.config = config
        self.startTime = 0.0
        self.endTime = 0.0
        self.desiredSignalGroupString = ""
        self.clearanceStatus = 1
        self.greenStatus = 0
        self.fileDirectoryList_SPaT = []
        self.intersectionName = []
        self.intersectionDistanceApart = []
        self.signalGroup = []
        
        

    def getSpatConfigData(self):
        """
        method to get config data for the SPaT files
        """
        [self.fileDirectoryList_SPaT.append(fileDirectory) for fileDirectory in self.config["SPaTFileDirectory"]]
        [self.intersectionName.append(corridorName) for corridorName in self.config["IntersectionName"]]
        [self.intersectionDistanceApart.append(distance) for distance in self.config["IntersectionDistance"]]
        [self.signalGroup.append(phase) for phase in self.config["DeisredSignalGroup"]]
        self.startTime = self.config["StartTimeOfDiagram"]
        self.endTime = self.config["EndTimeOfDiagram"]

    def manageSpatData(self):
        """
        manages list for green time start point, clearance time start point, green time, clearance time, intersection distance for green time, intersection distance for clearance 
        """
        greenTimeStartPoint, clearanceTimeStartPoint, greenTime, clearanceTime, intersectionDistance_Green, intersectionDistance_Clearance = ([
        ] for i in range(6))
        
        self.getSpatConfigData()

        for index, spatFile in enumerate(self.fileDirectoryList_SPaT):
            desiredSignaGroup = self.signalGroup[index]
            self.desiredSignalGroupString = "v" + str(desiredSignaGroup) + "_currState"
            greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance = self.processSpatFile(
                spatFile, self.intersectionDistanceApart[index])

            [greenTimeStartPoint.append(element)
             for element in greenRectangleStartPoint]
            [greenTime.append(element) for element in greenRectangleTime]
            [clearanceTimeStartPoint.append(element)
             for element in clearanceRectangleStartPoint]
            [clearanceTime.append(element)
             for element in clearanceRectangleTime]
            [intersectionDistance_Green.append(element)
             for element in distance_Green]
            [intersectionDistance_Clearance.append(
                element) for element in distance_Clearance]

        return greenTimeStartPoint, clearanceTimeStartPoint, greenTime, clearanceTime, intersectionDistance_Green, intersectionDistance_Clearance


    def processSpatFile(self, spatFile, distance):
        """
        method to process all the SPaT files sequentially
        """
        df = pd.read_csv(spatFile)

        startTimeIndex, endTimeIndex = self.getStartAndEndTimeIndex(df)

        df = self.modifyPhaseStatusForDesiredSignalGroup(df)
        
        greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance = self.getGreenAndClearanceTime(df, startTimeIndex, endTimeIndex, distance)

        return greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance


    def getStartAndEndTimeIndex(self, dataframe):
        """
        method to get index for the start time and end time of the diagram
        """
        startTimeIndexList = dataframe.index[dataframe['timestamp_posix'] == self.startTime].tolist()
        endTimeIndexList = dataframe.index[dataframe['timestamp_posix'] == self.endTime].tolist()

        if not bool(startTimeIndexList):
            startTimeIndexList = dataframe.index[(dataframe['timestamp_posix'] > self.startTime) & (
                dataframe['timestamp_posix'] < self.startTime+1)].tolist()
        if not bool(endTimeIndexList):
            endTimeIndexList = dataframe.index[(dataframe['timestamp_posix'] > self.endTime) & (
                dataframe['timestamp_posix'] < self.endTime+1)].tolist()

        startTimeIndex = startTimeIndexList[0]
        endTimeIndex = endTimeIndexList[0]

        return startTimeIndex, endTimeIndex


    def modifyPhaseStatusForDesiredSignalGroup(self, dataframe):
        """
        method to convert  the phase state from string to numeric 
        """
        dataframe[self.desiredSignalGroupString] = dataframe[self.desiredSignalGroupString].replace('red', 1)
        dataframe[self.desiredSignalGroupString] = dataframe[self.desiredSignalGroupString].replace('yellow', 1)
        dataframe[self.desiredSignalGroupString] = dataframe[self.desiredSignalGroupString].replace('green', 0)

        return dataframe

    def getGreenAndClearanceTime(self, dataframe, startTimeIndex, endTimeIndex, distance):
        """
        method to get list for green rectangle start point, green rectangle time, clearance rectangle start point, clearance rectangle time
        """
        greenRectangleStartPoint, clearanceRectangleStartPoint, greenRectangleTime, clearanceRectangleTime, distance_Green, distance_Clearance = ([
        ] for i in range(6))

        currentPhaseStatusStartTime = self.startTime
        currentPhaseStatus = dataframe[self.desiredSignalGroupString][startTimeIndex]

        if currentPhaseStatus == self.greenStatus:
            greenRectangleStartPoint.append(0.0)

        else:
            clearanceRectangleStartPoint.append(0.0)

        for index, row in dataframe.loc[startTimeIndex:endTimeIndex].iterrows():
            if row[self.desiredSignalGroupString] != currentPhaseStatus:
                if row[self.desiredSignalGroupString] == self.greenStatus:
                    greenRectangleStartPoint.append(row['timestamp_posix'] - self.startTime)
                    clearanceRectangleTime.append(row['timestamp_posix'] - currentPhaseStatusStartTime)
                    distance_Clearance.append(distance)
                    currentPhaseStatus = row[self.desiredSignalGroupString]
                    currentPhaseStatusStartTime = row['timestamp_posix']

                else: 
                    
                    clearanceRectangleStartPoint.append(row['timestamp_posix'] - self.startTime)
                    greenRectangleTime.append(row['timestamp_posix'] - currentPhaseStatusStartTime)
                    distance_Green.append(distance)
                    currentPhaseStatus = row[self.desiredSignalGroupString]
                    currentPhaseStatusStartTime = row['timestamp_posix']

        if len(greenRectangleStartPoint) > len(greenRectangleTime):
            greenRectangleTime.append(dataframe['timestamp_posix'].iloc[-1] - currentPhaseStatusStartTime + 25)
            distance_Green.append(distance)

        if len(clearanceRectangleStartPoint) > len(clearanceRectangleTime):
            clearanceRectangleTime.append(dataframe['timestamp_posix'].iloc[-1] - currentPhaseStatusStartTime + 25)
            distance_Clearance.append(distance)


        return greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance

    def getTimeAndPhaseStateList(self, dataframe, startTimeIndex, endTimeIndex):
        """
        """
        timeList = []
        phaseStateList = []
        currentState = dataframe[self.desiredSignalGroupString][startTimeIndex]
        currentStateTime = self.startTime

        for index, row in dataframe.loc[startTimeIndex:endTimeIndex].iterrows():
            if row[self.desiredSignalGroupString] != currentState:
                timeList.append(row['timestamp_posix'] - currentStateTime)
                phaseStateList.append(currentState)
                currentState = row[self.desiredSignalGroupString]
                currentStateTime = row['timestamp_posix']

        return timeList, phaseStateList

    def calculateGreenAndRedTime(self, timeList, phaseStateList, distance):
        greenRectangleStartPoint, clearanceRectangleStartPoint, greenRectangleTime, clearanceRectangleTime, distance_Green, distance_Clearance = ([
        ] for i in range(6))
        cumulativeTime = 0


        if phaseStateList[0] == phaseStateList[-1]:
            del phaseStateList[-1]
            del timeList[-1]

        for data in range(0, len(timeList)):
            cumulativeTime += timeList[data]
            if phaseStateList[data] == self.greenStatus:
                clearanceRectangleStartPoint.append(cumulativeTime)
                greenRectangleTime.append(timeList[data])
                distance_Green.append(distance)

            elif phaseStateList[data] == self.clearanceStatus:
                greenRectangleStartPoint.append(cumulativeTime)
                clearanceRectangleTime.append(timeList[data])
                distance_Clearance.append(distance)
            

        # The first rectangle will draw at time 0.0. Therefore, for the first rectangle a point has to be instered in the list and last element is required to be deleted.
        if phaseStateList[0] == self.greenStatus:
            greenRectangleStartPoint.insert(0, 0)
            del greenRectangleStartPoint[-1]

        elif phaseStateList[0] == self.clearanceStatus:
            clearanceRectangleStartPoint.insert(0, 0)
            del clearanceRectangleStartPoint[-1]

        return greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance