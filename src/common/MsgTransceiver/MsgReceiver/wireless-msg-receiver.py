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

def getMsgPayload(rawMsg:str, psidDict:dict, msgIdDict:dict):
    psidBegin = rawMsg[52:]
    if psidBegin[:2] == psidDict["bsm"]:
        extractedPayload = psidBegin[psidBegin.find(msgIdDict["bsm"]):][:-66]
    elif psidBegin[:8] == psidDict["map"]:
        extractedPayload = psidBegin[psidBegin.find(msgIdDict["map"]):][:-66]
    elif psidBegin[:4] == psidDict["spat"]:
        extractedPayload = psidBegin[psidBegin.find(msgIdDict["spat"]):][:-66]
    elif psidBegin[:8] == psidDict["ssm"]:
        extractedPayload = psidBegin[psidBegin.find(msgIdDict["ssm"]):][:-66]
    elif psidBegin[:8] == psidDict["srm"]:
        extractedPayload = psidBegin[psidBegin.find(msgIdDict["srm"]):][:-66]
    elif psidBegin[:4] == psidDict["rsm"]:
        extractedPayload = psidBegin[psidBegin.find(msgIdDict["rsm"]):][:-66]
    return extractedPayload   


def main():
    DEBUGGING = True

    # Open configuration file and load the data into JSON object
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r').read()
    config = (json.loads(configFile))
    if DEBUGGING: print("Configuration file read successfully.")

    # From config Json object, get the hostIp and Port for this application.
    hostIp = config["HostIp"]
    msgReceiverPort = config["PortNumber"]["MessageTransceiver"]['MessageReceiver']
    hostComm = (hostIp, msgReceiverPort)

    transceiverDecoderPort = config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]
    transceiverDecoderComm = (hostIp, transceiverDecoderPort)

    rsmDecoderPort = config["PortNumber"]["RsmDecoder"]
    rsmDecoderComm = (hostIp, rsmDecoderPort)

    psidDict = config["psid"]
    msgIdDict = config["msgId"]

    # Create a socket and bind it to the information extracted from the config.
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(hostComm)
    
    if DEBUGGING: print('''HostIp = {}\nwireless-receiverPort = {}'''.format(hostIp, msgReceiverPort))
    
    #setup logging to a file for test output
    import datetime

    if DEBUGGING:
        firstIteration = True

    while True:
        if DEBUGGING and firstIteration == True:
            if s.recvfrom(4096):
                print("Started receiving wireless messages.")
                firstIteration = False

        # Receive a binary message packet and convert it to hex packet.
        receivedMsg, addr = s.recvfrom(4096)
        receivedMsg = receivedMsg.hex()
        msgPayload = getMsgPayload(receivedMsg, psidDict, msgIdDict)
        if msgPayload[:4]=="0021": 
            s.sendto(msgPayload.encode(), rsmDecoderComm)
            if DEBUGGING: print("Received RSM from OBU")
        else: 
            s.sendto(msgPayload.encode(), transceiverDecoderComm)
            if DEBUGGING: 
                if msgPayload[:4]=="0014": print("Received BSM from RSU")
                elif msgPayload[:4]=="0012": print("Received MAP from OBU")
                elif msgPayload[:4]=="0013": print("Received SPAT from OBU")
                elif msgPayload[:4]=="001d": print("Received SRM from RSU")
                elif msgPayload[:4]=="001e": print("Received SSM from OBU")
                else: print ("Received invalid message.")
        
    s.close()
if __name__ == "__main__":
    main()
