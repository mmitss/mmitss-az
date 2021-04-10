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
TESTING = False
import json
import socket
import sys
from MessageDistributor import MessageDistributor
import datetime

if TESTING: 
    configFile = open("../../../config/simulation-tools/nojournal/bin/mmitss-phase3-master-config.json", 'r')    
else:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
masterConfig = json.load(configFile)
configFile.close()

if TESTING:
    configFile = open("../../../config/simulation-tools/nojournal/bin/mmitss-message-distributor-config.json", 'r')
else: 
    configFile = open("/nojournal/bin/mmitss-message-distributor-config.json", 'r')
config = json.load(configFile)
configFile.close()

msgDist = MessageDistributor(config)

receivingSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
if TESTING:
    receivingSocket.bind(("127.0.0.1", masterConfig["PortNumber"]["MessageDistributor"]))
else:
    receivingSocket.bind((masterConfig["MessageDistributorIP"], masterConfig["PortNumber"]["MessageDistributor"]))

rawBsmLogging = config["raw_bsm_logging"]
if rawBsmLogging == True:
    logfile = open(("rawBsmLog_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"), 'w')
    logfile.write("timestamp,secMark,temporaryId,latitude,longitude,elevation,speed,heading,type,length,width\n")

while True:
    data, addr = receivingSocket.recvfrom(40960)
    msg = json.loads(data.decode())
    msg = msgDist.timestampMessage(msg)
    messageType = msgDist.distributeMsgToInfrastructureAndGetType(msg)
    if messageType == "BSM":
        msgDist.distributeBsmToClients(msg)
        if rawBsmLogging == True:
            logfile.write(str(msg["Timestamp_posix"]) + "," +
            str(msg["BasicVehicle"]["secMark_Second"]) + "," +
            str(msg["BasicVehicle"]["temporaryID"]) + "," +
            str(msg["BasicVehicle"]["position"]["latitude_DecimalDegree"]) + "," +
            str(msg["BasicVehicle"]["position"]["longitude_DecimalDegree"]) + "," +
            str(msg["BasicVehicle"]["position"]["elevation_Meter"]) + "," +
            str(msg["BasicVehicle"]["speed_MeterPerSecond"]) + "," +
            str(msg["BasicVehicle"]["heading_Degree"]) + "," +
            str(msg["BasicVehicle"]["type"]) + "," +
            str(msg["BasicVehicle"]["size"]["length_cm"]) + "," +
            str(msg["BasicVehicle"]["size"]["width_cm"]) + "\n"
            )
    elif messageType == "MAP":
        msgDist.distributeMapToClients(msg)
    elif messageType == "SSM":
        msgDist.distributeSsmToClients(msg)

if rawBsmLogging == True:
    logfile.close()
receivingSocket.close()