'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    msgSenderReceiver.py
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:

'''
import socket
import json
import time

fileName = "./../sampleJsons/LocateVehicleOnMapRequest.json"

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
solverPort = config["PortNumber"]["TrajectoryAware"]+50
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp,solverPort))

mapEnginePort = config["PortNumber"]["MapEngine"]
mapEngine_commInfo = (hostIp, mapEnginePort)

f = open(fileName, 'r')
data = f.read()
#while True:
s.sendto(data.encode(),mapEngine_commInfo)
requestTime = time.time()
receivedData,address = s.recvfrom(2048)
receivedTime = time.time()
print((receivedTime - requestTime))
#time.sleep(0.05)
print(receivedData.decode())

f.close()
s.close()
