import numpy as np
import pandas as pd
from matplotlib.patches import Rectangle
import matplotlib.pyplot as plt
from matplotlib import lines
from matplotlib.collections import PatchCollection
import json

from pandas.core.frame import DataFrame

ClearanceStatus = 1
GreenStatus = 0


def modifyPhaseStatusForDesiredSignalGroup(dataframe, desiredSignalGroupString):
    dataframe[[desiredSignalGroupString]] = dataframe[[
        desiredSignalGroupString]].replace('red', 1)
    dataframe[[desiredSignalGroupString]
              ] = dataframe[desiredSignalGroupString].replace('yellow', 1)
    dataframe[[desiredSignalGroupString]
              ] = dataframe[desiredSignalGroupString].replace('green', 0)

    return dataframe


def getStartAndEndTimeIndex(dataframe, startTime, endTime):
    startTimeIndexList = dataframe.index[dataframe['timestamp_posix'] == startTime].tolist(
    )
    endTimeIndexList = dataframe.index[dataframe['timestamp_posix'] == endTime].tolist(
    )

    if not bool(startTimeIndexList):
        startTimeIndexList = dataframe.index[(dataframe['timestamp_posix'] > startTime) & (
            dataframe['timestamp_posix'] < startTime+1)].tolist()
    if not bool(endTimeIndexList):
        endTimeIndexList = dataframe.index[(dataframe['timestamp_posix'] > endTime) & (
            dataframe['timestamp_posix'] < endTime+1)].tolist()

    startTimeIndex = startTimeIndexList[0]
    endTimeIndex = endTimeIndexList[0]

    return startTimeIndex, endTimeIndex


def getTimeAndPhaseStateList(dataframe, desiredSignalGroupString, startTimeIndex, endTimeIndex, currentState, currentStateTime):
    timeList = []
    phaseStateList = []
    # print("Start Time Index", startTimeIndex)
    # print("End Time Index", endTimeIndex)
    for index, row in dataframe.loc[startTimeIndex:endTimeIndex].iterrows():
        if row[desiredSignalGroupString] != currentState:
            timeList.append(row['timestamp_posix'] - currentStateTime)
            phaseStateList.append(currentState)
            currentState = row[desiredSignalGroupString]
            currentStateTime = row['timestamp_posix']

    return timeList, phaseStateList


def calculateGreenAndRedTime(timeList, phaseStateList, distance):
    greenRectangleStartPoint, clearanceRectangleStartPoint, greenRectangleTime, clearanceRectangleTime, distance_Green, distance_Clearance = ([
    ] for i in range(6))
    cumulativeTime = 0
    statusListIndex = 0

    if phaseStateList[0] == phaseStateList[-1]:
        del phaseStateList[-1]
        del timeList[-1]

    for data in range(0, len(timeList)):
        cumulativeTime += timeList[data]
        if phaseStateList[statusListIndex] == GreenStatus:
            clearanceRectangleStartPoint.append(cumulativeTime)
            greenRectangleTime.append(timeList[data])
            distance_Green.append(distance)

        elif phaseStateList[statusListIndex] == ClearanceStatus:
            greenRectangleStartPoint.append(cumulativeTime)
            clearanceRectangleTime.append(timeList[data])
            distance_Clearance.append(distance)
        statusListIndex += 1

    # The first rectangle will draw at time 0.0. Therefore, for the first rectangle a point has to be instered in the list and last element is required to be deleted.
    if phaseStateList[0] == GreenStatus:
        greenRectangleStartPoint.insert(0, 0)
        del greenRectangleStartPoint[-1]

    elif phaseStateList[0] == ClearanceStatus:
        clearanceRectangleStartPoint.insert(0, 0)
        del clearanceRectangleStartPoint[-1]

    return greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance


