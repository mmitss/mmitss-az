import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
import json

#Get the stating phase information
def getStartingPhases():
    with open('/nojournal/bin/OptimizationResults.txt') as f:
        first_line = f.readline()
    return first_line

#Appending the SP1 into phasesInring list
#Appending the phases which are greater than the starting phase for that ring
#Appending the phases which are less than the starting phase for that ring
#Repeat all the phase number
#Appending the phases which are greater than the starting phase for that ring
def phaseGroupInRing(SP, ring_phases, phasesInRing):
    i = SP
    for i in ring_phases:
        if i > SP:
            phasesInRing.append(i)

    for i in ring_phases:
        if i < SP:
            phasesInRing.append(i)
    # Extending the phases for 2nd&3rdcycles
    phasesInRing.extend(phasesInRing*1)
    # for i in ring_phases:
    #     if i > SP:
    #         phasesInRing.append(i)
    # # Need to delete the phases based on starting phase
    # phasesInRing.pop(len(phasesInRing)-(SP-1))
    # print(phaseInRing)
    return phasesInRing


def getInitToPhasesAndElaspedGreenTime():
    with open('/nojournal/bin/OptimizationResults.txt') as f:

        for i, line in enumerate(f):
            if i == 1:
                break
    return line

#Find the phase duration for all the planned phases.
def getPhaseTimesForCycle1(phase_Times, SP, CP, RingNo):
    if(CP == 'Left'):
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 2:
                    break
        durationOfP1K1R1, durationOfP2K1R1, durationOfP3K1R1, durationOfP4K1R1, durationOfP5K1R1, durationOfP6K1R1, durationOfP7K1R1, durationOfP8K1R1 = line.split()
        # print(line)
        if(RingNo == 'Ring1'):
            left_r1_k1_Phase_Times = [float(durationOfP1K1R1), float(durationOfP2K1R1), float(durationOfP3K1R1), float(durationOfP4K1R1)]
            if SP > 1:
                left_r1_k1_Phase_Times = left_r1_k1_Phase_Times[SP-1:]

        elif(RingNo == 'Ring2'):
            left_r2_k1_Phase_Times = [float(durationOfP5K1R1), float(durationOfP6K1R1), float(durationOfP7K1R1), float(durationOfP8K1R1)]
            if SP > 4:
                left_r2_k1_Phase_Times = left_r2_k1_Phase_Times[SP-5:]

        # For cycle2 Left CP
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 3:
                    break
        durationOfP1K2R1, durationOfP2K2R1, durationOfP3K2R1, durationOfP4K2R1, durationOfP5K2R1, durationOfP6K2R1, durationOfP7K2R1, durationOfP8K2R1 = line.split()

        if(RingNo == 'Ring1'):    
            left_r1_k2_Phase_Times = [float(durationOfP1K2R1), float(durationOfP2K2R1), float(durationOfP3K2R1), float(durationOfP4K2R1)]
            left_r1_k1_Phase_Times.extend(left_r1_k2_Phase_Times)
        
        elif(RingNo == 'Ring2'):
            left_r2_k2_Phase_Times = [float(durationOfP5K2R1), float(durationOfP6K2R1), float(durationOfP7K2R1), float(durationOfP8K2R1)]
            left_r2_k1_Phase_Times.extend(left_r2_k2_Phase_Times)

        # For cycle3 Left CP
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 4:
                    break
        durationOfP1K3R1, durationOfP2K3R1, durationOfP3K3R1, durationOfP4K3R1, durationOfP5K3R1, durationOfP6K3R1, durationOfP7K3R1, durationOfP8K3R1 = line.split()

        if(RingNo == 'Ring1'):
            left_r1_k3_Phase_Times = [float(durationOfP1K3R1), float(durationOfP2K3R1), float(durationOfP3K3R1), float(durationOfP4K3R1)]
            left_r1_k1_Phase_Times.extend(left_r1_k3_Phase_Times)
            del left_r1_k1_Phase_Times[8:]
            phase_Times = left_r1_k1_Phase_Times
        
        elif(RingNo == 'Ring2'):
            left_r2_k3_Phase_Times = [float(durationOfP5K3R1), float(durationOfP6K3R1), float(durationOfP7K3R1), float(durationOfP8K3R1)]
            left_r2_k1_Phase_Times.extend(left_r2_k3_Phase_Times)
            del left_r2_k1_Phase_Times[8:]
            phase_Times = left_r2_k1_Phase_Times


    # # # For cycle1 Right CP
    if(CP == 'Right'):
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 5:
                    break
        durationOfP1K1R2, durationOfP2K1R2, durationOfP3K1R2, durationOfP4K1R2, durationOfP5K1R2, durationOfP6K1R2, durationOfP7K1R2, durationOfP8K1R2 = line.split()
        # print(line)

        if(RingNo == 'Ring1'):
            right_r1_k1_Phase_Times = [float(durationOfP1K1R2), float(durationOfP2K1R2), float(durationOfP3K1R2), float(durationOfP4K1R2)]
            if SP > 1:
                right_r1_k1_Phase_Times = right_r1_k1_Phase_Times[SP-1:]
        
        elif(RingNo == 'Ring2'):
            right_r2_k1_Phase_Times = [float(durationOfP5K1R2), float(durationOfP6K1R2), float(durationOfP7K1R2), float(durationOfP8K1R2)]
            if SP > 4:
                right_r2_k1_Phase_Times = right_r2_k1_Phase_Times[SP-5:]


        # For cycle2 Right CP
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 6:
                    break
        durationOfP1K2R2, durationOfP2K2R2, durationOfP3K2R2, durationOfP4K2R2, durationOfP5K2R2, durationOfP6K2R2, durationOfP7K2R2, durationOfP8K2R2 = line.split()

        if(RingNo == 'Ring1'):
            right_r1_k2_Phase_Times = [float(durationOfP1K2R2), float(durationOfP2K2R2), float(durationOfP3K2R2), float(durationOfP4K2R2)]
            right_r1_k1_Phase_Times.extend(right_r1_k2_Phase_Times)
        
        elif(RingNo == 'Ring2'):
            right_r2_k2_Phase_Times = [float(durationOfP5K2R2), float(durationOfP6K2R2), float(durationOfP7K2R2), float(durationOfP8K2R2)]
            right_r2_k1_Phase_Times.extend(right_r2_k2_Phase_Times)

        # For cycle3 Right CP
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 7:
                    break
        durationOfP1K3R2, durationOfP2K3R2, durationOfP3K3R2, durationOfP4K3R2, durationOfP5K3R2, durationOfP6K3R2, durationOfP7K3R2, durationOfP8K3R2 = line.split()
        
        if(RingNo == 'Ring1'):
            right_r1_k3_Phase_Times = [float(durationOfP1K3R2), float(durationOfP2K3R2), float(durationOfP3K3R2), float(durationOfP4K3R2)]
            right_r1_k1_Phase_Times.extend(right_r1_k3_Phase_Times)
            del right_r1_k1_Phase_Times[8:]
            phase_Times = right_r1_k1_Phase_Times

        elif(RingNo == 'Ring2'):
            right_r2_k3_Phase_Times = [float(durationOfP5K3R2), float(durationOfP6K3R2), float(durationOfP7K3R2), float(durationOfP8K3R2)]
            right_r2_k1_Phase_Times.extend(right_r2_k3_Phase_Times)
            del right_r2_k1_Phase_Times[8:]
            phase_Times = right_r2_k1_Phase_Times

    phase_Times = [x for x in phase_Times if x != 0]
    
    return phase_Times


