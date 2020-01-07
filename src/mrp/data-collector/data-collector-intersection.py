import socket
import json
import datetime
import os
import DataCollectorMethods as DCM 

appLocation = 1 # 0 = vehicle; 1 = infrastructure
flushLogFileEvery = 6001 # Seconds

def main():
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = config["DataCollectorIP"]
    port = config["PortNumber"]["DataCollector"]
    dataCollectorAddress = (hostIp, port)
    s.bind(dataCollectorAddress)

    logFileCreationDay = datetime.datetime.now().minute

    spatLogFile = open(("spatLog_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
    DCM.initializeSpatLogFile(spatLogFile)
    spatLogCounter = 1

    surroundingBsmLogFile = open(("surroundingBsmLog_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
    DCM.initializeBsmLogFile(surroundingBsmLogFile)
    surroundingBsmLogCounter = 1

    while True:
        currentDay = datetime.datetime.now().minute
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
            print("MinuteHasChanged")
            logFileCreationDay = currentDay
            spatLogFile.close()

            os.system('icd /iplant/home/nvaltekar/')
            os.system('iput ./spatLog*')
            os.system('rm ./spatLog*')

            surroundingBsmLogFile.close()
            spatLogFile = open(("spatLog_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
            surroundingBsmLogFile = open(("surroundingBsmLog_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
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