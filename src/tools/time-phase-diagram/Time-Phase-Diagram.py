import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
import json


def getStartingPhases():
    """
    Get the stating phase information from the OptimizationResults.txt file
    """
    with open('/nojournal/bin/OptimizationResults.txt') as f:
        first_line = f.readline()

    startingPhase1, startingPhase2 = first_line.split()
    # Converting starting phase into integar value
    startingPhase1 = int(startingPhase1)
    # Converting starting phase into integar value
    startingPhase2 = int(startingPhase2)

    return startingPhase1, startingPhase2


def phaseGroupInRing(startingPhase, ring_phases, phasesInRing):
    """
    starting Phase is already appended into phasesInring list
    Appending the phases which are greater than the starting phase for that ring
    Appending the phases which are less than the starting phase for that ring
    Repeat all the phase number by using extend method
    """
    i = startingPhase
    for i in ring_phases:
        if i > startingPhase:
            phasesInRing.append(i)

    for i in ring_phases:
        if i < startingPhase:
            phasesInRing.append(i)
    # Extending the phases for 2nd & 3rdcycles
    phasesInRing.extend(phasesInRing*1)

    return phasesInRing


def getInitToPhasesAndElaspedGreenTime():
    with open('/nojournal/bin/OptimizationResults.txt') as f:

        for i, line in enumerate(f):
            if i == 1:
                break
    return line