def getCummulativePhaseTimes(ring_Phase_Times):
    cum_Ring_Phase_Times = []
    cum_Ring_Phase_Times = np.cumsum(ring_Phase_Times)

    # Appending 0 in the  beiginning of the list.
    cum_Ring_Phase_Times = np.insert(
        cum_Ring_Phase_Times, 0, 0)  # First 0 is position
    return cum_Ring_Phase_Times


def getPriorityRequest():
    eta = []
    with open('/nojournal/bin/OptimizationResults.txt') as f:
        for i, line in enumerate(f):
            if i == 14:
                break


    noOfReq = line
    noOfReq = int(noOfReq)
    print("No of Request", noOfReq)
    reqInfoLineNo = 15+noOfReq
    with open('/nojournal/bin/OptimizationResults.txt') as f:
        for i, line in enumerate(f):
            if i<15:
                continue
            # elif i>=15 & i<reqInfoLineNo:
            elif i in range(15,reqInfoLineNo):
                val1, Rl, Ru, val2, val3 = line.split()
                # eta.append([float(Rl), float(Ru)])
                eta.append(float(Rl))
            else:
                break
    # print("ETA",eta)

    return eta

# Plotting time-phase diagram
def timePhaseDiagram(SP1, SP2, cum_Left_Ring1_Phase_Times, cum_Right_Ring1_Phase_Times, cum_phaseInRing1, cum_Left_Ring2_Phase_Times, cum_Right_Ring2_Phase_Times, cum_phaseInRing2, phasesInRing1, phasesInRing2, ETA, req_phase, dilemmaZone_phases, dilemmaZone_ETA, ringNo):
    fig, ax1 = plt.subplots()
    if ringNo == 'Ring1&2':
        #Ring1
        color = 'tab:orange'
        ax1.set_xlabel('Time (s)',fontsize=24, fontweight = 'bold')
        ax1.set_ylabel('Ring 1 Phases', color=color, fontsize=28,fontweight = 'bold')
        ax1.plot(cum_Left_Ring1_Phase_Times, cum_phaseInRing1, color=color,linewidth = 2)
        ax1.plot(cum_Right_Ring1_Phase_Times, cum_phaseInRing1, color=color,linewidth = 2)
        plt.xticks(np.arange(cum_Right_Ring1_Phase_Times[0], cum_Right_Ring1_Phase_Times[-1], 10),fontsize = 24) 
        ax1.set_yticks(ticks=np.arange(cum_phaseInRing1[0], cum_phaseInRing1[-1], 10))  
        ax1.set_yticklabels(phasesInRing1)  
        ax1.tick_params(axis='y', labelcolor=color, labelsize=24)
        for axis in ['top','bottom','left','right']:
            ax1.spines[axis].set_linewidth(4)
        

        #Ring2
        ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
        color = 'tab:blue'
        ax2.set_ylabel('Ring 2 Phases', color=color, fontsize=28, fontweight = 'bold')
        ax2.plot(cum_Left_Ring2_Phase_Times, cum_phaseInRing2, color=color,linewidth = 4)
        ax2.plot(cum_Right_Ring2_Phase_Times, cum_phaseInRing2, color=color, linewidth = 4)
        ax2.set_yticks(ticks=np.arange(cum_phaseInRing2[0], cum_phaseInRing2[-1], 10))  
        ax2.set_yticklabels(phasesInRing2)
        ax2.tick_params(axis='y', labelcolor=color,labelsize=24)
    
    elif ringNo == 'Ring1':
        color = 'tab:red'
        ax1.set_xlabel('time (s)', fontsize=20)
        ax1.set_ylabel('Ring 1', color=color, fontsize=20)
        ax1.plot(cum_Left_Ring1_Phase_Times, cum_phaseInRing1, color=color)
        ax1.plot(cum_Right_Ring1_Phase_Times, cum_phaseInRing1, color=color)
        plt.xticks(np.arange(cum_Right_Ring1_Phase_Times[0], cum_Right_Ring1_Phase_Times[-1], 10),fontsize = 18) 
        ax1.set_yticks(ticks=np.arange(cum_phaseInRing1[0], cum_phaseInRing1[-1], 10))  
        ax1.set_yticklabels(phasesInRing1)  
        ax1.tick_params(axis='y', labelcolor=color, labelsize=18)
        
    
    elif ringNo == 'Ring2':
        color = 'tab:blue'
        ax1.set_xlabel('time (s)', fontsize=20)
        ax1.set_ylabel('Ring 2', color=color, fontsize=20)
        ax1.plot(cum_Left_Ring2_Phase_Times, cum_phaseInRing2, color=color)
        ax1.plot(cum_Right_Ring2_Phase_Times, cum_phaseInRing2, color=color)
        plt.xticks(np.arange(cum_Right_Ring2_Phase_Times[0], cum_Right_Ring2_Phase_Times[-1], 10),fontsize = 18) 
        ax1.set_yticks(ticks=np.arange(cum_phaseInRing2[0], cum_phaseInRing2[-1], 10))  
        ax1.set_yticklabels(phasesInRing2)  
        ax1.tick_params(axis='y', labelcolor=color,labelsize=18)
        
    # EV Requested phase
    requestedPhasePosition =[]
    indexPosList =[]
    indexPos = 0
    for i in req_phase:
        if i<5:
            for j in range(len(phasesInRing1)): 
                if phasesInRing1[j] == i:
                    indexPosList.append(j)
        elif i>4:
            for j in range(len(phasesInRing2)): 
                if phasesInRing2[j] == i:
                    indexPosList.append(j)
  

    if i<5:
        for i in indexPosList:
            pos = cum_phaseInRing1[i]
            requestedPhasePosition.append(pos)
    elif i>4:
        for i in indexPosList:
            pos = cum_phaseInRing2[i]
            requestedPhasePosition.append(pos)
                        


    patches =[]
    req_phase_length = len(requestedPhasePosition)
    for i in range(0,req_phase_length):
        x = ETA[i]
        y = requestedPhasePosition[i]
        if i == 0:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y),2,10,angle=0.0,color = 'red',linewidth = 2, label = 'EV Priority Request'))
        else:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y),2,10,angle=0.0,color = 'red',linewidth = 2))
    #     patches.append(matplotlib.patches.Rectangle((x, y),25,10,angle=0.0,color = 'red'))
    # ax1.add_collection(PatchCollection(patches))
    
    # Dilemma Requested phase
    requestedPhasePosition =[]
    indexPosList =[]
    indexPos = 0
    for i in dilemmaZone_phases:
        if i<5:
            for j in range(len(phasesInRing1)): 
                if phasesInRing1[j] == i:
                    indexPosList.append(j)
        elif i>4:
            for j in range(len(phasesInRing2)): 
                if phasesInRing2[j] == i:
                    indexPosList.append(j)
  

    if i<5:
        for i in indexPosList:
            pos = cum_phaseInRing1[i]
            requestedPhasePosition.append(pos)
    elif i>4:
        for i in indexPosList:
            pos = cum_phaseInRing2[i]
            requestedPhasePosition.append(pos)
                        


    patches =[]
    req_phase_length = len(requestedPhasePosition)
    for i in range(0,req_phase_length):
        x = dilemmaZone_ETA[i]
        y = requestedPhasePosition[i]
        if i == 0:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y),2,10,angle=0.0,color = 'green',linewidth = 4, label = 'DilemmaZone Request'))
        else:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y),2,10,angle=0.0,color = 'green', linewidth = 4))
    
    ax1.legend(loc='upper right', bbox_to_anchor=(.9, 1), prop={"size":18})
    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    plt.grid(color='black', linestyle='-', linewidth=2)
    # plt.legend(loc='best', bbox_to_anchor=(1.1, 1.1))
    # plt.legend()
    plt.show()



