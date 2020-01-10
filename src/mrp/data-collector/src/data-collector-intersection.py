import socket
import json
import datetime
import DataCollectorMethods as DCM
import sys
import sh

flushLogFileEvery = 6001 # Seconds

def main():

    intersectionConfigFile = open(sys.argv[1], 'r')    
    intersectionConfig = (json.load(intersectionConfigFile))
    IntersectionName = intersectionConfig["IntersectionName"]

    dataCollectorConfigFile = open('./../config/data-collection-module-config.json', 'r')
    dataCollectorConfig = json.load(dataCollectorConfigFile)

    ################## CyVerse Directory Paths ##################
    CyVerse_DirectoryPath_Spat = intersectionConfig["CyVerse_DirectoryPath"]["Spat"]
    CyVerse_DirectoryPath_SurroundingBsms = intersectionConfig["CyVerse_DirectoryPath"]["SurroundingBsms"]
    CyVerse_DirectoryPath_PseudoSurroundingBsms = intersectionConfig["CyVerse_DirectoryPath"]["PseudoSurroundingBsms"]

    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = dataCollectorConfig["DataCollectorIp"]
    port = intersectionConfig["DataCollectorPort"]
    dataCollectorAddress = (hostIp, port)
    s.bind(dataCollectorAddress)

    logFileCreationDay = datetime.datetime.now().minute

    spatLogFile, currentSpatFilename = DCM.initializeSpatLogFile(IntersectionName)
    spatLogCounter = 1

    surroundingBsmLogFile, currentSurroundingBsmFilename = DCM.initializeBsmLogFile(IntersectionName)
    surroundingBsmLogCounter = 1

    while True:
        currentDay = datetime.datetime.now().minute
        if currentDay == logFileCreationDay:
            DCM.receiveProcessAndStoreDataLocally(s, spatLogFile, surroundingBsmLogFile)
        else:
            logFileCreationDay = currentDay

            # Close the file so that it can be transferred to CyVerse and deleted from local file system            
            spatLogFile.close()
            surroundingBsmLogFile.close()

            # Interaction with CyVerse:
            DCM.transferToCyVerseAndDeleteLocal(CyVerse_DirectoryPath_Spat, currentSpatFilename)
            DCM.transferToCyVerseAndDeleteLocal(CyVerse_DirectoryPath_SurroundingBsms, currentSurroundingBsmFilename)      

            # Open a new log file with new timestamp and initialize it.
            spatLogFile, currentSpatFilename = DCM.initializeSpatLogFile(IntersectionName)
            surroundingBsmLogFile, currentSurroundingBsmFilename = DCM.initializeBsmLogFile(IntersectionName)

            # Start collection again
            DCM.receiveProcessAndStoreDataLocally(s, spatLogFile, surroundingBsmLogFile)

    s.close()
    spatLogFile.close()
    surroundingBsmLogFile.close()

if __name__ == "__main__":
    main()