def getPhaseDuration(phase_Times, startingPhase, CP, RingNo):
    """
    Find the phase Duration for all the planned phases.
    Store the information of left and critical points based on ring number for each cycle
    """

    # Storing Left Critcial Points information
    if(CP == 'Left'):
        # Left Critical Points information for first cycle
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 2:
                    break
        # phaseDurationOfP1K1R1 means phase duration of phase 1 (P1) in ring1 (R1) for cycle 1 (K1)
        phaseDurationOfP1K1R1, phaseDurationOfP2K1R1, phaseDurationOfP3K1R1, phaseDurationOfP4K1R1, phaseDurationOfP5K1R1, phaseDurationOfP6K1R1, phaseDurationOfP7K1R1, phaseDurationOfP8K1R1 = line.split()

        if(RingNo == 'Ring1'):
            left_r1_k1_Phase_Times = [float(phaseDurationOfP1K1R1), float(
                phaseDurationOfP2K1R1), float(phaseDurationOfP3K1R1), float(phaseDurationOfP4K1R1)]
            if startingPhase > 1:
                left_r1_k1_Phase_Times = left_r1_k1_Phase_Times[startingPhase-1:]

        elif(RingNo == 'Ring2'):
            left_r2_k1_Phase_Times = [float(phaseDurationOfP5K1R1), float(
                phaseDurationOfP6K1R1), float(phaseDurationOfP7K1R1), float(phaseDurationOfP8K1R1)]
            if startingPhase > 4:
                left_r2_k1_Phase_Times = left_r2_k1_Phase_Times[startingPhase-5:]

        #  Left Critical Points information for second cycle
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 3:
                    break
        phaseDurationOfP1K2R1, phaseDurationOfP2K2R1, phaseDurationOfP3K2R1, phaseDurationOfP4K2R1, phaseDurationOfP5K2R1, phaseDurationOfP6K2R1, phaseDurationOfP7K2R1, phaseDurationOfP8K2R1 = line.split()

        if(RingNo == 'Ring1'):
            left_r1_k2_Phase_Times = [float(phaseDurationOfP1K2R1), float(
                phaseDurationOfP2K2R1), float(phaseDurationOfP3K2R1), float(phaseDurationOfP4K2R1)]
            left_r1_k1_Phase_Times.extend(left_r1_k2_Phase_Times)

        elif(RingNo == 'Ring2'):
            left_r2_k2_Phase_Times = [float(phaseDurationOfP5K2R1), float(
                phaseDurationOfP6K2R1), float(phaseDurationOfP7K2R1), float(phaseDurationOfP8K2R1)]
            left_r2_k1_Phase_Times.extend(left_r2_k2_Phase_Times)

        #  Left Critical Points information for third cycle
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 4:
                    break
        phaseDurationOfP1K3R1, phaseDurationOfP2K3R1, phaseDurationOfP3K3R1, phaseDurationOfP4K3R1, phaseDurationOfP5K3R1, phaseDurationOfP6K3R1, phaseDurationOfP7K3R1, phaseDurationOfP8K3R1 = line.split()

        if(RingNo == 'Ring1'):
            left_r1_k3_Phase_Times = [float(phaseDurationOfP1K3R1), float(
                phaseDurationOfP2K3R1), float(phaseDurationOfP3K3R1), float(phaseDurationOfP4K3R1)]
            left_r1_k1_Phase_Times.extend(left_r1_k3_Phase_Times)
            del left_r1_k1_Phase_Times[8:]
            phase_Times = left_r1_k1_Phase_Times

        elif(RingNo == 'Ring2'):
            left_r2_k3_Phase_Times = [float(phaseDurationOfP5K3R1), float(
                phaseDurationOfP6K3R1), float(phaseDurationOfP7K3R1), float(phaseDurationOfP8K3R1)]
            left_r2_k1_Phase_Times.extend(left_r2_k3_Phase_Times)
            del left_r2_k1_Phase_Times[8:]
            phase_Times = left_r2_k1_Phase_Times

    # Storing RIght Critcial Points information
    if(CP == 'Right'):
        # Right Critical Points information for first cycle
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 5:
                    break

        phaseDurationOfP1K1R2, phaseDurationOfP2K1R2, phaseDurationOfP3K1R2, phaseDurationOfP4K1R2, phaseDurationOfP5K1R2, phaseDurationOfP6K1R2, phaseDurationOfP7K1R2, phaseDurationOfP8K1R2 = line.split()

        if(RingNo == 'Ring1'):
            right_r1_k1_Phase_Times = [float(phaseDurationOfP1K1R2), float(
                phaseDurationOfP2K1R2), float(phaseDurationOfP3K1R2), float(phaseDurationOfP4K1R2)]
            if startingPhase > 1:
                right_r1_k1_Phase_Times = right_r1_k1_Phase_Times[startingPhase-1:]

        elif(RingNo == 'Ring2'):
            right_r2_k1_Phase_Times = [float(phaseDurationOfP5K1R2), float(
                phaseDurationOfP6K1R2), float(phaseDurationOfP7K1R2), float(phaseDurationOfP8K1R2)]
            if startingPhase > 4:
                right_r2_k1_Phase_Times = right_r2_k1_Phase_Times[startingPhase-5:]

        # # Right Critical Points information for second cycle
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 6:
                    break
        phaseDurationOfP1K2R2, phaseDurationOfP2K2R2, phaseDurationOfP3K2R2, phaseDurationOfP4K2R2, phaseDurationOfP5K2R2, phaseDurationOfP6K2R2, phaseDurationOfP7K2R2, phaseDurationOfP8K2R2 = line.split()

        if(RingNo == 'Ring1'):
            right_r1_k2_Phase_Times = [float(phaseDurationOfP1K2R2), float(
                phaseDurationOfP2K2R2), float(phaseDurationOfP3K2R2), float(phaseDurationOfP4K2R2)]
            right_r1_k1_Phase_Times.extend(right_r1_k2_Phase_Times)

        elif(RingNo == 'Ring2'):
            right_r2_k2_Phase_Times = [float(phaseDurationOfP5K2R2), float(
                phaseDurationOfP6K2R2), float(phaseDurationOfP7K2R2), float(phaseDurationOfP8K2R2)]
            right_r2_k1_Phase_Times.extend(right_r2_k2_Phase_Times)

        # Right Critical Points information for third cycle
        with open('/nojournal/bin/OptimizationResults.txt') as f:
            for i, line in enumerate(f):
                if i == 7:
                    break
        phaseDurationOfP1K3R2, phaseDurationOfP2K3R2, phaseDurationOfP3K3R2, phaseDurationOfP4K3R2, phaseDurationOfP5K3R2, phaseDurationOfP6K3R2, phaseDurationOfP7K3R2, phaseDurationOfP8K3R2 = line.split()

        if(RingNo == 'Ring1'):
            right_r1_k3_Phase_Times = [float(phaseDurationOfP1K3R2), float(
                phaseDurationOfP2K3R2), float(phaseDurationOfP3K3R2), float(phaseDurationOfP4K3R2)]
            right_r1_k1_Phase_Times.extend(right_r1_k3_Phase_Times)
            del right_r1_k1_Phase_Times[8:]
            phase_Times = right_r1_k1_Phase_Times

        elif(RingNo == 'Ring2'):
            right_r2_k3_Phase_Times = [float(phaseDurationOfP5K3R2), float(
                phaseDurationOfP6K3R2), float(phaseDurationOfP7K3R2), float(phaseDurationOfP8K3R2)]
            right_r2_k1_Phase_Times.extend(right_r2_k3_Phase_Times)
            del right_r2_k1_Phase_Times[8:]
            phase_Times = right_r2_k1_Phase_Times

    return phase_Times


