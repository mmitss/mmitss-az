'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  DsrcBypass.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is an initial revision. This application allows to bypass the dsrc devices for testing. Do not run MsgSender and MsgReceiver while running this application.
'''

import socket
import json

def main():
    
    DEBUGGING = True

    # Open configuration file and load the data into JSON object
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r').read()
    config = (json.loads(configFile))
    if DEBUGGING: print("Configuration file read successfully.")

    # From config Json object, get the hostIp and Port for this application.
    hostIp = config["HostIp"]
    dsrcBypassPort = config["PortNumber"]["MessageTransceiver"]['MessageSender']
    hostComm = (hostIp, dsrcBypassPort)

    # Create a socket
    outerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind the created socket to the server information.
    outerSocket.bind(hostComm)

    sourceDsrcDeviceIP = config["SourceDsrcDeviceIp"]
    sourceDsrcDevicePort = config["PortNumber"]["MessageTransceiver"]['MessageDecoder']
    sourceDsrcDevice = (sourceDsrcDeviceIP, sourceDsrcDevicePort)
    #For sending to second Laptop 
    # sourceDsrcDeviceIP2 = "10.12.6.57"
    # sourceDsrcDevicePort2 = 10004
    # sourceDsrcDevice2 = (sourceDsrcDeviceIP2, sourceDsrcDevicePort2)

    while True:
        receivedMsg, addr = outerSocket.recvfrom(4096)
        outerSocket.sendto(receivedMsg, sourceDsrcDevice)
        # outerSocket.sendto(receivedMsg, sourceDsrcDevice2)
        print("Sent")

if __name__ == '__main__':
    main()

