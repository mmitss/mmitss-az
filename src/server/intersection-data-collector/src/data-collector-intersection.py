'''
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  data-collector-intersection.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

***************************************************************************************
'''
import socket
import json
import datetime
import DataCollectorMethods as DCM
import sys
import sh
import time

DEBUGGING = False

def main():

    intersectionConfigFile = open(sys.argv[1], 'r')    
    intersectionConfig = (json.load(intersectionConfigFile))
    IntersectionName = intersectionConfig["IntersectionName"]

    dataCollectorConfigFile = open('./../config/data-collection-module-config.json', 'r')
    dataCollectorConfig = json.load(dataCollectorConfigFile)

    ################## CyVerse Directory Paths ##################
    CyVerse_DirectoryPath_Spat = intersectionConfig["CyVerse_DirectoryPath"]["Spat"]
    CyVerse_DirectoryPath_SurroundingBsms = intersectionConfig["CyVerse_DirectoryPath"]["SurroundingBsms"]
    CyVerse_DirectoryPath_Srm = intersectionConfig["CyVerse_DirectoryPath"]["Srm"]
    CyVerse_DirectoryPath_Ssm = intersectionConfig["CyVerse_DirectoryPath"]["Ssm"]

    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = dataCollectorConfig["DataCollectorIp"]
    port = intersectionConfig["DataCollectorPort"]
    dataCollectorAddress = (hostIp, port)
    s.bind(dataCollectorAddress)

    emailServerPort = dataCollectorConfig["DataCollectorEmailServer"]["Port"]
    emailServerAddress = (hostIp, emailServerPort)

    if DEBUGGING: logFileCreationDay = datetime.datetime.now().minute
    else: logFileCreationDay = datetime.datetime.now().day

    spatLogFile, currentSpatFilename = DCM.initializeSpatLogFile(IntersectionName)
    surroundingBsmLogFile, currentSurroundingBsmFilename = DCM.initializeBsmLogFile(IntersectionName)
    srmLogFile, currentSrmFilename = DCM.initializeSrmLogFile(IntersectionName)
    ssmLogFile, currentSsmFilename = DCM.initializeSsmLogFile(IntersectionName)
    previousSignalStatus = 0

    while True:
        if DEBUGGING: currentDay = datetime.datetime.now().minute
        else: currentDay = datetime.datetime.now().day
        if currentDay == logFileCreationDay:
            signalStatus = DCM.receiveProcessAndStoreIntersectionDataLocally(s, spatLogFile, surroundingBsmLogFile, srmLogFile, ssmLogFile)
            if signalStatus == None:
                pass
            elif signalStatus == previousSignalStatus:
                pass
            else :
                previousSignalStatus = signalStatus
                signalStatusFile = open(("./../datalogs/signal_status/" + IntersectionName + ".json"), 'w')
                signalStatusDict = {
                    "SignalStatus": signalStatus
                }
                signalStatusFile.write(json.dumps(signalStatusDict))
                signalStatusFile.close()

        else:
            logFileCreationDay = currentDay

            # Close the file so that it can be transferred to CyVerse and deleted from local file system            
            spatLogFile.close()
            surroundingBsmLogFile.close()
            srmLogFile.close()
            ssmLogFile.close()

            # Interaction with CyVerse
            #spatLogSizeBytes = DCM.transferToCyVerseAndDeleteLocal(CyVerse_DirectoryPath_Spat, currentSpatFilename)
            #bsmLogSizeBytes = DCM.transferToCyVerseAndDeleteLocal(CyVerse_DirectoryPath_SurroundingBsms, currentSurroundingBsmFilename)
            #srmLogSizeBytes = DCM.transferToCyVerseAndDeleteLocal(CyVerse_DirectoryPath_Srm, currentSrmFilename)
            #ssmLogSizeBytes = DCM.transferToCyVerseAndDeleteLocal(CyVerse_DirectoryPath_Ssm, currentSsmFilename)
            #totalTransferSizeBytes = spatLogSizeBytes + bsmLogSizeBytes + srmLogSizeBytes + ssmLogSizeBytes
            #notification = str(totalTransferSizeBytes)
            
            # Send notification to email server
            #s.sendto(notification.encode(),emailServerAddress)
            
            # Open a new log file with new timestamp and initialize it
            spatLogFile, currentSpatFilename = DCM.initializeSpatLogFile(IntersectionName)
            surroundingBsmLogFile, currentSurroundingBsmFilename = DCM.initializeBsmLogFile(IntersectionName)
            srmLogFile, currentSrmFilename = DCM.initializeSrmLogFile(IntersectionName)
            ssmLogFile, currentSsmFilename = DCM.initializeSsmLogFile(IntersectionName)

            # Start collection again
            DCM.receiveProcessAndStoreIntersectionDataLocally(s, spatLogFile, surroundingBsmLogFile, srmLogFile, ssmLogFile)

    s.close()
    spatLogFile.close()
    surroundingBsmLogFile.close()
    srmLogFile.close()
    ssmLogFile.close()

if __name__ == "__main__":
    main()
