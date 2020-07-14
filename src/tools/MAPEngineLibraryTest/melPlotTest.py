# -*- coding: utf-8 -*-
"""
Created on Tue Jun 25 02:18:08 2019

@author: Debashis Das
"""

import matplotlib.pyplot as plt
import pandas as pd
import os
import glob

 
#Reading csv file and plotting points
columns = ['lat','lon', 'elev', 'heading', 'output']

df= pd.read_csv('Local_GPS_Coordinates.txt', delimiter=',', names=columns)
df1 = df[df.output == 0]
df2 = df[df.output == 1 ]

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
fig_size=plt.rcParams["figure.figsize"]
fig_size[0] = 10
fig_size[1] = 10
plt.rcParams["figure.figsize"] = fig_size

df1.plot.scatter(x="lon", y="lat", ax = ax, c = "blue", marker='o',label='Vehicle Not IN MAP')
df2.plot.scatter(x="lon", y="lat", ax = ax, c = "red", marker='o', label='Vehicle IN MAP')

#Readng csv files to plot MAP points
#columns_map = ['lat','lon', 'elev']
#df_map= pd.read_csv('Local_Map_GPSCoordinates.txt', delimiter=',', names=columns_map)
#df_map.plot.scatter(x="lon", y="lat", ax = ax, c = "black", marker='o',label='MAP Points')

#Deleting txt files
files=glob.glob('*.txt')
for filename in files:
    os.remove(filename)

#setting axis limit
plt.xlim(-600, 600)
plt.ylim(-600, 600)

# Set the line chart title and the text font size.
plt.title("MAP Engine Library Test", fontsize=19)

# Set x axes label.
plt.xlabel("West-East", fontsize=16)

# Set y axes label.
plt.ylabel("South-North", fontsize=16)

# Set the x, y axis tick marks text size.
plt.tick_params(axis='both', labelsize=12)

# Display the plot in the matplotlib's viewer.
plt.legend(loc='best')
plt.legend()
plt.show()