def main():
    configFile = open("configuration.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()
    
    r1_phases = []
    r2_phases = []
    left_R1_CP_phase_times = []
    right_R1_CP_phase_times = []
    cum_Left_Ring1_Phase_Times = []
    cum_Right_Ring1_Phase_Times = []
    cum_phaseInRing1 = []

    left_R2_CP_phase_times = []
    right_R2_CP_phase_times = []
    cum_Left_Ring2_Phase_Times = []
    cum_Right_Ring2_Phase_Times = []
    cum_phaseInRing2 = []

    dilemmaZone_phases = []
    dilemmaZone_ETA = []
    ETA = []
    req_phase = []
    phasesInRing1 = []
    phasesInRing2 = []
    count = 0
    noOfIteration = config["NoOfRequest"]
 
    while (count < noOfIteration):
            ETA_Val = config["ETA"][count]
            #Append the same ETA value twice for draw two rectangles for two cycle
            ETA.append(ETA_Val)
            ETA.append(ETA_Val)
            count = count + 1
    print("ETA", ETA)
    
    count = 0
    noOfIteration = config["NoOfRequiredPhase"]
    while (count < noOfIteration):
            phaseVal = config["RequestedPhase"][count]
            req_phase.append(phaseVal)
            count = count + 1
    print("Requested Phase", req_phase)
    
    #Dilemma-Zone Information
    count = 0
    noOfIteration = config["NoOfDilemmaZoneRequest"]
 
    while (count < noOfIteration):
            ETA_Val = config["DilemmaZoneETA"][count]
            #Append the same ETA value twice for draw two rectangles for two cycle
            dilemmaZone_ETA.append(ETA_Val)
            dilemmaZone_ETA.append(ETA_Val)
            count = count + 1
    print("DilemmaZone ETA", dilemmaZone_ETA)
    
    count = 0
    noOfIteration = config["NoOfRequiredDilemmaZonePhase"]
    while (count < noOfIteration):
            phaseVal = config["DilemmaZonePhases"][count]
            dilemmaZone_phases.append(phaseVal)
            count = count + 1
    print("DilemmaZone Requested Phase", dilemmaZone_phases)


    SP1, SP2 = getStartingPhases().split() #Get the stating phase information
    print("SP1 =", SP1)
    print("SP2 =", SP2)
    SP1 = int(SP1) #Converting starting phase into integar value
    SP2 = int(SP2) #Converting starting phase into integar value

    
    #Obtained planned signal phase of cycle1,2,3 for ring 1. There will be 8 phases.
    #phasesInRing1 = []
    
    # n = int(input("Enter no of Phases in Ring1: "))
    # phasesInRing1 = list(map(int,input("\nEnter the phase numbers following by space : ").strip().split()))[:n] 
    
    #Obtained planned signal phase of cycle1,2,3 for ring 2. There will be 8 phases
    # phasesInRing2 = []
    
    # n = int(input("Enter no of Phases in Ring2: "))
    # phasesInRing2 = list(map(int,input("\nEnter the phase numbers following by space : ").strip().split()))[:n]
    
    count = 0
    noOfIteration = config["NoOfPhasesInRing1"]
 
    while (count < noOfIteration):
            phaseVal = config["PhasesInRing1"][count]
            phasesInRing1.append(phaseVal)
            count = count + 1
    print("Phases In Ring1", phasesInRing1)
    
    count = 0
    noOfIteration = config["NoOfPhasesInRing2"]
 
    while (count < noOfIteration):
            phaseVal = config["PhasesInRing2"][count]
            phasesInRing2.append(phaseVal)
            count = count + 1
    print("Phases In Ring2", phasesInRing2)
    
    #obtained init time and green elapssed time
    init1, init2, grn1, grn2 = getInitToPhasesAndElaspedGreenTime().split()
    print("ini1 =", init1)
    print("ini2 =", init2)
    print("Elapesd Green1 =", grn1)
    print("Elapesd Green2 =", grn2)
    



    ################## For Ring1##################

    #Obatined ring wise phase duration for left and right critical points
    left_R1_CP_phase_times = getPhaseTimesForCycle1(
        left_R1_CP_phase_times, SP1, 'Left','Ring1')
    right_R1_CP_phase_times = getPhaseTimesForCycle1(
        right_R1_CP_phase_times, SP1, 'Right', 'Ring1')
    print("Left Critical Points Phase times for Ring1 =", left_R1_CP_phase_times)
    print("Right Critical Points Phase times for Ring1 =", right_R1_CP_phase_times)
    # #creating cumulative list

    # if SP2-SP1 == 3: ##starting phase 4,7
    #     cum_Left_Ring1_Phase_Times = getCummulativePhaseTimes(
    #     left_R1_CP_phase_times)
        
    #     cum_Right_Ring1_Phase_Times = getCummulativePhaseTimes(
    #     right_R1_CP_phase_times)

    #     cum_Left_Ring1_Phase_Times = np.insert(cum_Left_Ring1_Phase_Times,0,0.0)
    #     cum_Right_Ring1_Phase_Times = np.insert(cum_Right_Ring1_Phase_Times,0,0.0)

    #     phasesInRing1 = np.insert(phasesInRing1, 0, SP1-1)
    #     x = 0
    #     cum_phaseInRing1= [x]
    #     length = len(cum_Left_Ring1_Phase_Times)-1
    #     for i in range(length):
    #         x = x+10
    #         cum_phaseInRing1.append(x)

    # elif SP2-SP1 == 5:  ##starting phase 3,8
    #     cum_Left_Ring1_Phase_Times = getCummulativePhaseTimes(
    #         left_R1_CP_phase_times)
        
    #     cum_Right_Ring1_Phase_Times = getCummulativePhaseTimes(
    #         right_R1_CP_phase_times)

    #     cum_Left_Ring1_Phase_Times = np.insert(cum_Left_Ring2_Phase_Times,len(cum_Left_Ring2_Phase_Times),cum_Left_Ring2_Phase_Times[-1]+10)
    #     cum_Right_Ring1_Phase_Times = np.insert(cum_Right_Ring2_Phase_Times,len(cum_Right_Ring2_Phase_Times),cum_Right_Ring2_Phase_Times[-1]+10)

    #     phasesInRing1 = np.insert(phasesInRing1, len(phasesInRing1), SP1)
    #     x = 0
    #     cum_phaseInRing1= [x]
    #     length = len(cum_Left_Ring1_Phase_Times)-1
    #     for i in range(length):
    #         x = x+10
    #         cum_phaseInRing1.append(x)


    # else:
    #     cum_Left_Ring1_Phase_Times = getCummulativePhaseTimes(left_R1_CP_phase_times)
    #     cum_Right_Ring1_Phase_Times = getCummulativePhaseTimes(right_R1_CP_phase_times)
    #     x = 0
    #     cum_phaseInRing1= [x]
    #     length = len(cum_Left_Ring1_Phase_Times)-1
    #     for i in range(length):
    #         x = x+10
    #         cum_phaseInRing1.append(x)

    cum_Left_Ring1_Phase_Times = getCummulativePhaseTimes(left_R1_CP_phase_times)
    cum_Right_Ring1_Phase_Times = getCummulativePhaseTimes(right_R1_CP_phase_times)
    x = 0
    cum_phaseInRing1= [x]
    length = len(cum_Left_Ring1_Phase_Times)-1
    for i in range(length):
        x = x+10
        cum_phaseInRing1.append(x)
    print("Phases In Ring1", phasesInRing1)
    print("Cumulative Left Critical Points Phase times for Ring1 =",
          cum_Left_Ring1_Phase_Times)
    print("Cumulative Right Critical Points Phase times for Ring1 =",
          cum_Right_Ring1_Phase_Times)
    print("Cumulative Phases in Ring1 =", cum_phaseInRing1)
    
    ################## For Ring2##################
    
    left_R2_CP_phase_times = getPhaseTimesForCycle1(
        left_R2_CP_phase_times, SP2, 'Left','Ring2')
    right_R2_CP_phase_times = getPhaseTimesForCycle1(
        right_R2_CP_phase_times, SP2, 'Right', 'Ring2')
    print("Left Critical Points Phase times for Ring2 =", left_R2_CP_phase_times)
    print("Right Critical Points Phase times for Ring2 =", right_R2_CP_phase_times)
    # # #creating cumulative list
    # if SP2-SP1 == 3:
    #     cum_Left_Ring2_Phase_Times = getCummulativePhaseTimes(
    #         left_R2_CP_phase_times)
        
    #     cum_Right_Ring2_Phase_Times = getCummulativePhaseTimes(
    #         right_R2_CP_phase_times)

    #     cum_Left_Ring2_Phase_Times = np.insert(cum_Left_Ring2_Phase_Times,len(cum_Left_Ring2_Phase_Times),cum_Left_Ring2_Phase_Times[-1]+10)
    #     cum_Right_Ring2_Phase_Times = np.insert(cum_Right_Ring2_Phase_Times,len(cum_Right_Ring2_Phase_Times),cum_Right_Ring2_Phase_Times[-1]+10)

    #     phasesInRing2 = np.insert(phasesInRing2, len(phasesInRing2), SP2)
    #     x = 0
    #     cum_phaseInRing2= [x]
    #     length = len(cum_Left_Ring2_Phase_Times)-1
    #     for i in range(length):
    #         x = x+10
    #         cum_phaseInRing2.append(x)


    # elif SP2-SP1 == 5:
    #     cum_Left_Ring2_Phase_Times = getCummulativePhaseTimes(
    #     left_R2_CP_phase_times)
        
    #     cum_Right_Ring2_Phase_Times = getCummulativePhaseTimes(
    #     right_R2_CP_phase_times)

    #     cum_Left_Ring2_Phase_Times = np.insert(cum_Left_Ring2_Phase_Times,0,0.0)
    #     cum_Right_Ring2_Phase_Times = np.insert(cum_Right_Ring2_Phase_Times,0,0.0)

    #     phasesInRing2 = np.insert(phasesInRing2, 0, SP2-1)
    #     x = 0
    #     cum_phaseInRing2= [x]
    #     length = len(cum_Left_Ring2_Phase_Times)-1
    #     for i in range(length):
    #         x = x+10
    #         cum_phaseInRing2.append(x)

    # else:
    #     cum_Left_Ring2_Phase_Times = getCummulativePhaseTimes(left_R2_CP_phase_times)
    #     cum_Right_Ring2_Phase_Times = getCummulativePhaseTimes(right_R2_CP_phase_times)
    #     x = 0
    #     cum_phaseInRing2= [x]
    #     length = len(cum_Left_Ring2_Phase_Times)-1
    #     for i in range(length):
    #         x = x+10
    #         cum_phaseInRing2.append(x)
    
    
    cum_Left_Ring2_Phase_Times = getCummulativePhaseTimes(left_R2_CP_phase_times)
    cum_Right_Ring2_Phase_Times = getCummulativePhaseTimes(right_R2_CP_phase_times)
    x = 0
    cum_phaseInRing2= [x]
    length = len(cum_Left_Ring2_Phase_Times)-1
    for i in range(length):
        x = x+10
        cum_phaseInRing2.append(x)
            
    print("Phases In Ring2", phasesInRing2)
    print("Cumulative Left Critical Points Phase times for Ring2 =",
          cum_Left_Ring2_Phase_Times)
    print("Cumulative Right Critical Points Phase times for Ring2 =",
          cum_Right_Ring2_Phase_Times)
    print("Cumulative Phases in Ring2 =", cum_phaseInRing2)


    timePhaseDiagram(SP1, SP2,cum_Left_Ring1_Phase_Times, cum_Right_Ring1_Phase_Times,
                     cum_phaseInRing1,cum_Left_Ring2_Phase_Times, cum_Right_Ring2_Phase_Times,
                     cum_phaseInRing2, phasesInRing1, phasesInRing2, ETA, req_phase, dilemmaZone_phases, dilemmaZone_ETA, 'Ring1&2')



if __name__ == '__main__':
    main()