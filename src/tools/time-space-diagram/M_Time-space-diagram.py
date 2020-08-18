import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
import json

def getStartingPhases(fileName):
    with open(fileName) as f:
        line = f.readline()
    f.close()
    return line


def getInitToPhasesAndElaspedGreenTime(fileName):
    with open(fileName) as f:
        for i, line in enumerate(f):
            if i == 1:
                break
        init1, init2, grn1, grn2 = line.split()
    f.close()
    return float(init1), float(init2), float(grn1), float(grn2)


def getSignalPhaseDuration(fileName):
    with open(fileName) as f:
        for i, line in enumerate(f):
            # Read ringwise left critical phase duration from results.txt file for cycle1
            if i == 2:
                durationOfP1C1, durationOfP2C1, durationOfP3C1, durationOfP4C1, durationOfP5C1, durationOfP6C1, durationOfP7C1, durationOfP8C1 = line.split()
                leftCriticalPhaseDurationOfRing1 = [float(durationOfP1C1), float(
                    durationOfP2C1), float(durationOfP3C1), float(durationOfP4C1)]
                leftCriticalPhaseDurationOfRing2 = [float(durationOfP5C1), float(
                    durationOfP6C1), float(durationOfP7C1), float(durationOfP8C1)]
            # Read ringwise right critical phase duration from results.txt file for cycle1
            if i == 5:
                durationOfP1C1, durationOfP2C1, durationOfP3C1, durationOfP4C1, durationOfP5C1, durationOfP6C1, durationOfP7C1, durationOfP8C1 = line.split()
                rightCriticalPhaseDurationOfRing1 = [float(durationOfP1C1), float(
                    durationOfP2C1), float(durationOfP3C1), float(durationOfP4C1)]
                rightCriticalPhaseDurationOfRing2 = [float(durationOfP5C1), float(
                    durationOfP6C1), float(durationOfP7C1), float(durationOfP8C1)]
            # Read ringwise left critical green time from results.txt file for cycle1
            if i == 8:
                greenTimeOfP1C1, greenTimeOfP2C1, greenTimeOfP3C1, greenTimeOfP4C1, greenTimeOfP5C1, greenTimeOfP6C1, greenTimeOfP7C1, greenTimeOfP8C1 = line.split()
                leftCriticalGreenTimeOfRing1 = [float(greenTimeOfP1C1), float(
                    greenTimeOfP2C1), float(greenTimeOfP3C1), float(greenTimeOfP4C1)]
                leftCriticalGreenTimeOfRing2 = [float(greenTimeOfP5C1), float(
                    greenTimeOfP6C1), float(greenTimeOfP7C1), float(greenTimeOfP8C1)]
            # Read ringwise right critical green time from results.txt file for cycle1
            if i == 11:
                greenTimeOfP1C1, greenTimeOfP2C1, greenTimeOfP3C1, greenTimeOfP4C1, greenTimeOfP5C1, greenTimeOfP6C1, greenTimeOfP7C1, greenTimeOfP8C1 = line.split()
                rightCriticalGreenTimeOfRing1 = [float(greenTimeOfP1C1), float(
                    greenTimeOfP2C1), float(greenTimeOfP3C1), float(greenTimeOfP4C1)]
                rightCriticalGreenTimeOfRing2 = [float(greenTimeOfP5C1), float(
                    greenTimeOfP6C1), float(greenTimeOfP7C1), float(greenTimeOfP8C1)]
                break
    f.close()

    # All the null values will be deleted
    unwanted_num = {0.0}
    leftCriticalPhaseDurationOfRing1 = [
        ele for ele in leftCriticalPhaseDurationOfRing1 if ele not in unwanted_num]
    leftCriticalPhaseDurationOfRing2 = [
        ele for ele in leftCriticalPhaseDurationOfRing2 if ele not in unwanted_num]
    rightCriticalPhaseDurationOfRing1 = [
        ele for ele in rightCriticalPhaseDurationOfRing1 if ele not in unwanted_num]
    rightCriticalPhaseDurationOfRing2 = [
        ele for ele in rightCriticalPhaseDurationOfRing2 if ele not in unwanted_num]
    leftCriticalGreenTimeOfRing1 = [
        ele for ele in leftCriticalGreenTimeOfRing1 if ele not in unwanted_num]
    leftCriticalGreenTimeOfRing2 = [
        ele for ele in leftCriticalGreenTimeOfRing2 if ele not in unwanted_num]
    rightCriticalGreenTimeOfRing1 = [
        ele for ele in rightCriticalGreenTimeOfRing1 if ele not in unwanted_num]
    rightCriticalGreenTimeOfRing2 = [
        ele for ele in rightCriticalGreenTimeOfRing2 if ele not in unwanted_num]

    # leftCriticalPhaseDurationOfRing1.remove(0.0)
    # leftCriticalPhaseDurationOfRing2.remove(0.0)
    # rightCriticalPhaseDurationOfRing1.remove(0.0)
    # rightCriticalPhaseDurationOfRing2.remove(0.0)
    # leftCriticalGreenTimeOfRing1.remove(0.0)
    # leftCriticalGreenTimeOfRing2.remove(0.0)
    # rightCriticalGreenTimeOfRing1.remove(0.0)
    # rightCriticalGreenTimeOfRing2.remove(0.0)

    # For the starting phase green time can be zero (if the phase is already served for minimum green time).
    # It is required to append 0.0 in the beginning of the list
    if(len(leftCriticalPhaseDurationOfRing1) != len(leftCriticalGreenTimeOfRing1)):
        leftCriticalGreenTimeOfRing1.insert(0, 0.0)

    if(len(leftCriticalPhaseDurationOfRing2) != len(leftCriticalGreenTimeOfRing2)):
        leftCriticalGreenTimeOfRing2.insert(0, 0.0)

    if(len(rightCriticalPhaseDurationOfRing1) != len(rightCriticalGreenTimeOfRing1)):
        rightCriticalGreenTimeOfRing1.insert(0, 0.0)

    if(len(rightCriticalPhaseDurationOfRing2) != len(rightCriticalGreenTimeOfRing2)):
        rightCriticalGreenTimeOfRing2.insert(0, 0.0)

    # Read ringwise left and right critical phase duration and green time from results.txt file for cycle 2 and 3
    with open(fileName) as f:
        for i, line in enumerate(f):
            if i < 3:
                continue
            if i == 3:
                durationOfP1C2, durationOfP2C2, durationOfP3C2, durationOfP4C2, durationOfP5C2, durationOfP6C2, durationOfP7C2, durationOfP8C2 = line.split()

                leftCricticalPhaseDurationOfRing1Cycle2 = [float(durationOfP1C2), float(
                    durationOfP2C2), float(durationOfP3C2), float(durationOfP4C2)]
                leftCricticalPhaseDurationOfRing2Cycle2 = [float(durationOfP5C2), float(
                    durationOfP6C2), float(durationOfP7C2), float(durationOfP8C2)]

                leftCriticalPhaseDurationOfRing1.extend(
                    leftCricticalPhaseDurationOfRing1Cycle2)
                leftCriticalPhaseDurationOfRing2.extend(
                    leftCricticalPhaseDurationOfRing2Cycle2)

            if i == 4:
                durationOfP1C3, durationOfP2C3, durationOfP3C3, durationOfP4C3, durationOfP5C3, durationOfP6C3, durationOfP7C3, durationOfP8C3 = line.split()
                leftCricticalPhaseDurationOfRing1Cycle3 = [float(durationOfP1C3), float(
                    durationOfP2C3), float(durationOfP3C3), float(durationOfP4C3)]
                leftCricticalPhaseDurationOfRing2Cycle3 = [float(durationOfP5C3), float(
                    durationOfP6C3), float(durationOfP7C3), float(durationOfP8C3)]

                leftCriticalPhaseDurationOfRing1.extend(
                    leftCricticalPhaseDurationOfRing1Cycle3)
                leftCriticalPhaseDurationOfRing2.extend(
                    leftCricticalPhaseDurationOfRing2Cycle3)

            if i == 5:
                continue

            if i == 6:
                durationOfP1C2, durationOfP2C2, durationOfP3C2, durationOfP4C2, durationOfP5C2, durationOfP6C2, durationOfP7C2, durationOfP8C2 = line.split()

                rightCricticalPhaseDurationOfRing1Cycle2 = [float(durationOfP1C2), float(
                    durationOfP2C2), float(durationOfP3C2), float(durationOfP4C2)]
                rightCricticalPhaseDurationOfRing2Cycle2 = [float(durationOfP5C2), float(
                    durationOfP6C2), float(durationOfP7C2), float(durationOfP8C2)]

                rightCriticalPhaseDurationOfRing1.extend(
                    rightCricticalPhaseDurationOfRing1Cycle2)
                rightCriticalPhaseDurationOfRing2.extend(
                    rightCricticalPhaseDurationOfRing2Cycle2)

            if i == 7:
                durationOfP1C3, durationOfP2C3, durationOfP3C3, durationOfP4C3, durationOfP5C3, durationOfP6C3, durationOfP7C3, durationOfP8C3 = line.split()

                rightCricticalPhaseDurationOfRing1Cycle3 = [float(durationOfP1C3), float(
                    durationOfP2C3), float(durationOfP3C3), float(durationOfP4C3)]
                rightCricticalPhaseDurationOfRing2Cycle3 = [float(durationOfP5C3), float(
                    durationOfP6C3), float(durationOfP7C3), float(durationOfP8C3)]

                rightCriticalPhaseDurationOfRing1.extend(
                    rightCricticalPhaseDurationOfRing1Cycle3)
                rightCriticalPhaseDurationOfRing2.extend(
                    rightCricticalPhaseDurationOfRing2Cycle3)

            if i == 8:
                continue

            if i == 9:
                greenTimeOfP1C2, greenTimeOfP2C2, greenTimeOfP3C2, greenTimeOfP4C2, greenTimeOfP5C2, greenTimeOfP6C2, greenTimeOfP7C2, greenTimeOfP8C2 = line.split()

                leftCriticalGreenTimeOfRing1Cycle2 = [float(greenTimeOfP1C2), float(
                    greenTimeOfP2C2), float(greenTimeOfP3C2), float(greenTimeOfP4C2)]
                leftCriticalGreenTimeOfRing2Cycle2 = [float(greenTimeOfP5C2), float(
                    greenTimeOfP6C2), float(greenTimeOfP7C2), float(greenTimeOfP8C2)]

                leftCriticalGreenTimeOfRing1.extend(
                    leftCriticalGreenTimeOfRing1Cycle2)
                leftCriticalGreenTimeOfRing2.extend(
                    leftCriticalGreenTimeOfRing2Cycle2)

            if i == 10:
                greenTimeOfP1C3, greenTimeOfP2C3, greenTimeOfP3C3, greenTimeOfP4C3, greenTimeOfP5C3, greenTimeOfP6C3, greenTimeOfP7C3, greenTimeOfP8C3 = line.split()

                leftCriticalGreenTimeOfRing1Cycle3 = [float(greenTimeOfP1C3), float(
                    greenTimeOfP2C3), float(greenTimeOfP3C3), float(greenTimeOfP4C3)]
                leftCriticalGreenTimeOfRing2Cycle3 = [float(greenTimeOfP5C3), float(
                    greenTimeOfP6C3), float(greenTimeOfP7C3), float(greenTimeOfP8C3)]

                leftCriticalGreenTimeOfRing1.extend(
                    leftCriticalGreenTimeOfRing1Cycle3)
                leftCriticalGreenTimeOfRing2.extend(
                    leftCriticalGreenTimeOfRing2Cycle3)

            if i == 11:
                continue

            if i == 12:
                greenTimeOfP1C2, greenTimeOfP2C2, greenTimeOfP3C2, greenTimeOfP4C2, greenTimeOfP5C2, greenTimeOfP6C2, greenTimeOfP7C2, greenTimeOfP8C2 = line.split()

                rightCriticalGreenTimeOfRing1Cycle2 = [float(greenTimeOfP1C2), float(
                    greenTimeOfP2C2), float(greenTimeOfP3C2), float(greenTimeOfP4C2)]
                rightCriticalGreenTimeOfRing2Cycle2 = [float(greenTimeOfP5C2), float(
                    greenTimeOfP6C2), float(greenTimeOfP7C2), float(greenTimeOfP8C2)]

                rightCriticalGreenTimeOfRing1.extend(
                    rightCriticalGreenTimeOfRing1Cycle2)
                rightCriticalGreenTimeOfRing2.extend(
                    rightCriticalGreenTimeOfRing2Cycle2)

            if i == 13:
                greenTimeOfP1C3, greenTimeOfP2C3, greenTimeOfP3C3, greenTimeOfP4C3, greenTimeOfP5C3, greenTimeOfP6C3, greenTimeOfP7C3, greenTimeOfP8C3 = line.split()

                rightCriticalGreenTimeOfRing1Cycle3 = [float(greenTimeOfP1C3), float(
                    greenTimeOfP2C3), float(greenTimeOfP3C3), float(greenTimeOfP4C3)]
                rightCriticalGreenTimeOfRing2Cycle3 = [float(greenTimeOfP5C3), float(
                    greenTimeOfP6C3), float(greenTimeOfP7C3), float(greenTimeOfP8C3)]

                rightCriticalGreenTimeOfRing1.extend(
                    rightCriticalGreenTimeOfRing1Cycle3)
                rightCriticalGreenTimeOfRing2.extend(
                    rightCriticalGreenTimeOfRing2Cycle3)

            if i == 14:
                break
    f.close()
    
    # All the null values (for third cycle few phases phase duartion and green time will be zero, since the model considers only 16 phases).
    del leftCriticalPhaseDurationOfRing1[8:]
    del leftCriticalPhaseDurationOfRing2[8:]
    del rightCriticalPhaseDurationOfRing1[8:]
    del rightCriticalPhaseDurationOfRing2[8:]
    del leftCriticalGreenTimeOfRing1[8:]
    del leftCriticalGreenTimeOfRing2[8:]
    del rightCriticalGreenTimeOfRing1[8:]
    del rightCriticalGreenTimeOfRing2[8:]

    # Get ringwise left and right critical clearance time
    leftCriticalClearanceOfRing1 = [leftCriticalPhaseDurationOfRing1[i] -
                                    leftCriticalGreenTimeOfRing1[i] for i in range(len(leftCriticalPhaseDurationOfRing1))]

    leftCriticalClearanceOfRing2 = [leftCriticalPhaseDurationOfRing2[i] -
                                    leftCriticalGreenTimeOfRing2[i] for i in range(len(leftCriticalPhaseDurationOfRing2))]

    rightCriticalClearanceOfRing1 = [rightCriticalPhaseDurationOfRing1[i] -
                                     rightCriticalGreenTimeOfRing1[i] for i in range(len(rightCriticalPhaseDurationOfRing1))]

    rightCriticalClearanceOfRing2 = [rightCriticalPhaseDurationOfRing2[i] -
                                     rightCriticalGreenTimeOfRing2[i] for i in range(len(rightCriticalPhaseDurationOfRing2))]

    return leftCriticalGreenTimeOfRing1, leftCriticalGreenTimeOfRing2, rightCriticalGreenTimeOfRing1, rightCriticalGreenTimeOfRing2, leftCriticalClearanceOfRing1, leftCriticalClearanceOfRing2, rightCriticalClearanceOfRing1, rightCriticalClearanceOfRing2