def getCummulativePhaseTimes(startingPhase1, startingPhase2, left_ring_Phase_Times, right_ring_Phase_Times, phasesInRing, cum_phaseInRing, ringNo):
    """
    Compute cumulative phase duration using cumsum method from numpy
    Appending 0 in the  beiginning of the list to define the starting point of the diagram
    check the special condition of starting phase (for example phase 1&6, phase 2&5, phase 3&8, phase 4&7)
    Append starting in the beginning of phasesInRing list if require. For example, if Starting phase is 4 and 7 then append phase 3 in the beginning of the list
    """
    cum_left_Ring_Phase_Times = []
    cum_right_Ring_Phase_Times = []
    cum_left_Ring_Phase_Times = np.cumsum(left_ring_Phase_Times)
    cum_right_Ring_Phase_Times = np.cumsum(right_ring_Phase_Times)

    # First 0 is index in the list
    cum_left_Ring_Phase_Times = np.insert(cum_left_Ring_Phase_Times, 0, 0)
    cum_right_Ring_Phase_Times = np.insert(cum_right_Ring_Phase_Times, 0, 0)

    if ringNo == 'Ring1':
        if startingPhase2-startingPhase1 == 3:  # For example, if starting phase 4,7
            cum_left_Ring_Phase_Times = np.insert(
                cum_left_Ring_Phase_Times, 0, 0.0)

            cum_right_Ring_Phase_Times = np.insert(
                cum_right_Ring_Phase_Times, 0, 0.0)

            # If Starting phase is 4 and 7 then append phase 3 in the beginning of the list
            phasesInRing = np.insert(phasesInRing, 0, startingPhase1-1)

        elif startingPhase2-startingPhase1 == 5:  # starting phase 3,8
            cum_left_Ring_Phase_Times = np.insert(cum_left_Ring_Phase_Times, len(
                cum_left_Ring_Phase_Times), cum_left_Ring_Phase_Times[-1]+10)

            cum_right_Ring_Phase_Times = np.insert(cum_right_Ring_Phase_Times, len(
                cum_right_Ring_Phase_Times), cum_right_Ring_Phase_Times[-1]+10)

            phasesInRing = np.insert(
                phasesInRing, len(phasesInRing), startingPhase1)
                
        x = 0
        cum_phaseInRing = [x]
        length = len(cum_left_Ring_Phase_Times)-1
        for i in range(length):
            x = x+10
            cum_phaseInRing.append(x)

    if ringNo == 'Ring2':
        if startingPhase2-startingPhase1 == 3:
            cum_left_Ring_Phase_Times = np.insert(cum_left_Ring_Phase_Times, len(
                cum_left_Ring_Phase_Times), cum_left_Ring_Phase_Times[-1]+10)

            cum_right_Ring_Phase_Times = np.insert(cum_right_Ring_Phase_Times, len(
                cum_right_Ring_Phase_Times), cum_right_Ring_Phase_Times[-1]+10)

            phasesInRing = np.insert(
                phasesInRing, len(phasesInRing), startingPhase2)

        elif startingPhase2-startingPhase1 == 5:
            cum_left_Ring_Phase_Times = np.insert(
                cum_left_Ring_Phase_Times, 0, 0.0)

            cum_right_Ring_Phase_Times = np.insert(
                cum_right_Ring_Phase_Times, 0, 0.0)

            phasesInRing = np.insert(phasesInRing, 0, startingPhase2 - 1)

        x = 0
        cum_phaseInRing = [x]
        length = len(cum_left_Ring_Phase_Times)-1
        for i in range(length):
            x = x+10
            cum_phaseInRing.append(x)

    return cum_left_Ring_Phase_Times, cum_right_Ring_Phase_Times, phasesInRing, cum_phaseInRing


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
            if i < 15:
                continue
            # elif i>=15 & i<reqInfoLineNo:
            elif i in range(15, reqInfoLineNo):
                val1, Rl, Ru, val2, val3 = line.split()
                # eta.append([float(Rl), float(Ru)])
                eta.append(float(Rl))
            else:
                break
    # print("ETA",eta)

    return eta

