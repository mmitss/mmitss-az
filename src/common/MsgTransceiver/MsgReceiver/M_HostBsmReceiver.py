#!/usr/bin/env python3
'''
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  bsm-receiver.py
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  
  Operational Description:
  This application does the following tasks:
  1. Reads the hostIp and port of this application from a configuration file.
  2. Creates a socket, and binds it to the information retrieved from the configuration file.
  3. Receives the BSMs over the socket.
  4. Extracts the BSM payload and forwards the payload to TransceiverDecoder.
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
    bsmReceiverPort = config["PortNumber"]['OBUBSMReceiver']
    hostComm = (hostIp, bsmReceiverPort)

    cvwz_bsmDecoderPort = 10008
    cvwz_bsmDecoderComm = (hostIp, cvwz_bsmDecoderPort)

    transceiverDecoderPort = config["PortNumber"]["HostBsmDecoder"]
    transceiverDecoderComm = (hostIp, transceiverDecoderPort)

    # Create a socket and bind it to the information extracted from the config.
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(hostComm)
    
    if DEBUGGING: print('''HostIp = {}\bsmCollectorPort = {}'''.format(hostIp, bsmReceiverPort))
    
    #setup logging to a file for test output
    import datetime

    # Create a timestamp which will be appended at the end of name of a file storing received data.
    timestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))
    # Create complete file name: example: msgLog_<timestamp>
    fileName = "HostBsmLog_" + timestamp + ".txt"
    dataLog = open(fileName, 'w')

    if DEBUGGING:
        firstIteration = True

    while True:
        if DEBUGGING and firstIteration == True:
            if s.recvfrom(4096):
                print("Started receiving basic safety messages.")
                firstIteration = False

        # Receive a binary message packet and convert it to hex packet.
        receivedMsg, addr = s.recvfrom(4096)
        receivedMsg = receivedMsg.hex()
        if DEBUGGING: dataLog.write(receivedMsg + '\n')
        bsmPayload = receivedMsg[receivedMsg.find("0014"):]
        s.sendto(bsmPayload.encode(), transceiverDecoderComm)
        s.sendto(bsmPayload.encode(), cvwz_bsmDecoderComm)
        
        if DEBUGGING: print("Received BSM from OBU")
    s.close()
    dataLog.close()
if __name__ == "__main__":
    main()
