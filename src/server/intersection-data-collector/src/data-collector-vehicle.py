import socket
import json
import datetime
import DataCollectorMethods as DCM
import sys
import sh
import time

DEBUGGING = False

def main():

    mmitssConfigFile = open('/nojournal/bin/mmitss-phase3-master-config.json', 'r')
    mmitssConfig = json.load(mmitssConfigFile)

    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = mmitssConfig["DataCollectorIP"]
    port = mmitssConfig["PortNumber"]["DataCollector"]
    dataCollectorAddress = (hostIp, port)
    s.bind(dataCollectorAddress)


    hostBsmLogFile, currentHostBsmLogFilename = DCM.initializeBsmLogFile('Host')
    surroundingBsmLogFile, currentSurroundingBsmFilename = DCM.initializeBsmLogFile('Surrounding')

    while True:
            DCM.receiveProcessAndStoreVehicleDataLocally(s, hostBsmLogFile, surroundingBsmLogFile)
    s.close()
    hostBsmLogFile.close()
    surroundingBsmLogFile.close()

if __name__ == "__main__":
    main()