def timeSpaceDiagram(greenRectangleStartPoint, clearanceRectangleStartPoint, greenTime, clearanceTime, distance_Green, distance_Clearance, evTrajectoryTimePoint, evTrajectoryDistancePoint, transitTrajectoryTimePoint, transitTrajectoryDistancePoint, truckTrajectoryTimePoint, truckTrajectoryDistancePoint, carTrajectoryTimePoint, carTrajectoryDistancePoint, textString, xAxisRange):
    fig, ax1 = plt.subplots()

    ax1.set_xlabel('Time (s)', fontsize=24, fontweight='bold')
    ax1.set_ylabel('Distance (m)', fontsize=24, fontweight='bold')
    max_x_limit = xAxisRange-100
    plt.xlim([0, max_x_limit])
    plt.ylim([0, max(distance_Green)+400])
    plt.xticks(np.arange(0, xAxisRange-75, 50), fontsize=24)
    ax1.tick_params(axis='y',  labelsize=18)
    for axis in ['top', 'bottom', 'left', 'right']:
        ax1.spines[axis].set_linewidth(4)
    # ax1.set_yticks(ticks=np.arange(0, 100, 20),fontsize = 24)
    #newYlabel = ['-400','0','395','810','1225']
    # plt.gca().set_yticklabels(newYlabel)
    # plt.yticks([])
    req_phase_length = len(greenRectangleStartPoint)
    for i in range(0, req_phase_length):
        x = greenRectangleStartPoint[i]
        y = distance_Green[i]
        ax1.add_patch(Rectangle(
            (x, y), greenTime[i], 30, angle=0.0, color='green', linewidth=2,))
       

    req_phase_length = len(clearanceRectangleStartPoint)
    for i in range(0, req_phase_length):
        x = clearanceRectangleStartPoint[i]
        y = distance_Clearance[i]
        ax1.add_patch(Rectangle(
            (x, y), clearanceTime[i], 30, angle=0.0, color='red', linewidth=2))

    # plt.text(0,max(distance_Green)+25, textString, fontsize=10, bbox=dict(facecolor='white', alpha=0.5))

    #ax1.legend(loc='upper right', bbox_to_anchor=(1, 1), prop={"size":18})

    if len(evTrajectoryTimePoint)>0:
        ax1.scatter(evTrajectoryTimePoint, evTrajectoryDistancePoint, c="red",  linewidths=4,
                    marker=".",  edgecolor="none",  s=50, label='EmergencyVehicle Trajectory', zorder=2)

    # if len(transitTrajectoryTimePoint)>0:
    #     ax1.scatter(transitTrajectoryTimePoint, transitTrajectoryDistancePoint, c="limegreen",
    #                 linewidths=4, marker=".",  edgecolor="none",  s=50, label='Transit Trajectory', zorder=2)

    # if len(truckTrajectoryTimePoint) > 0:
    #     ax1.scatter(truckTrajectoryTimePoint, truckTrajectoryDistancePoint, c="blue",
    #                 linewidths=4, marker=".",  edgecolor="none",  s=50, label='Truck Trajectory', zorder=2)

    if len(carTrajectoryTimePoint)>0:
        ax1.scatter(carTrajectoryTimePoint, carTrajectoryDistancePoint, c="black",  linewidths=4,
                    marker=".",  edgecolor="none",  s=50, label='Passenger Vehicle Trajectory', zorder=2)

    ax1.legend(loc='upper right', prop={"size": 16})
    ax1.set_title("Time-Space Diagram", fontsize=20, fontweight='bold')
    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    plt.grid(color='black', linestyle='-', linewidth=0.5)
    plt.show()


def getSpatFileDirecyoryList(config):
    fileDirectoryList_SPaT = []
    intersectionName = []
    intersectionDistanceApart = []

    [fileDirectoryList_SPaT.append(fileDirectory)
     for fileDirectory in config["SPaTFileDirectory"]]
    [intersectionName.append(corridorName)
     for corridorName in config["IntersectionName"]]
    [intersectionDistanceApart.append(distance)
     for distance in config["IntersectionDistance"]]

    return fileDirectoryList_SPaT, intersectionName, intersectionDistanceApart


