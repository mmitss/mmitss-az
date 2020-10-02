'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  v2x-data-collector.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
'''

import socket
import json
import time, datetime
import atexit
from V2XDataCollector import V2XDataCollector

DEBUGGING = False

configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

environment = config["ApplicationPlatform"]

host = (config["HostIp"],config["PortNumber"]["DataCollector"])

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(host)

v2xDc = V2XDataCollector(environment)
atexit.register(v2xDc.archive_current_directory)

if DEBUGGING: logFileCreationDay = datetime.datetime.now().minute
else: logFileCreationDay = datetime.datetime.now().day

while True:
    if DEBUGGING: currentDay = datetime.datetime.now().minute
    else: currentDay = datetime.datetime.now().day
    
    data, addr = s.recvfrom(20480)
    senderPort = addr[1]

    if ((currentDay != logFileCreationDay)) :
        v2xDc.close_logfiles()
        v2xDc.initialize_logfiles()   
        v2xDc.decode_and_store_data(data, senderPort)  
        logFileCreationDay = currentDay      
    else:
        v2xDc.decode_and_store_data(data, senderPort)

s.close()
v2xDc.close_logfiles()





