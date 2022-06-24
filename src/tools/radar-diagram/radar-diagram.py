import matplotlib.pyplot as plt
import pandas as pd
from math import pi
import json
import os
import glob
 
# print(df)
configFile = open("configuration.json", 'r')
config = (json.load(configFile))
# Close the config file:
configFile.close()
filePath = config["FileDirectory"]
pd.read_csv(filePath, header=None).T.to_csv('data1.csv', header=False, index=False)
df = pd.read_csv('data1.csv')#, index_col=0) 
print(df)
#remove the csv file
# files=glob.glob('*.csv')
# for filename in files:
#     os.remove(filename)
# number of variable
categories=list(df)[1:]
N = len(categories)
 
# We are going to plot the first line of the data frame.
# But we need to repeat the first value to close the circular graph:
values=df.loc[0].drop('group').values.flatten().tolist()
# values = df.iloc[0]
values += values[:1]
print(values)
 
# What will be the angle of each axis in the plot? (we divide the plot / number of variable)
angles = [n / float(N) * 2 * pi for n in range(N)]
angles += angles[:1]

# Initialise the spider plot
ax = plt.subplot(111, polar=True)
 
# If you want the first axis to be on top:
ax.set_theta_offset(pi /config["Offset"])
ax.set_theta_direction(-1)
 
# Draw one axe per variable + add labels labels yet
plt.xticks(angles[:-1], categories, color="black", size = 14)
 
# Draw ylabels
count = 0
noOfCircle = config["NoOfCircle"]
yTickList = []
yTickStringList = []
while (count < noOfCircle):
        val = config["ytick"][count]
        yTickList.append(val)
        stringVal = config["ytickString"][count]
        yTickStringList.append(stringVal)
        count = count + 1
ax.set_rlabel_position(0)
plt.yticks(yTickList, yTickStringList, color="black", size=14)
plt.ylim(0,yTickList[-1]+10)
# plt.yticks([10,20,30,40,50,60,70,80,90,100,110,120,130,140,150], ["10","20","30","40","50","60","70","80","90","100","110","120","130","140","150"], color="grey", size=7)
# plt.ylim(0,160)
 
 
# ------- PART 2: Add plots
 
# Plot each individual = each line of the data
# I don't do a loop, because plotting more than 3 groups makes the chart unreadable
 
# Ind1
values=df.loc[0].drop('group').values.flatten().tolist()
values += values[:1]
ax.plot(angles, values, linewidth = 3, linestyle = 'solid', label = "Preemption")
ax.fill(angles, values, 'b', alpha = 0.1)
 
# Ind2
values=df.loc[1].drop('group').values.flatten().tolist()
values += values[:1]
ax.plot(angles, values, color= "red", linewidth = 3, linestyle = 'solid', label = "MMITSS Emergency Vehicle Priority")
ax.fill(angles, values, 'r', alpha = 0.1)
 
# Add legend
plt.legend(loc='upper right', bbox_to_anchor=(1.40, 1.15), fontsize = 18)
plt.show()