def processSpatFile(spatFile, desiredSignalGroupString, distance, startTime, endTime):
    df = pd.read_csv(spatFile)

    startTimeIndex, endTimeIndex = getStartAndEndTimeIndex(
        df, startTime, endTime)

    # spatDf = df.loc[((df["timestamp_posix"] >= startTime) & (df["timestamp_posix"] <= endTime))]

    df = modifyPhaseStatusForDesiredSignalGroup(df, desiredSignalGroupString)

    currentState = df[desiredSignalGroupString][startTimeIndex]
    timeList, phaseStateList = getTimeAndPhaseStateList(
        df, desiredSignalGroupString, startTimeIndex, endTimeIndex, currentState, startTime)

    # print("Time List is: ",timeList)
    # print("Phase State List is: ", phaseStateList)
    greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance = calculateGreenAndRedTime(
        timeList, phaseStateList, distance)

    return greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance


def getBsmFileDirecyoryList(config):
    fileDirectoryList_BSM = []
    [fileDirectoryList_BSM.append(fileDirectory)
     for fileDirectory in config["BsmFileDirectory"]]

    return fileDirectoryList_BSM


def findUniqueVehicleIdInDataFrame(bsmDf, desiredSignaGroup, startTime, endTime):
    uniqueVehicleId = []

    if not bsmDf.empty:
        for idx, row in bsmDf.loc[:].iterrows():
            if row['temporaryId'] not in uniqueVehicleId and row['trajectory_signal_group'] == desiredSignaGroup and row['timestamp_posix'] >= startTime and row['timestamp_posix'] <= endTime:
                uniqueVehicleId.append(row['temporaryId'])

    return uniqueVehicleId


def getVehicleTrajectory(uniqueVehicleId, dataframe, currentStateTime, distance, startTime, endTime):

    vehicleTrajectoryTimeList = []
    vehicleTrajectoryDistanceList = []
    vehicleTypeList = []
    temporaryDistanceAlongPath = -5.0
    inboundDistanceAlongPath = 0.0
    cumulativeTime = 0.0
    previousStartTime = currentStateTime
    temporaryDistance = 0.0
    bsmDf = dataframe.loc[dataframe["temporaryId"] == uniqueVehicleId]

    for idx, row in bsmDf.loc[:].iterrows():

        if row['position_on_map'] == "inbound" and row['timestamp_posix'] >= startTime and row['timestamp_posix'] <= endTime and row['timestamp_posix'] - previousStartTime >= 1.0:
            temporaryDistance = distance - row['dist_to_stopbar']
            vehicleTrajectoryDistanceList.append(temporaryDistance)
            inboundDistanceAlongPath = row['distance_along_path']
            temporaryDistanceAlongPath = row['distance_along_path']
            cumulativeTime += row['timestamp_posix'] - previousStartTime
            vehicleTrajectoryTimeList.append(cumulativeTime)
            vehicleTypeList.append(row['type'])
            previousStartTime = row['timestamp_posix']

        elif (row['position_on_map'] == "insideIntersectionBox" or row['position_on_map'] == "outbound") and row['timestamp_posix'] >= startTime and row['timestamp_posix'] <= endTime and row['timestamp_posix'] - previousStartTime >= 1.0:
            # print(row['position_on_map'], row['distance_along_path'], row['timestamp_posix'])
            vehicleTrajectoryDistanceList.append(
                temporaryDistance + (row['distance_along_path'] - inboundDistanceAlongPath))
            temporaryDistanceAlongPath = row['distance_along_path']
            cumulativeTime += row['timestamp_posix'] - previousStartTime
            vehicleTrajectoryTimeList.append(cumulativeTime)
            vehicleTypeList.append(row['type'])
            previousStartTime = row['timestamp_posix']

    return vehicleTrajectoryTimeList, vehicleTrajectoryDistanceList, vehicleTypeList


def processBsmFile(BsmFile, desiredSignaGroup, startTime, endTime, distance):
    vehicleTrajectoryTimePoint = []
    vehicleTrajectoryDistancePoint = []
    vehicleType = []
    dataframe = pd.read_csv(BsmFile)
    bsmDf = dataframe.loc[dataframe["trajectory_signal_group"]
                          == desiredSignaGroup]
    uniqueVehicleId = findUniqueVehicleIdInDataFrame(
        bsmDf, desiredSignaGroup, startTime, endTime)

    for vehicleId in uniqueVehicleId:
        vehicleTrajectoryTimeList, vehicleTrajectoryDistanceList, vehicleTypeList = getVehicleTrajectory(
            vehicleId, dataframe, startTime, distance, startTime, endTime,)

        [vehicleTrajectoryTimePoint.append(element)
         for element in vehicleTrajectoryTimeList]

        [vehicleTrajectoryDistancePoint.append(
            element) for element in vehicleTrajectoryDistanceList]

        [vehicleType.append(element) for element in vehicleTypeList]

    return vehicleTrajectoryTimePoint, vehicleTrajectoryDistancePoint, vehicleType


