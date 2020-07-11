'''
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  data-collector-vehicle.py  
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

    hostBsmDecoderPort = mmitssConfig["HostBsmDecoder"]

    hostBsmLogFile, currentHostBsmLogFilename = DCM.initializeBsmLogFile('Host')
    surroundingBsmLogFile, currentSurroundingBsmFilename = DCM.initializeBsmLogFile('Surrounding')
    spatLogFile, currentSpatLogFilename = DCM.initializeSpatLogFile('Vehicle')
    ssmLogFile, currentSsmLogFilename = DCM.initializeSsmLogFile('Vehicle')
    srmLogFile, currentSrmLogFilename = DCM.initializeSrmLogFile('Vehicle"')

    while True:
            DCM.receiveProcessAndStoreVehicleDataLocally(s, hostBsmDecoderPort, 
                                                            hostBsmLogFile, 
                                                            surroundingBsmLogFile,
                                                            spatLogFile,
                                                            srmLogFile,
                                                            ssmLogFile)
    s.close()
    hostBsmLogFile.close()
    surroundingBsmLogFile.close()
    spatLogFile.close()
    ssmLogFile.close()
    srmLogFile.close()

if __name__ == "__main__":
    main()
