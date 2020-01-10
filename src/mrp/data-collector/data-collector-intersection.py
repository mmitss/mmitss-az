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

    logFileCreationDay = datetime.datetime.now().day

    spatLogFile = open(("spatLog_" + IntersectionName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
    DCM.initializeSpatLogFile(spatLogFile)
    spatLogCounter = 1

    surroundingBsmLogFile = open(("surroundingBsmLog_" + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
    DCM.initializeBsmLogFile(surroundingBsmLogFile)
    surroundingBsmLogCounter = 1

    while True:
        currentDay = datetime.datetime.now().day
        if currentDay == logFileCreationDay:
            data, address = s.recvfrom(4096)
            jsonData = json.loads(data.decode())
            if address[1] == config["PortNumber"]["MapSPaTBroadcaster"]: # then this message is a SPAT message
                spatLogFile.write(DCM.spatJsonToCsv(jsonData))
                spatLogCounter = spatLogCounter+1
                if spatLogCounter > flushLogFileEvery: # flush every 1 minutes
                    spatLogFile.flush()
                    spatLogCounter = 1
            elif address[1] == config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]: # then this message is a bsm message from 
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
            sh.icd(CyVerse_DirectoryPath_Spat) # Go to correct directory for storing SPAT data
            sh.iput("./spatLog_" + IntersectionName + "*") # Upload all files matching to current directory
            sh.rm("./spatLog_*") # Remove the file from local storage

            # Open a new log file with new timestamp and initialize it.
            spatLogFile = open(("spatLog_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
            DCM.initializeSpatLogFile(spatLogFile)


            surroundingBsmLogFile.close()
            surroundingBsmLogFile = open(("surroundingBsmLog_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
            DCM.initializeBsmLogFile(surroundingBsmLogFile)
            data, address = s.recvfrom(4096)
            jsonData = json.loads(data.decode())
            if address[1] == config["PortNumber"]["MapSPaTBroadcaster"]: # then this message is a SPAT message
                spatLogFile.write(DCM.spatJsonToCsv(jsonData))
                spatLogCounter = spatLogCounter+1
                if spatLogCounter > flushLogFileEvery: # flush every 1 minutes
                    spatLogFile.flush()
                    spatLogCounter = 1
            elif address[1] == config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]: # then this message is a bsm message from 
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