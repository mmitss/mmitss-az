# -*- coding: utf-8 -*-
"""
Created on Wed Jun 26 11:07:12 2019

@author: Debashis Das
"""

import numpy as np
import pandas as pd
import glob
import json
import os

pd.set_option("display.precision", 7)

#Define the range of GPS Coordinates

configFile = (open("mapEngineLibraryTestConfig.json", 'r')).read()
jsonObject= json.loads(configFile)

latmin = jsonObject["GPSSpace"]['LatitudeMin']
latmax =  jsonObject["GPSSpace"]['LatitudeMax']
lonmin = jsonObject["GPSSpace"]['LongitudeMin']
lonmax = jsonObject["GPSSpace"]['LongitudeMax']
n = 1000 #Number of points along each axis
deltalat = (latmax - latmin)/n
deltalon = (lonmax - lonmin)/n

lat_list= [latmin]
lon_list= [lonmin]

for i in range (1,n+1,1):
    c1= latmin+i*deltalat
    lat_list.append(c1)

for i in range (1,n+1,1):
    c2= lonmin+i*deltalon
    lon_list.append(c2)

#Creating csv files for diffrent heading
headingDeviation = jsonObject['HeadingDeviation']
 
for i in range (0,360,headingDeviation):
    index = pd.MultiIndex.from_product([lat_list, lon_list], names = ["lat", "lon"])
    df=pd.DataFrame(index = index).reset_index()
    df['elevation'] = jsonObject['ReferencePoint']['Elevation'] #Define the approximate Elevation
    df['heading'] = i
    df.to_csv("GPSCoordinates"+str(i)+".csv", index = None, header=None)

#merging csv files into txt file. Since excel has 1,048,576 rows, we are using txt file.
extension = 'csv'
all_filenames = [j for j in glob.glob('*.{}'.format(extension))]
combined_csv = pd.concat([pd.read_csv(f, header=None) for f in all_filenames],axis=0)
np.savetxt("GPSCoordinates.txt", combined_csv.values, fmt='%1.7f', delimiter ="\t")

#Deleting csv files
files=glob.glob('*.csv')
for filename in files:
    os.remove(filename)