# Plotting time-phase diagram Method


def timePhaseDiagramMethod(startingPhase1, startingPhase2, cum_Left_Ring1_Phase_Times, cum_Right_Ring1_Phase_Times, cum_phaseInRing1, cum_Left_Ring2_Phase_Times, cum_Right_Ring2_Phase_Times, cum_phaseInRing2, phasesInRing1, phasesInRing2, ETA, ETA_Duration, req_phase, secondPriorityRequestphases, secondPriorityRequestETA, secondPriorityRequestETA_Duration, ringNo):
    fig, ax1 = plt.subplots()
    if ringNo == 'Ring1&2':
        # Ring1
        color = 'tab:red'
        ax1.set_xlabel('Time (s)', fontsize=24, fontweight='bold')
        ax1.set_ylabel('Ring 1', color=color, fontsize=28, fontweight='bold')
        ax1.plot(cum_Left_Ring1_Phase_Times,
                 cum_phaseInRing1, color=color, linewidth=4)
        ax1.plot(cum_Right_Ring1_Phase_Times,
                 cum_phaseInRing1, color=color, linewidth=4)
        plt.xticks(np.arange(
            cum_Right_Ring1_Phase_Times[0], cum_Right_Ring1_Phase_Times[-1], 20), fontsize=24)
        ax1.set_yticks(ticks=np.arange(
            cum_phaseInRing1[0], cum_phaseInRing1[-1], 10))
        ax1.set_yticklabels(phasesInRing1)
        ax1.tick_params(axis='y', labelcolor=color, labelsize=18)
        for axis in ['top', 'bottom', 'left', 'right']:
            ax1.spines[axis].set_linewidth(4)

        # Ring2
        ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis
        color = 'tab:blue'
        ax2.set_ylabel('Ring 2', color=color, fontsize=28, fontweight='bold')
        ax2.plot(cum_Left_Ring2_Phase_Times,
                 cum_phaseInRing2, color=color, linewidth=4)
        ax2.plot(cum_Right_Ring2_Phase_Times,
                 cum_phaseInRing2, color=color, linewidth=4)
        ax2.set_yticks(ticks=np.arange(
            cum_phaseInRing2[0], cum_phaseInRing2[-1], 10))
        ax2.set_yticklabels(phasesInRing2)
        ax2.tick_params(axis='y', labelcolor=color, labelsize=24)

    elif ringNo == 'Ring1':
        color = 'tab:red'
        ax1.set_xlabel('time (s)', fontsize=20)
        ax1.set_ylabel('Ring 1', color=color, fontsize=20)
        ax1.plot(cum_Left_Ring1_Phase_Times, cum_phaseInRing1, color=color)
        ax1.plot(cum_Right_Ring1_Phase_Times, cum_phaseInRing1, color=color)
        plt.xticks(np.arange(
            cum_Right_Ring1_Phase_Times[0], cum_Right_Ring1_Phase_Times[-1], 10), fontsize=18)
        ax1.set_yticks(ticks=np.arange(
            cum_phaseInRing1[0], cum_phaseInRing1[-1], 10))
        ax1.set_yticklabels(phasesInRing1)
        ax1.tick_params(axis='y', labelcolor=color, labelsize=18)

    elif ringNo == 'Ring2':
        color = 'tab:blue'
        ax1.set_xlabel('time (s)', fontsize=20)
        ax1.set_ylabel('Ring 2', color=color, fontsize=20)
        ax1.plot(cum_Left_Ring2_Phase_Times, cum_phaseInRing2, color=color)
        ax1.plot(cum_Right_Ring2_Phase_Times, cum_phaseInRing2, color=color)
        plt.xticks(np.arange(
            cum_Right_Ring2_Phase_Times[0], cum_Right_Ring2_Phase_Times[-1], 10), fontsize=18)
        ax1.set_yticks(ticks=np.arange(
            cum_phaseInRing2[0], cum_phaseInRing2[-1], 10))
        ax1.set_yticklabels(phasesInRing2)
        ax1.tick_params(axis='y', labelcolor=color, labelsize=18)

    # Requested phase of First Type Priority Request
    requestedPhasePosition = []
    # indexPosList = []
    indexPos = 0

    for requestedPhase in req_phase:
        indexPosList = []
        if requestedPhase < 5:
            # Get the requested phases index in phasesInRing1 list
            for j in range(len(phasesInRing1)):
                if phasesInRing1[j] == requestedPhase:
                    indexPosList.append(j)
    # Get the cumulative phase phaseDuration value of requested phases based on the index
            for i in indexPosList:
                pos = cum_phaseInRing1[i]
                requestedPhasePosition.append(pos)
        elif requestedPhase > 4:
            # Get the requested phases index in phasesInRing2 list
            for j in range(len(phasesInRing2)):
                if phasesInRing2[j] == requestedPhase:
                    indexPosList.append(j)
    # Get the cumulative phase phaseDuration value of requested phases based on the index
            for i in indexPosList:
                pos = cum_phaseInRing2[i]
                requestedPhasePosition.append(pos)

    # Draw the rectangles to denote priority request
    patches = []
    req_phase_length = len(requestedPhasePosition)
    for i in range(0, req_phase_length):
        x = ETA[i]
        y = requestedPhasePosition[i]
        z = ETA_Duration[i]
        if i == 0:
            ax1.add_patch(matplotlib.patches.Rectangle(
                (x, y), z, 10, angle=0.0, color='green', linewidth=2, label='Coordination Priority Request'))
        else:
            ax1.add_patch(matplotlib.patches.Rectangle(
                (x, y), z, 10, angle=0.0, color='green', linewidth=2))

    # Requested phase of Second Type Priority Request
    requestedPhasePosition = []
    # indexPosList = []
    indexPos = 0
    for requestedPhase in secondPriorityRequestphases:
        indexPosList = []
        if requestedPhase < 5:
            # Get the requested phases index in phasesInRing1 list
            for j in range(len(phasesInRing1)):
                if phasesInRing1[j] == requestedPhase:
                    indexPosList.append(j)
    # Get the cumulative phase phaseDuration value of requested phases based on the index
            for i in indexPosList:
                pos = cum_phaseInRing1[i]
                requestedPhasePosition.append(pos)
        elif requestedPhase > 4:
            # Get the requested phases index in phasesInRing2 list
            for j in range(len(phasesInRing2)):
                if phasesInRing2[j] == requestedPhase:
                    indexPosList.append(j)
    # Get the cumulative phase phaseDuration value of requested phases based on the index
            for i in indexPosList:
                pos = cum_phaseInRing2[i]
                requestedPhasePosition.append(pos)

    patches = []
    req_phase_length = len(requestedPhasePosition)
    for i in range(0, req_phase_length):
        x = secondPriorityRequestETA[i]
        y = requestedPhasePosition[i]
        z = secondPriorityRequestETA_Duration[i]
        if i == 0:
            ax1.add_patch(matplotlib.patches.Rectangle((x, y), z, 10, angle=0.0,
                                                       color='#0099FF', linewidth=2, label='Transit Priority Request'))
        else:
            ax1.add_patch(matplotlib.patches.Rectangle(
                (x, y), z, 10, angle=0.0, color='#0099FF', linewidth=2))

    ax1.legend(loc='upper right', bbox_to_anchor=(1, 1), prop={"size": 18})
    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    plt.grid(color='black', linestyle='-', linewidth=2)

    plt.show()


