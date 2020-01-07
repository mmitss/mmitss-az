'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  MapSpatBroadcaster.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision. MapSpatBroadcaster does the follwoing tasks: 
    -> Read configuration data: controllerIP, selfIP, mapPayload, regionalID, and IntersectionID.
    -> Receive SPAT data from the ctraffic controller (currently NTCIP1202v2 Blob: defined in separate class)
    -> Calculate the data required for trafficControllerObserver and J2735 SPAT message.
    -> Formulate a json string and send it to trafficControllerObserver and msgEncoder.
    -> Send the mapPayload as it is to msgSender.
    -> ### IMPORTANT ### If the format of NTCIP1202 blob changes in future (for example, NTCIP1202v3), a new class will be required to created which could be used in similar manner like NTCIP1202v2Blob class.
'''
import socket
import time
import json
import Ntcip1202v2Blob
import Spat

def main():

    # Read a config file by creating an object of the time MapSpatBroadcasterConfig
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))



    # Establish a socket and bind it to IP and port
    outerSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    mrpIp = config["HostIp"]
    port = 6053
    MapSpatBroadcastAddress = (mrpIp, port)
    outerSocket.bind(MapSpatBroadcastAddress)
    outerSocket.settimeout(3)

    msgSenderPort = config["PortNumber"]["MessageTransceiver"]["MessageSender"]
    msgSenderAddress = (mrpIp, msgSenderPort)

    msgEncoderPort = config["PortNumber"]["MessageTransceiver"]["MessageEncoder"]
    msgEncoderAddress = (mrpIp, msgEncoderPort)

    dataCollectorIp = config["DataCollectorIP"]
    dataCollectorPort = config["PortNumber"]["DataCollector"]
    dataCollectorAddress = (dataCollectorIp, dataCollectorPort)

    # Store map payload in a string
    mapPayload = config["MapPayload"]

    # Create an empty Ntcip1202v2Blob object to store the information to be received from the signal controller:
    currentBlob = Ntcip1202v2Blob.Ntcip1202v2Blob()

    # Create an object of Spat class filled with static information:
    spatObject = Spat.Spat()
    spatObject.setIntersectionID(config["IntersectionID"])
    spatObject.setRegionalID(config["RegionalID"])

    # Read controllerIp from the config file and store it.
    controllerIp = config["ControllerIp"]

    msgCnt = 0
    spatMapMsgCount = 0
    while True:
        try:
            spatBlob, addr = outerSocket.recvfrom(1024)            
            if addr[0] == controllerIp:
                currentBlob.processNewData(spatBlob)
                if(msgCnt < 127):
                    msgCnt = msgCnt + 1
                else: msgCnt = 0
                spatObject.setmsgCnt(msgCnt)
                spatObject.fillSpatInformation(currentBlob)
                spatJsonString = spatObject.Spat2Json()
                
                outerSocket.sendto(spatJsonString.encode(), msgEncoderAddress)
                outerSocket.sendto(spatJsonString.encode(), dataCollectorAddress)
                print(spatJsonString)
                print("Sent SPAT to MsgEncoder")
                spatMapMsgCount = spatMapMsgCount + 1
                if spatMapMsgCount > 9:
                    outerSocket.sendto(mapPayload.encode(), msgSenderAddress)
                    spatMapMsgCount = 0
                    print("Sent MAP to MsgSender")
                #print("Sent SPaT JSON to msgEncoder and trafficControllerObserver, and MAP payload to msgSender.")
        except socket.timeout:
            print("No packets received from the Traffic Signal Controller. Check:\n1. Physical connection between CVCP and Traffic Signal Controller.\n2. Server IP in MM-1-5-1 of the Signal Controller must match the IP address of CVCP.\n3. Address in MM-1-5-3 must be set to 6053.\n4. Controller must be power-cycled after changes in internal configuration.\n")

if __name__ == "__main__":
    main()
    

