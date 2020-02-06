import numpy as np
import matplotlib.pyplot as plt

with open('Results.txt') as f:
    first_line = f.readline()
print(first_line)
SP1, SP2 =first_line.split()
print("SP1 =", SP1)
print("SP2 =", SP2)

SP1 = int(SP1)
SP2 = int(SP2)

r1_phases = [1, 2, 3, 4]
r2_phases = (5, 6, 7, 8)
phaseInRing1 = [SP1]
phaseInRing2 = [SP2]


#Storing the values of StartingPhase of ring1 and based on that fill rest of phases in the list
i = SP1
for i in r1_phases:
    if i>SP1:
        phaseInRing1.append(i)
for i in r1_phases:  
    if i<SP1:
        phaseInRing1.append(i)
#Extending the phases for 3cycles
phaseInRing1.extend(phaseInRing1*2)
#Need to delete the phases based on starting phase
phaseInRing1.pop(len(phaseInRing1)-(SP1-1))
print (phaseInRing1)

#Storing the values of StartingPhase of ring2 and based on that fill rest of phases in the list
j = SP2

for j in r2_phases:
    if j>SP2:
        phaseInRing2.append(j)

for j in r2_phases:       
    if j<SP2:
        phaseInRing2.append(j)
#Extending the phases for 3cycles
phaseInRing2.extend(phaseInRing2*2)
#Need to delete the phases based on starting phase
phaseInRing2.pop(len(phaseInRing2)-(SP2-1))
print (phaseInRing2)



with open('Results.txt') as f:
    #line = f.readline()
    for i, line in enumerate(f):
        if i == 1:
            break

init1, init2, grn1, grn2 =line.split()
# print(init1)
# print(init2)
# print(grn1)
# print(grn2)



# For cycle1 ring1
with open('Results.txt') as f:
    for i, line in enumerate(f):
        if i == 2:
            break
durationOfP1K1R1, durationOfP2K1R1, durationOfP3K1R1, durationOfP4K1R1, durationOfP5K1R1, durationOfP6K1R1, durationOfP7K1R1, durationOfP8K1R1 = line.split()
#print(line)

r1k1_phase_times = [float(durationOfP1K1R1), float(durationOfP2K1R1), float(durationOfP3K1R1), float(durationOfP4K1R1)]


for i, item in enumerate (r1k1_phase_times, start=1):
    if i < SP1:
        r1k1_phase_times.pop(i-1)


# For cycle2 ring1
with open('Results.txt') as f:
    for i, line in enumerate(f):
        if i == 3:
            break
durationOfP1K2R1, durationOfP2K2R1, durationOfP3K2R1, durationOfP4K2R1, durationOfP5K2R1, durationOfP6K2R1, durationOfP7K2R1, durationOfP8K2R1 = line.split()

r1k2_phase_times = [float(durationOfP1K2R1), float(durationOfP2K2R1), float(durationOfP3K2R1), float(durationOfP4K2R1)]
r1k1_phase_times.extend(r1k2_phase_times)

# For cycle3 ring1
with open('Results.txt') as f:
    for i, line in enumerate(f):
        if i == 4:
            break
durationOfP1K3R1, durationOfP2K3R1, durationOfP3K3R1, durationOfP4K3R1, durationOfP5K3R1, durationOfP6K3R1, durationOfP7K3R1, durationOfP8K3R1 = line.split()

r1k3_phase_times = [float(durationOfP1K3R1), float(durationOfP2K3R1), float(durationOfP3K3R1), float(durationOfP4K3R1)]
r1k1_phase_times.extend(r1k3_phase_times)

print(r1k1_phase_times)



# For cycle1 ring2
with open('Results.txt') as f:
    for i, line in enumerate(f):
        if i == 5:
            break
durationOfP1K1R2, durationOfP2K1R2, durationOfP3K1R2, durationOfP4K1R2, durationOfP5K1R2, durationOfP6K1R2, durationOfP7K1R2, durationOfP8K1R2 = line.split()
#print(line)

r2k1_phase_times = [float(durationOfP5K1R2), float(durationOfP6K1R2), float(durationOfP7K1R2), float(durationOfP8K1R2)]


for i, item in enumerate (r2k1_phase_times, start=1):
    if i < SP2-4:
        r2k1_phase_times.pop(i-1)