def main():
    configFile = open("configuration.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()
    # Defining the phases in ring 1 and ring 2
    r1_phases = [1, 2, 3, 4]
    r2_phases = [5, 6, 7, 8]
    # Declaration of lists to store the left and right critical points for ring 1
    left_R1_CP_phase_times = []
    right_R1_CP_phase_times = []
    # Declaration of lists to store the cumulative left and right critical points for ring 1
    cum_Left_Ring1_Phase_Times = []
    cum_Right_Ring1_Phase_Times = []
    cum_phaseInRing1 = []
    # Declaration of lists to store the left and right critical points for ring 2
    left_R2_CP_phase_times = []
    right_R2_CP_phase_times = []
    # Declaration of lists to store the cumulative left and right critical points for ring 2
    cum_Left_Ring2_Phase_Times = []
    cum_Right_Ring2_Phase_Times = []
    cum_phaseInRing2 = []
    # Declaration of lists to store the ETA, ETA_Duration and requested phases
    ETA = []
    ETA_Duration = []
    req_phase = []

    count = 0
    noOfIteration = config["NoOfRequest"]

    while (count < noOfIteration):
        ETA_Val = config["ETA"][count]
        ETA_Duration_Val = config["ETA_Duration"][count]
        # Append the same ETA and ETA_Duration value twice for plotting two rectangles for two cycle
        ETA.append(ETA_Val)
        ETA.append(ETA_Val)
        ETA_Duration.append(ETA_Duration_Val)
        ETA_Duration.append(ETA_Duration_Val)
        count = count + 1
    #print("ETA", ETA)

    count = 0
    noOfIteration = config["NoOfRequiredPhase"]
    while (count < noOfIteration):
        phaseVal = config["RequestedPhase"][count]
        req_phase.append(phaseVal)
        count = count + 1
    #print("Requested Phase", req_phase)

    # For Differentiating two types of request
    secondPriorityRequestphases = []
    secondPriorityRequestETA = []
    secondPriorityRequestETA_Duration = []
    # Dilemma-Zone Information
    multipleTypesOfRequest = config["MultipleTypesofRequest"]
    if bool(multipleTypesOfRequest):
        count = 0
        noOfIteration = config["NoOfDilemmaZoneRequest"]

        while (count < noOfIteration):
            ETA_Val = config["DilemmaZoneETA"][count]
            ETA_Duration_Val = config["DilemmaZoneETA_Duration"][count]
            # Append the same ETA value twice for draw two rectangles for two cycle
            secondPriorityRequestETA.append(ETA_Val)
            secondPriorityRequestETA.append(ETA_Val)
            secondPriorityRequestETA_Duration.append(ETA_Duration_Val)
            secondPriorityRequestETA_Duration.append(ETA_Duration_Val)
            count = count + 1
        #print("Second Requests ETA", secondPriorityRequestETA)

        count = 0
        noOfIteration = config["NoOfRequiredDilemmaZonePhase"]
        while (count < noOfIteration):
            phaseVal = config["DilemmaZonePhases"][count]
            secondPriorityRequestphases.append(phaseVal)
            count = count + 1
        #print("Second Requests' Requested Phase", secondPriorityRequestphases)

    # Get the stating phase information
    startingPhase1, startingPhase2 = getStartingPhases()

    # Appending the startingPhase1 into phasesInring1 list
    phasesInRing1 = [startingPhase1]
    # Appending the startingPhase2 into phasesInring2 list
    phasesInRing2 = [startingPhase2]

    # Obtained planned signal phase of cycle1,2,3 for ring 1. There will be 8 phases.
    phasesInRing1 = phaseGroupInRing(startingPhase1, r1_phases, phasesInRing1)
    print("Phases In Ring1", phasesInRing1)

    # Obtained planned signal phase of cycle1,2,3 for ring 2. There will be 8 phases
    phasesInRing2 = phaseGroupInRing(startingPhase2, r2_phases, phasesInRing2)
    print("Phases In Ring2", phasesInRing2)
    # obtained init time and green elapssed time
    init1, init2, grn1, grn2 = getInitToPhasesAndElaspedGreenTime().split()

    ################## For Ring1##################

    # Obatined ring wise phase phaseDuration for left and right critical points
    left_R1_CP_phase_times = getPhaseDuration(
        left_R1_CP_phase_times, startingPhase1, 'Left', 'Ring1')
    right_R1_CP_phase_times = getPhaseDuration(
        right_R1_CP_phase_times, startingPhase1, 'Right', 'Ring1')
    print("Left Critical Points Phase Duration for Ring1 =", left_R1_CP_phase_times)
    print("Right Critical Points Phase Duration for Ring1 =",
          right_R1_CP_phase_times)

    # creating cumulative list
    # getCummulativePhaseTimes(startingPhase1, startingPhase2, ring_Phase_Times, phasesInRing, cum_phaseInRing, ringNo)
    cum_Left_Ring1_Phase_Times, cum_Right_Ring1_Phase_Times, phasesInRing1, cum_phaseInRing1 = getCummulativePhaseTimes(
        startingPhase1, startingPhase2, left_R1_CP_phase_times, right_R1_CP_phase_times, phasesInRing1, cum_phaseInRing1, "Ring1")

    print("Phases In Ring1", phasesInRing1)
    print("Cumulative Left Critical Points Phase times for Ring1 =",
          cum_Left_Ring1_Phase_Times)
    print("Cumulative Right Critical Points Phase times for Ring1 =",
          cum_Right_Ring1_Phase_Times)
    print("Cumulative Phases in Ring1 =", cum_phaseInRing1)

    ################## For Ring2 ##################

    left_R2_CP_phase_times = getPhaseDuration(
        left_R2_CP_phase_times, startingPhase2, 'Left', 'Ring2')
    right_R2_CP_phase_times = getPhaseDuration(
        right_R2_CP_phase_times, startingPhase2, 'Right', 'Ring2')
    print("Left Critical Points Phase times for Ring2 =", left_R2_CP_phase_times)
    print("Right Critical Points Phase times for Ring2 =", right_R2_CP_phase_times)

    cum_Left_Ring2_Phase_Times, cum_Right_Ring2_Phase_Times, phasesInRing2, cum_phaseInRing2 = getCummulativePhaseTimes(
        startingPhase1, startingPhase2, left_R2_CP_phase_times, right_R2_CP_phase_times, phasesInRing2, cum_phaseInRing2, "Ring2")

    print("Phases In Ring2", phasesInRing2)
    print("Cumulative Left Critical Points Phase times for Ring2 =",
          cum_Left_Ring2_Phase_Times)
    print("Cumulative Right Critical Points Phase times for Ring2 =",
          cum_Right_Ring2_Phase_Times)
    print("Cumulative Phases in Ring2 =", cum_phaseInRing2)

    timePhaseDiagramMethod(startingPhase1, startingPhase2, cum_Left_Ring1_Phase_Times, cum_Right_Ring1_Phase_Times,
                           cum_phaseInRing1, cum_Left_Ring2_Phase_Times, cum_Right_Ring2_Phase_Times,
                           cum_phaseInRing2, phasesInRing1, phasesInRing2, ETA, ETA_Duration, req_phase, secondPriorityRequestphases, secondPriorityRequestETA, secondPriorityRequestETA_Duration, 'Ring1&2')


if __name__ == '__main__':
    main()
