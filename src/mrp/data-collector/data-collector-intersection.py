import socket
import json
import datetime
import os
import DataCollectorMethods as DCM
import glob
import sys
import sh

appLocation = 1 # 0 = vehicle; 1 = infrastructure
flushLogFileEvery = 6001 # Seconds

def main():

    configFile = open(sys.argv[1], 'r')    
    config = (json.load(configFile))
    IntersectionName = config["IntersectionName"]

    ################## CyVerse Directory Paths ##################
    CyVerse_DirectoryPath_Spat = config["CyVerse_DirectoryPath"]["Spat"]
    CyVerse_DirectoryPath_SurroundingBsms = config["CyVerse_DirectoryPath"]["SurroundingBsms"]
    CyVerse_DirectoryPath_PseudoSurroundingBsms = config["CyVerse_DirectoryPath"]["PseudoSurroundingBsms"]

    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = config["DataCollectorIp"]
    port = config["DataCollectorPort"]
    dataCollectorAddress = (hostIp, port)
    s.bind(dataCollectorAddress)

    logFileCreationDay = datetime.datetime.now().minute

    currentSpatFilename = "spatLog_" + IntersectionName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"
    spatLogFile = open(("spatLog_" + IntersectionName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
    DCM.initializeSpatLogFile(spatLogFile)
    spatLogCounter = 1

    currentSurroundingBsmFilename = "surroundingBsmLog_" + IntersectionName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"
    surroundingBsmLogFile = open((currentSurroundingBsmFilename), 'w')
    DCM.initializeBsmLogFile(surroundingBsmLogFile)
    surroundingBsmLogCounter = 1

    while True:
        currentDay = datetime.datetime.now().minute
        if currentDay == logFileCreationDay:
            data, address = s.recvfrom(4096)
            jsonData = json.loads(data.decode())
            if jsonData["MsgType"]=="SPAT": # then this message is a SPAT message
                spatLogFile.write(DCM.spatJsonToCsv(jsonData))
                spatLogCounter = spatLogCounter+1
                if spatLogCounter > flushLogFileEvery: # flush every 1 minutes
                    spatLogFile.flush()
                    spatLogCounter = 1
            elif jsonData["MsgType"]=="BSM": # then this message is a bsm message from 
                surroundingBsmLogFile.write(DCM.bsmJsonToCsv(jsonData))
                surroundingBsmLogCounter = surroundingBsmLogCounter+1
                if surroundingBsmLogCounter > flushLogFileEvery: # flush every 1 minutes
                    surroundingBsmLogFile.flush()
                    surroundingBsmLogCounter = 1
        else:
            logFileCreationDay = currentDay

            # Close the file so that it can be transferred to CyVerse and deleted from local file system            
            spatLogFile.close()

            # Interaction with CyVerse:
            sh.icd(CyVerse_DirectoryPath_Spat) # Go to correct CyVerse directory for storing SPAT data
            sh.iput(currentSpatFilename) # Upload all files of this intersection to current directory of CyVerse
            sh.rm(currentSpatFilename) # Remove the files of this intersection from local storage

            # Open a new log file with new timestamp and initialize it.
            currentSpatFilename = "spatLog_" + IntersectionName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"
            spatLogFile = open(currentSpatFilename, 'w')
            DCM.initializeSpatLogFile(spatLogFile)

            # Close the file so that it can be transferred to CyVerse and deleted from local file system            
            surroundingBsmLogFile.close()

            # Interaction with CyVerse:
            sh.icd(CyVerse_DirectoryPath_Spat) # Go to correct directory for storing SPAT data
            sh.iput(currentSurroundingBsmFilename) # Upload all files matching to current directory
            sh.rm(currentSurroundingBsmFilename) # Remove the file from local storage            

            # Open a new log file with new timestamp and initialize it.
            currentSurroundingBsmFilename = "surroundingBsmLog_" + IntersectionName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"
            surroundingBsmLogFile = open((currentSurroundingBsmFilename), 'w')
            DCM.initializeBsmLogFile(surroundingBsmLogFile)
            
            data, address = s.recvfrom(4096)
            jsonData = json.loads(data.decode())
            if jsonData["MsgType"]=="SPAT": # then this message is a SPAT message
                spatLogFile.write(DCM.spatJsonToCsv(jsonData))
                spatLogCounter = spatLogCounter+1
                if spatLogCounter > flushLogFileEvery: # flush every 1 minutes calculated using the frequency of received message
                    spatLogFile.flush()
                    spatLogCounter = 1
            elif jsonData["MsgType"]=="BSM": # then this message is a bsm message from 
                surroundingBsmLogFile.write(DCM.bsmJsonToCsv(jsonData))
                surroundingBsmLogCounter = surroundingBsmLogCounter+1
                if surroundingBsmLogCounter > flushLogFileEvery: # flush every 1 minutes 
                    surroundingBsmLogFile.flush()
                    surroundingBsmLogCounter = 1

    s.close()
    spatLogFile.close()
    surroundingBsmLogFile.close()

if __name__ == "__main__":
    main()