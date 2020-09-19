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
import cv2
from VideoSpatRecorder import VideoSpatRecorder

cameraConfigFile = open(sys.argv[1], 'r')    
cameraConfig = (json.load(cameraConfigFile))
intersectionName = cameraConfig["IntersectionName"]
cameraDirection = cameraConfig["CameraDirection"]
url = cameraConfig["URL"]
localStorageDirectory = cameraConfig["LocalStorageDirectory"]
cyverseDirectory = cameraConfig["CyVerse_DirectoryPath"]
leftTurnPhase = cameraConfig["LeftTurnPhase"]
throughPhase = cameraConfig["ThroughPhase"]

configFile = open('/nojournal/bin/config/global_config.json', 'r')
config = json.load(configFile)

totalLength_sec = config["TotalLength_Sec"]
sliceLength_sec = config["SliceLength_Sec"]
totalStartTime = time.time()

signalStatusFileName = ('/nojournal/bin/signal_status/' + intersectionName + ".json")


fileName = localStorageDirectory + "/" + intersectionName + "_" + cameraDirection

vsr = VideoSpatRecorder(signalStatusFileName, leftTurnPhase, throughPhase)

cap = cv2.VideoCapture(url)

fourcc = cv2.VideoWriter_fourcc(*'XVID')

while(cap.isOpened()):
    
    if time.time()-totalStartTime < totalLength_sec:
    
        currentVideoFileName = fileName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".avi"
        currentMetadataFilename = currentVideoFileName.replace(".avi", "_meta.json")
        out = cv2.VideoWriter(currentVideoFileName, fourcc, 30, (1920,1080))
        metadataFile = open(currentMetadataFilename, 'w')
        sliceStartTime = time.time()
        metadata = {
            "Metadata_per_frame":[]
        }

        metadata_frame = []

        while time.time()-sliceStartTime < sliceLength_sec:
            
            ret, frame = cap.read()  
            if ret == True:
                vsr.drawBlankSignalHeads(frame)
                vsr.fillSignalHeads(frame)
                vsr.overlayTimestamp(frame)
                out.write(frame)
                metadata_frame = metadata_frame + [vsr.getMetadataJson()]

        
        metadata["Metadata_per_frame"] = metadata_frame
        metadataFile.write(json.dumps(metadata))
        metadataFile.close()
        out.release()    
    
    else: cap.release()