def processTime(ring_Green_Time, ring_Clearance_Time, distance_apart, init):
    unwanted_num = {0.0}
    intersectionDistance = []
    if init == 0.0:
        ring_Green_Time = [ele for ele in ring_Green_Time if ele not in unwanted_num]
        if len(ring_Green_Time) != len(ring_Clearance_Time):
            greenRectangleStartPoint = []
            clearanceRectangleStartPoint = [0.0]
            for j in range(len(ring_Green_Time)):
                x = clearanceRectangleStartPoint[j] + ring_Clearance_Time[j] + ring_Green_Time[j]
                clearanceRectangleStartPoint.append(x)                   
                y = clearanceRectangleStartPoint[j] + ring_Clearance_Time[j]
                greenRectangleStartPoint.append(y)                    
                intersectionDistance.append(distance_apart)
            
            clearanceRectangleStartPoint = clearanceRectangleStartPoint[:-1]
            ring_Clearance_Time = ring_Clearance_Time[:-1]  

        elif len(ring_Green_Time) == len(ring_Clearance_Time):
            greenRectangleStartPoint = [0.0]
            clearanceRectangleStartPoint = []
            for j in range(len(ring_Green_Time)):
                x = greenRectangleStartPoint[j] + ring_Green_Time[j] + ring_Clearance_Time[j]
                greenRectangleStartPoint.append(x)
                y = greenRectangleStartPoint[j] + ring_Green_Time[j]
                clearanceRectangleStartPoint.append(y)                
                intersectionDistance.append(distance_apart)
                    
            del greenRectangleStartPoint[8:]
        
        

    if init > 0:
        ring_Clearance_Time.insert(0, init)
        if len(ring_Green_Time) != len(ring_Clearance_Time):
            ring_Clearance_Time = ring_Clearance_Time[:-1]
            greenRectangleStartPoint = []
            clearanceRectangleStartPoint = [0.0]
            for j in range(len(ring_Green_Time)):
                x = clearanceRectangleStartPoint[j] + ring_Clearance_Time[j] 
                greenRectangleStartPoint.append(x)
                y = clearanceRectangleStartPoint[j] + ring_Clearance_Time[j] + ring_Green_Time[j]
                clearanceRectangleStartPoint.append(y)                
                intersectionDistance.append(distance_apart)
            clearanceRectangleStartPoint= clearanceRectangleStartPoint[:-1]
            
    return ring_Green_Time, ring_Clearance_Time, greenRectangleStartPoint, clearanceRectangleStartPoint, intersectionDistance
        



