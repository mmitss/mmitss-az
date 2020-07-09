'''
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  video-data-collector.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

***************************************************************************************
'''
import os
import time
import datetime
import sys
import json
import DataCollectorMethods as DCM

cameraConfigFile = open(sys.argv[1], 'r')    
cameraConfig = (json.load(cameraConfigFile))
intersectionName = cameraConfig["IntersectionName"]
cameraDirection = cameraConfig["CameraDirection"]
url = cameraConfig["URL"]
localStorageDirectory = cameraConfig["LocalStorageDirectory"]
cyverseDirectory = cameraConfig["CyVerse_DirectoryPath"]

configFile = open('./../config/video-data-collection-module-config.json', 'r')
config = json.load(configFile)

totalLength_sec = config["TotalLength_Sec"]
sliceLength_sec = config["SliceLength_Sec"]
startTime = time.time()

FileName = localStorageDirectory + "/" + intersectionName + "_" + cameraDirection

while True:
    currentTime = time.time()
    if currentTime-startTime < totalLength_sec:
        currentFileName = FileName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".mpg"
        os.system("cvlc " + url + " --sout=file/ts:" + currentFileName + " --stop-time " + str(sliceLength_sec) + " vlc://quit")
        print ("Transferred to CyVerse!")
        #DCM.transferToCyVerseAndDeleteLocal(cyverseDirectory, currentFileName)
    else: break