# For cycle2 ring2
with open('Results.txt') as f:
    for i, line in enumerate(f):
        if i == 6:
            break
durationOfP1K2R2, durationOfP2K2R2, durationOfP3K2R2, durationOfP4K2R2, durationOfP5K2R2, durationOfP6K2R2, durationOfP7K2R2, durationOfP8K2R2 = line.split()

r2k2_phase_times = [float(durationOfP5K2R2), float(durationOfP6K2R2), float(durationOfP7K2R2), float(durationOfP8K2R2)]
r2k1_phase_times.extend(r2k2_phase_times)

# For cycle3 ring2
with open('Results.txt') as f:
    for i, line in enumerate(f):
        if i == 4:
            break
durationOfP1K3R2, durationOfP2K3R2, durationOfP3K3R2, durationOfP4K3R2, durationOfP5K3R2, durationOfP6K3R2, durationOfP7K3R2, durationOfP8K3R2 = line.split()

r2k3_phase_times = [float(durationOfP5K3R2), float(durationOfP6K3R2), float(durationOfP7K3R2), float(durationOfP8K3R2)]
r2k1_phase_times.extend(r2k3_phase_times)

print(r2k1_phase_times)


#creating cumulative list
cum_Ring1_Phase_Times = []
cum_Ring1_Phase_Times = np.cumsum(r1k1_phase_times)

cum_Ring2_Phase_Times = []
cum_Ring2_Phase_Times = np.cumsum(r2k1_phase_times)
##Appending 0 in the  beiginning of the list.
cum_Ring1_Phase_Times = np.insert(cum_Ring1_Phase_Times, 0,0)
cum_Ring2_Phase_Times = np.insert(cum_Ring2_Phase_Times, 0,0)

print(cum_Ring1_Phase_Times)
print(cum_Ring2_Phase_Times)
# for x in range(len(r1k1_phase_times)): 
#     print (r1k1_phase_times[x])


# with open('Results.txt') as f:
#     #line = f.readline()
#     for i, line in enumerate(f):
#         if i == 5:
#             break
# durationOfP1K1R2, durationOfP2K1R2, durationOfP3K1R2, durationOfP4K1R2, durationOfP5K1R2, durationOfP6K1R2, durationOfP7K1R2, durationOfP8K1R2 = line.split()
# print(line)


# r2k1_phase_times = [durationOfP5K1R2, durationOfP6K1R2, durationOfP7K1R2, durationOfP8K1R2]

# for i, item in enumerate (r2k1_phase_times, start=1):
#     if i < int(SP2)-4:
#         r2k1_phase_times.append(r2k1_phase_times[i-1])
#         r2k1_phase_times.pop(i-1)

# for x in range(len(r2k1_phase_times)): 
#     print (r2k1_phase_times[x])











# grn1 = float(grn1)

# cum_phaseInRing1=[grn1]
# i=1
# for i in range(10):
#     x= cum_phaseInRing1[i-1]+5
#     cum_phaseInRing1.append(x)
# print(cum_phaseInRing1)

# grn2 = float(grn2)

# cum_phaseInRing2=[grn2]
# i=1
# for i in range(10):
#     x= cum_phaseInRing2[i-1]+5
#     cum_phaseInRing2.append(x)
# print(cum_phaseInRing2)

cum_phaseInRing1 = [0,5,10,15,20,25,30,35,40,45,50,55] 
cum_phaseInRing2 = cum_phaseInRing1
#Plotting time-phase diagram
fig, ax1 = plt.subplots()

color = 'tab:red'
ax1.set_xlabel('time (s)')
ax1.set_ylabel('Ring 1', color=color)
ax1.plot(cum_Ring1_Phase_Times, cum_phaseInRing1, color=color)
ax1.tick_params(axis='y', labelcolor=color)

ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

color = 'tab:blue'
ax2.set_ylabel('Ring 2', color=color)  # we already handled the x-label with ax1
ax2.plot(cum_Ring2_Phase_Times, cum_phaseInRing2, color=color)
#ax2.set_ylabel(phaseInRing2)
ax2.tick_params(axis='y', labelcolor=color)

fig.tight_layout()  # otherwise the right y-label is slightly clipped
plt.show()