def timeSpaceDiagram(greenRectangleStartPoint, clearanceRectangleStartPoint, greenTime, clearanceTime, distance, textString, tciRedTimeStartPoint, tciGreenTimeStartPoint, tciRedTime, tciGreenTime, tci_distance):
    # fig, ax1 = plt.subplots()
    fig = plt.figure()
    ax1 = fig.add_subplot(111, aspect='equal')
    # max_x_limit = greenRectangleStartPoint[-1]+greenTime[-1]+15
    max_x_limit = 250.0
    plt.xlim([0, max_x_limit])
    #plt.ylim([0, max(distance)+20])
    plt.ylim([0,max(tci_distance)+10])
    plt.xticks(np.arange(0, max_x_limit, 20),fontsize = 18) 
    # ax1.set_yticks(ticks=np.arange(0, max(tci_distance)+10, 10)) 
    
    # patches = []
    # req_phase_length = len(greenRectangleStartPoint)
    # for i in range(0, req_phase_length):
    #     x = greenRectangleStartPoint[i]
    #     y = distance[i]
    #     ax1.add_patch(matplotlib.patches.Rectangle(
    #         (x, y), greenTime[i], 5, angle = 0.0, color = 'green', linewidth = 2))

    # patches = []
    # req_phase_length = len(clearanceRectangleStartPoint)
    # for i in range(0, req_phase_length):
    #     x = clearanceRectangleStartPoint[i]
    #     y = distance[i]
    #     ax1.add_patch(matplotlib.patches.Rectangle((x, y), clearanceTime[i], 5, angle = 0.0, color = 'red', linewidth = 2))

    #Plot TCI Data
    patches = []
    req_phase_length = len(tciGreenTimeStartPoint)
    for i in range(0, req_phase_length):
        x = tciGreenTimeStartPoint[i]
        y = tci_distance[i]
        if i == 0:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y), tciGreenTime[i], 5, angle = 0.0, color = 'green', linewidth = 2, label = 'Green Time for Coordinated Phases'))

        else:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y), tciGreenTime[i], 5, angle = 0.0, color = 'green', linewidth = 2))
    
        
    patches = []
    req_phase_length = len(tciRedTimeStartPoint)
    for i in range(0, req_phase_length):
        x = tciRedTimeStartPoint[i]
        y = tci_distance[i]
        if i == 0:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y), tciRedTime[i], 5, angle = 0.0, color = 'red', linewidth = 2, label = 'Non-Coordinated Phases'))

        else:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y), tciRedTime[i], 5, angle = 0.0, color = 'red', linewidth = 2))


    # plt.text(0,max(distance)+25, textString, fontsize=10, bbox=dict(facecolor='white', alpha=0.5))
    ax1.legend(loc='upper right', bbox_to_anchor=(1, 1.3), prop={"size":18})
    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    plt.grid(color='black', linestyle='-', linewidth= 0.5)
    plt.show()