def main():
    # Read the config file into a json object:
    configFile = open("configuration.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()

    startTime = config["StartTimeOfDiagram"]
    endTime = config["EndTimeOfDiagram"]
    desiredSignaGroup = 0.0
    signalGroup = []
    [signalGroup.append(phase) for phase in config["DeisredSignalGroup"]]

    greenTimeStartPoint, clearanceTimeStartPoint, greenTime, clearanceTime, intersectionDistance_Green, intersectionDistance_Clearance = ([
    ] for i in range(6))
    evTrajectoryTimePoint, evTrajectoryDistancePoint = ([] for i in range(2))
    transitTrajectoryTimePoint, transitTrajectoryDistancePoint = (
        [] for i in range(2))
    truckTrajectoryTimePoint, truckTrajectoryDistancePoint = (
        [] for i in range(2))
    carTrajectoryTimePoint, carTrajectoryDistancePoint = ([] for i in range(2))

    xAxisRange = endTime - startTime
    vehicleTrajectoryDistanceList = {}
    """
    Process SPaT file
    """
    fileDirectoryList_SPaT, intersectionName, intersectionDistanceApart = getSpatFileDirecyoryList(
        config)

    for index, spatFile in enumerate(fileDirectoryList_SPaT):
        print("intersection name is: ", intersectionName[index])
        desiredSignaGroup = signalGroup[index]
        desiredSignalGroupString = "v" + str(desiredSignaGroup) + "_currState"
        greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance = processSpatFile(
            spatFile, desiredSignalGroupString, intersectionDistanceApart[index], startTime, endTime)

        [greenTimeStartPoint.append(element)
         for element in greenRectangleStartPoint]
        [greenTime.append(element) for element in greenRectangleTime]
        [clearanceTimeStartPoint.append(element)
         for element in clearanceRectangleStartPoint]
        [clearanceTime.append(element) for element in clearanceRectangleTime]
        [intersectionDistance_Green.append(element)
         for element in distance_Green]
        [intersectionDistance_Clearance.append(
            element) for element in distance_Clearance]

    # print("Green Rectangle Start Point are: ",greenTimeStartPoint)
    # print("Green Time is: ", greenTime)

    # print("Clearance Rectangle Start Point are: ",clearanceTimeStartPoint)
    # print("Clearance Time is: ", clearanceTime)

    # print("Intersection Distance Green is: ",intersectionDistance_Green)
    # print("Intersection Distance Clearance is: ",intersectionDistance_Clearance)
    # for element in vehicleTrajectoryDistanceList:
    #         vehicleTrajectoryDictionary["Distance"].append(element)
    """
    Process BSM file
    """
    fileDirectoryList_BSM = getBsmFileDirecyoryList(config)
    for index, bsmFile in enumerate(fileDirectoryList_BSM):
        desiredSignaGroup = signalGroup[index]
        vehicleTrajectoryTime, vehicleTrajectoryDistance, vehicleType = processBsmFile(
            bsmFile, desiredSignaGroup, startTime, endTime, intersectionDistanceApart[index])

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

    """
    Plot Time-Space Diagram
    """
    timeSpaceDiagram(greenTimeStartPoint, clearanceTimeStartPoint,
                     greenTime, clearanceTime, intersectionDistance_Green, intersectionDistance_Clearance, evTrajectoryTimePoint, evTrajectoryDistancePoint, transitTrajectoryTimePoint, transitTrajectoryDistancePoint, truckTrajectoryTimePoint, truckTrajectoryDistancePoint, carTrajectoryTimePoint, carTrajectoryDistancePoint, "Time-Space Diagram", xAxisRange)


if __name__ == "__main__":
    main()