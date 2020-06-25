'''
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  M_MessageDistributor.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

***************************************************************************************
'''
import json
import socket
import sys
from MessageDistributor import MessageDistributor

configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
masterConfig = json.load(configFile)
configFile.close()

configFile = open("/nojournal/bin/mmitss-message-distributor-config.json", 'r')
config = json.load(configFile)
configFile.close()

msgDist = MessageDistributor(config)

receivingSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
receivingSocket.bind((masterConfig["MessageDistributorIP"], masterConfig["PortNumber"]["MessageDistributor"]))

while True:
    data, addr = receivingSocket.recvfrom(4096)
    msg = json.loads(data.decode())
    msg = msgDist.timestampMessage(msg)
    messageType = msgDist.distributeMsgToInfrastructureAndGetType(msg)
    if messageType == "BSM":
        msgDist.distributeBsmToClients(msg)
    elif messageType == "MAP":
        msgDist.distributeMapToClients(msg)
    elif messageType == "SSM":
        msgDist.distributeSsmToClients(msg)

receivingSocket.close()