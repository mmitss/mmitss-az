import socket
import json
import time, datetime
from V2XDataCollector import V2XDataCollector

DEBUGGING = True

configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

environment = config["ApplicationPlatform"]

host = (config["HostIp"],config["PortNumber"]["DataCollector"])

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(host)

v2xDc = V2XDataCollector(environment)

if DEBUGGING: logFileCreationDay = datetime.datetime.now().minute
else: logFileCreationDay = datetime.datetime.now().day

while True:
    if DEBUGGING: currentDay = datetime.datetime.now().minute
    else: currentDay = datetime.datetime.now().day
    
    data, addr = s.recvfrom(20480)
    senderPort = addr[1]

    if currentDay == logFileCreationDay:
        v2xDc.decode_and_store_data(data, senderPort)
    else:
        currentDay = logFileCreationDay
        v2xDc.close_logfiles()
        v2xDc.initialize_logfiles()        
        v2xDc.decode_and_store_data(data, senderPort)

s.close()
v2xDc.close_logfiles()





