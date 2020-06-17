import numpy as np
import matplotlib.pyplot as plt


def getStartingPhases():
    with open('Results.txt') as f:
        first_line = f.readline()
    return first_line


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
    for i in ring_phases:
        if i > SP:
            phasesInRing.append(i)
    # # Need to delete the phases based on starting phase
    # phasesInRing.pop(len(phasesInRing)-(SP-1))
    # print(phaseInRing)
    return phasesInRing


def getInitToPhasesAndElaspedGreenTime():
    with open('Results.txt') as f:

        for i, line in enumerate(f):
            if i == 1:
                break
    return line


def getPhaseTimesForCycle1(phase_Times, SP, CP):
    if(CP == 'Left'):
        with open('Results.txt') as f:
            for i, line in enumerate(f):
                if i == 2:
                    break
        durationOfP1K1R1, durationOfP2K1R1, durationOfP3K1R1, durationOfP4K1R1, durationOfP5K1R1, durationOfP6K1R1, durationOfP7K1R1, durationOfP8K1R1 = line.split()
        # print(line)

        left_r1_k1_Phase_Times = [float(durationOfP1K1R1), float(
            durationOfP2K1R1), float(durationOfP3K1R1), float(durationOfP4K1R1)]

        if SP > 1:
            left_r1_k1_Phase_Times = left_r1_k1_Phase_Times[SP-1:]
        
        # For cycle2 Left CP
        with open('Results.txt') as f:
            for i, line in enumerate(f):
                if i == 3:
                    break
        durationOfP1K2R1, durationOfP2K2R1, durationOfP3K2R1, durationOfP4K2R1, durationOfP5K2R1, durationOfP6K2R1, durationOfP7K2R1, durationOfP8K2R1 = line.split()

        left_r1_k2_Phase_Times = [float(durationOfP1K2R1), float(
            durationOfP2K2R1), float(durationOfP3K2R1), float(durationOfP4K2R1)]
        left_r1_k1_Phase_Times.extend(left_r1_k2_Phase_Times)

        # For cycle3 Left CP
        with open('Results.txt') as f:
            for i, line in enumerate(f):
                if i == 4:
                    break
        durationOfP1K3R1, durationOfP2K3R1, durationOfP3K3R1, durationOfP4K3R1, durationOfP5K3R1, durationOfP6K3R1, durationOfP7K3R1, durationOfP8K3R1 = line.split()

        left_r1_k2_Phase_Times = [float(durationOfP1K3R1), float(
            durationOfP2K3R1), float(durationOfP3K3R1), float(durationOfP4K3R1)]
        left_r1_k1_Phase_Times.extend(left_r1_k2_Phase_Times)
        del left_r1_k1_Phase_Times[8:]
        phase_Times = left_r1_k1_Phase_Times

    # # # For cycle1 Right CP
    if(CP == 'Right'):
        with open('Results.txt') as f:
            for i, line in enumerate(f):
                if i == 5:
                    break
        durationOfP1K1R2, durationOfP2K1R2, durationOfP3K1R2, durationOfP4K1R2, durationOfP5K1R2, durationOfP6K1R2, durationOfP7K1R2, durationOfP8K1R2 = line.split()
        # print(line)

        right_r1_k1_Phase_Times = [float(durationOfP1K1R2), float(
            durationOfP2K1R2), float(durationOfP3K1R2), float(durationOfP4K1R2)]
        if SP > 1:
            right_r1_k1_Phase_Times = right_r1_k1_Phase_Times[SP-1:]

        # For cycle2 Right CP
        with open('Results.txt') as f:
            for i, line in enumerate(f):
                if i == 6:
                    break
        durationOfP1K2R2, durationOfP2K2R2, durationOfP3K2R2, durationOfP4K2R2, durationOfP5K2R2, durationOfP6K2R2, durationOfP7K2R2, durationOfP8K2R2 = line.split()

        right_r1_k2_Phase_Times = [float(durationOfP1K2R2), float(
            durationOfP2K2R2), float(durationOfP3K2R2), float(durationOfP4K2R2)]
        right_r1_k1_Phase_Times.extend(right_r1_k2_Phase_Times)

        # For cycle3 Right CP
        with open('Results.txt') as f:
            for i, line in enumerate(f):
                if i == 4:
                    break
        durationOfP1K3R2, durationOfP2K3R2, durationOfP3K3R2, durationOfP4K3R2, durationOfP5K3R2, durationOfP6K3R2, durationOfP7K3R2, durationOfP8K3R2 = line.split()

        right_r1_k3_Phase_Times = [float(durationOfP1K3R2), float(
            durationOfP2K3R2), float(durationOfP3K3R2), float(durationOfP4K3R2)]
        right_r1_k1_Phase_Times.extend(right_r1_k3_Phase_Times)
        del right_r1_k1_Phase_Times[8:]
        phase_Times = right_r1_k1_Phase_Times

    return phase_Times