def getTCIData():
    # Read the config file into a json object:
    configFile = open("tci-data.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()
    
    redTime = []
    greenTime =[]
    redTimeStartPoint = []
    greenTimeStartPoint = []
    noOfIntersection = config["NoOfIntersection"]
    intersectionName = []
    distance = []
    dis = 10
    i = 1
    while (i <= noOfIntersection):
        x = "Intersection" + str(i)
        intersectionName.append(x)
        i = i +1
        
    
    
    for i in intersectionName:
        count = 0
        
        while (count<3):
            x = config["TCI_Data"][i]["RedTime"][count]
            y = config["TCI_Data"][i]["GreenTime"][count]
            u = config["TCI_Data"][i]["RedTimeStartPoint"][count]
            v = config["TCI_Data"][i]["GreenTimeStartPoint"][count]
            redTime.append(x)
            greenTime.append(y)
            redTimeStartPoint.append(u)
            greenTimeStartPoint.append(v)
            distance.append(dis)
            count = count + 1
        dis = dis+20
    return redTime, greenTime, redTimeStartPoint, greenTimeStartPoint, distance
    
def main():
    # Read the config file into a json object:
    configFile = open("configuration.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()
    noOfIntersection = config["NoOfIntersection"]
    fileDirectoryList = []
    intersectionDistance = []
    corridorDistance = []
    intersectionName = []
    greenTime = []
    clearanceTime = []
    greenTimeStartPoint = []
    clearanceTimeStartPoint = []
    # unwanted_num = {0.0}
    count = 0
    k = 0
    distance_apart = 0
    text_to_write = []

    while (count < noOfIntersection):
        filePath = config["FileDirectory"][count]
        fileDirectoryList.append(filePath)
        corridorName = config["IntersectionName"][count]
        intersectionName.append(corridorName)
        count = count + 1
      
    for i in fileDirectoryList:
        SP1, SP2 = getStartingPhases(i).split()
               
        distance_apart = distance_apart+20
        
        init1, init2, grn1, grn2 = getInitToPhasesAndElaspedGreenTime(i)
        text_to_write.append("Starting phases, green elapsed and init time of intersection No " + str(k+1) + "-" + intersectionName[k]+ " are: " + SP1 + " , " + SP2 +  " , " + str(grn1) + " , " + str(grn2) + " , " + str(init1) +  " , " + str(init2) + " , " + "\n")
        k = k+1
 
        left_Ring1_Green_Time, left_Ring2_Green_Time,  right_Ring1_Green_Time, right_Ring2_Green_Time, left_Ring1_Clearance_Time, left_Ring2_Clearance_Time,  right_Ring1_Clearance_Time,   right_Ring2_Clearance_Time = getSignalPhaseDuration(
            i)
        
        ring1_Green_Time, ring1_Clearance_Time, greenRectangleStartPoint, clearanceRectangleStartPoint, corridorDistance = processTime(right_Ring1_Green_Time, right_Ring1_Clearance_Time, distance_apart, init1)
        
        for element in ring1_Green_Time:
            greenTime.append(element)
            
        for element in ring1_Clearance_Time:
            clearanceTime.append(element)
        
        for element in greenRectangleStartPoint:
            greenTimeStartPoint.append(element)
            
        for element in clearanceRectangleStartPoint:
            clearanceTimeStartPoint.append(element)
        
        for element in corridorDistance:
            intersectionDistance.append(element)
        
        
    tciRedTime, tciGreenTime, tciRedTimeStartPoint, tciGreenTimeStartPoint, tciDistance = getTCIData()                    
    textString = ''.join(text_to_write)
    print (textString)
    timeSpaceDiagram(greenTimeStartPoint, clearanceTimeStartPoint,
                         greenTime, clearanceTime, intersectionDistance,textString, tciRedTimeStartPoint, tciGreenTimeStartPoint, tciRedTime, tciGreenTime, tciDistance)


if __name__ == "__main__":
    main()

