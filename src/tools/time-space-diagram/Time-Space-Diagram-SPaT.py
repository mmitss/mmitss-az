import numpy as np
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
from matplotlib import lines
from matplotlib.collections import PatchCollection
import json

ClearanceStatus = 1
GreenStatus = 0

def modifyDataframeForCoordinatePhaseStatus(dataframe):
    dataframe[['v2_currState']] = dataframe[['v2_currState']].replace('red',1)
    dataframe[['v2_currState']] = dataframe['v2_currState'].replace('yellow',1)
    dataframe[['v2_currState']] = dataframe['v2_currState'].replace('green',0)
    
    return dataframe

def getStartAndEndTimeIndex(dataframe, startTime, endTime):
    startTimeIndexList = dataframe.index[dataframe['timestamp_posix'] == startTime].tolist()
    endTimeIndexList = dataframe.index[dataframe['timestamp_posix'] == endTime].tolist()

    if not bool(startTimeIndexList):
        startTimeIndexList = dataframe.index[(dataframe['timestamp_posix'] > startTime) & (dataframe['timestamp_posix'] < startTime+1)].tolist()
    if not bool(endTimeIndexList):
         endTimeIndexList = dataframe.index[(dataframe['timestamp_posix'] > endTime) & (dataframe['timestamp_posix'] < endTime+1)].tolist()

    startTimeIndex = startTimeIndexList[0]
    endTimeIndex = endTimeIndexList[0]
     
    return startTimeIndex, endTimeIndex

def getTimeAndStatusList(dataframe, startTimeIndex, endTimeIndex, currentState, currentStateTime):
    timeList= []
    statusList = []
    print("Start Time Index", startTimeIndex)
    print("End Time Index", endTimeIndex)
    for idx, row in dataframe.loc[startTimeIndex:endTimeIndex].iterrows():
        if row['v2_currState'] != currentState:
            #print(row['timestamp_posix'], row['v2_currState'])
            timeList.append(row['timestamp_posix'] - currentStateTime)
            statusList.append(currentState)
            currentState = row['v2_currState']
            currentStateTime = row['timestamp_posix']
            
    return timeList, statusList

def calculateGreenAndRedTime(timeList, statusList, distance):
    greenRectangleStartPoint = []
    clearanceRectangleStartPoint = [] 
    greenRectangleTime = [] 
    clearanceRectangleTime = []
    distance_Green = [] 
    distance_Clearance= []
    cumulativeTime = 0
    statusListIndex = 0
    
    if statusList[0] == statusList[-1]:
        del statusList[-1]
        del timeList[-1]
           
    for data in range(0,len(timeList)):
        cumulativeTime += timeList[data]
        if statusList[statusListIndex] == GreenStatus:
            clearanceRectangleStartPoint.append(cumulativeTime)
            greenRectangleTime.append(timeList[data])
            distance_Green.append(distance)
           
        elif statusList[statusListIndex] == ClearanceStatus:
            greenRectangleStartPoint.append(cumulativeTime)
            clearanceRectangleTime.append(timeList[data])
            distance_Clearance.append(distance)
        statusListIndex += 1
    
    #The first rectangle will draw at time 0.0. Therefore, for the first rectangle a point has to be instered in the list and last element is required to be deleted. 
    if statusList[0] == GreenStatus:
        greenRectangleStartPoint.insert(0,0)
        del greenRectangleStartPoint[-1]
        
    elif statusList[0] == ClearanceStatus:
        clearanceRectangleStartPoint.insert(0,0)
        del clearanceRectangleStartPoint[-1]    
        
    return greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance

def timeSpaceDiagram(greenRectangleStartPoint, clearanceRectangleStartPoint, greenTime, clearanceTime, distance_Green, distance_Clearance, textString, xAxisRange):
    fig, ax1 = plt.subplots()
    #fig = plt.figure()
    #ax1 = fig.add_subplot(111, aspect='equal')
    
    ax1.set_xlabel('Time (s)',fontsize=24,fontweight = 'bold')
    ax1.set_ylabel('Distance (m)', fontsize=24,fontweight = 'bold')
    max_x_limit = xAxisRange-100
    plt.xlim([0, max_x_limit])
    plt.ylim([0, max(distance_Green)+200])
    plt.xticks(np.arange(0, xAxisRange-75, 25),fontsize = 24)
    ax1.tick_params(axis='y',  labelsize=18)
    for axis in ['top','bottom','left','right']:
            ax1.spines[axis].set_linewidth(4)
    # ax1.set_yticks(ticks=np.arange(0, 100, 20),fontsize = 24)
    #newYlabel = ['-400','0','395','810','1225']
    #plt.gca().set_yticklabels(newYlabel)
    # plt.yticks([])
    patches = []
    req_phase_length = len(greenRectangleStartPoint)
    for i in range(0, req_phase_length):
        x = greenRectangleStartPoint[i]
        y = distance_Green[i]
        if i==0:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y), greenTime[i], 30, angle = 0.0, color = 'green', linewidth = 2, label = 'Green Time of Coordinated Phases'))
        else:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y), greenTime[i], 30, angle = 0.0, color = 'green', linewidth = 2))
        
    patches = []
    req_phase_length = len(clearanceRectangleStartPoint)
    for i in range(0, req_phase_length):
        x = clearanceRectangleStartPoint[i]
        y = distance_Clearance[i]
        ax1.add_patch(matplotlib.patches.Rectangle((x, y), clearanceTime[i], 30, angle = 0.0, color = 'red', linewidth = 2))

    
    # plt.text(0,max(distance_Green)+25, textString, fontsize=10, bbox=dict(facecolor='white', alpha=0.5))
    
    #ax1.legend(loc='upper right', bbox_to_anchor=(1, 1), prop={"size":18})
    
    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    plt.grid(color='black', linestyle='-', linewidth= 0.5)
    plt.show()

def main():
    # Read the config file into a json object:
    configFile = open("configuration.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()
    noOfIntersection = config["NoOfIntersection"]
    startTime = config["StartTimeOfDiagram"]
    endTime = config["EndTimeOfDiagram"]
    fileDirectoryList_SPaT = []
    intersectionName = []
    intersectionDistanceApart = []
    
    greenTimeStartPoint = []
    clearanceTimeStartPoint = []
    greenTime = []
    clearanceTime = []
    intersectionDistance_Green = [] 
    intersectionDistance_Clearance = []
    count = 0
    j = 0
    xAxisRange = endTime - startTime
    while (count < noOfIntersection):
        filePath = config["SPaTFileDirectory"][count]
        fileDirectoryList_SPaT.append(filePath)
        corridorName = config["IntersectionName"][count]
        intersectionName.append(corridorName)
        distanceApart = config["IntersectionDistance"][count]
        intersectionDistanceApart.append(distanceApart)
        count = count + 1

    for i in fileDirectoryList_SPaT:
        df = pd.read_csv(i)
        print("intersection name is: ", intersectionName[j])
        distance = intersectionDistanceApart[j]
        startTimeIndex, endTimeIndex = getStartAndEndTimeIndex(df, startTime, endTime)
        
        df = modifyDataframeForCoordinatePhaseStatus(df)

        currentState = df['v2_currState'][startTimeIndex]
        timeList, statusList = getTimeAndStatusList(df, startTimeIndex, endTimeIndex, currentState, startTime)
        
        print("Time List is: ",timeList)
        print("Status List is: ", statusList)
        greenRectangleStartPoint, greenRectangleTime, distance_Green, clearanceRectangleStartPoint, clearanceRectangleTime, distance_Clearance = calculateGreenAndRedTime(timeList, statusList, distance)
        
        print("Length of Green Rectangle Start Point", len(greenRectangleStartPoint))
        print("Length of Distance Green", len(distance_Green))
        for element in greenRectangleStartPoint:
            greenTimeStartPoint.append(element)
            
        for element in greenRectangleTime:
            greenTime.append(element)
            
        for element in clearanceRectangleStartPoint:
            clearanceTimeStartPoint.append(element)
            
        for element in clearanceRectangleTime:
            clearanceTime.append(element)
        
        for element in distance_Green:
            intersectionDistance_Green.append(element)
        
        for element in distance_Clearance:
            intersectionDistance_Clearance.append(element)
        
        
        print("Green Rectangle Start Point are: ",greenTimeStartPoint)
        print("Green Time is: ", greenTime)
        
        print("Clearance Rectangle Start Point are: ",clearanceTimeStartPoint)
        print("Clearance Time is: ", clearanceTime)
        
        print("Intersection Distance Green is: ",intersectionDistance_Green)
        #print("Intersection Distance Clearance is: ",intersectionDistance_Clearance)
        j += 1
    textString = "Time-Space Diagram"
    #print (textString)
    timeSpaceDiagram(greenTimeStartPoint, clearanceTimeStartPoint,
                         greenTime, clearanceTime, intersectionDistance_Green, intersectionDistance_Clearance, textString, xAxisRange)
        
if __name__ == "__main__":
    main()