def getCummulativePhaseTimes(r1_Phase_Times):
    cum_Ring1_Phase_Times = []
    cum_Ring1_Phase_Times = np.cumsum(r1_Phase_Times)

    ##Appending 0 in the  beiginning of the list.
    cum_Ring1_Phase_Times = np.insert(cum_Ring1_Phase_Times, 0,0) #First 0 is position
    return cum_Ring1_Phase_Times
    
def getPriorityRequest():
    eta = []
    with open('Results.txt') as f:
            for i, line in enumerate(f):
                if i == 15:
                    break
    val1,Rl,Ru,val2, val3 = line.split()
        # print(line)

    eta = [float(Rl), float(Ru)]
    return eta            


def timePhaseDiagram(cum_Left_Ring1_Phase_Times, cum_Right_Ring1_Phase_Times, cum_phaseInRing1,ETA,phasesInRing1,phasesInRing2):
    #cum_phaseInRing1 = [0,10,20,30,40,50,60,70,80,90] 
    #um_phaseInRing2 = cum_phaseInRing1
    #Plotting time-phase diagram
    fig, ax1 = plt.subplots()

    #Requested phase
    req_phase = 2 #added by kelsey   
    pos = phasesInRing1.index(req_phase)
    pos = cum_phaseInRing1[pos]

    color = 'tab:red'
    ax1.set_xlabel('time (s)')
    ax1.set_ylabel('Ring 1', color=color)
    ax1.plot(cum_Left_Ring1_Phase_Times, cum_phaseInRing1, color=color)
    ax1.set_yticks(ticks=np.arange(cum_phaseInRing1[0], cum_phaseInRing1[-1],10)) #added by Kelsey
    ax1.set_yticklabels(phasesInRing1) #added by Kelsey
    ax1.tick_params(axis='y', labelcolor=color)

    ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

    color = 'tab:blue'
    ax2.set_ylabel('Ring 2', color=color)  # we already handled the x-label with ax1
    ax2.plot(cum_Right_Ring1_Phase_Times, cum_phaseInRing1, color=color)
    ax2.set_yticks(ticks=np.arange(cum_phaseInRing1[0], cum_phaseInRing1[-1], 10)) #added by Kelsey
    ax2.set_yticklabels(phasesInRing2) #added by Kelsey
    #ax2.set_ylabel(phaseInRing2)
    ax2.tick_params(axis='y', labelcolor=color)
    rect = plt.Rectangle((ETA[0],pos), ETA[1]-ETA[0], 10, color='r', alpha=0.3)
    ax1.add_patch(rect)
    fig.tight_layout()  # otherwise the right y-label is slightly clipped
    plt.grid()
    plt.show()


def main():
    r1_phases = [1, 2, 3, 4]
    r2_phases = (5, 6, 7, 8)
    left_R1_CP_phase_times = []
    right_R1_CP_phase_times = []
    cum_Left_Ring1_Phase_Times = []
    cum_Right_Ring1_Phase_Times = []
    cum_phaseInRing1 = [0]
    ETA = []

    SP1, SP2 = getStartingPhases().split()
    print("SP1 =", SP1)
    print("SP2 =", SP2)
    SP1 = int(SP1)
    SP2 = int(SP2)
    phasesInRing1 = [SP1]
    phasesInRing2 = [SP2]

    phasesInRing1 = phaseGroupInRing(SP1, r1_phases, phasesInRing1)
    print(phasesInRing1)
    phasesInRing2 = phaseGroupInRing(SP2, r2_phases, phasesInRing2)
    print(phasesInRing2)

    init1, init2, grn1, grn2 = getInitToPhasesAndElaspedGreenTime().split()
    print("ini1 =", init1)
    print("ini2 =", init2)
    print("Elapesd Green1 =", grn1)
    print("Elapesd Green2 =",grn2)
    left_R1_CP_phase_times = getPhaseTimesForCycle1(left_R1_CP_phase_times, SP1, 'Left')
    right_R1_CP_phase_times = getPhaseTimesForCycle1(right_R1_CP_phase_times, SP1, 'Right')
    print("Left Critical Points Phase times for Ring1 =", left_R1_CP_phase_times)
    print("Right Critical Points Phase times for Ring1 =",right_R1_CP_phase_times)
    # #creating cumulative list
    cum_Left_Ring1_Phase_Times = getCummulativePhaseTimes(left_R1_CP_phase_times)
    cum_Right_Ring1_Phase_Times = getCummulativePhaseTimes(right_R1_CP_phase_times)

    print("Cumulative Left Critical Points Phase times for Ring1 =",cum_Left_Ring1_Phase_Times)
    print("Cumulative Right Critical Points Phase times for Ring1 =",cum_Right_Ring1_Phase_Times)
    x=0
    length = len(cum_Left_Ring1_Phase_Times)-1
    for i in range(length):
        x = x+10
        cum_phaseInRing1.append(x)
    print("Cumulative Phases in Ring1 =",cum_phaseInRing1)
    ETA = getPriorityRequest()
    print("ETA", ETA)
    timePhaseDiagram(cum_Left_Ring1_Phase_Times,cum_Right_Ring1_Phase_Times,cum_phaseInRing1,ETA,phasesInRing1,phasesInRing2)



if __name__ == '__main__':
    main()
