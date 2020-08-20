import socket
import json
import datetime
import time

fileName = "intersectionLogData.json"

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
port = config["PortNumber"]["PriorityRequestGenerator"]
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp,port))

dataCollectorPort = config["PortNumber"]["SystemPerformanceDataCollector"]
communicationInfo = (hostIp, dataCollectorPort)

f = open(fileName, 'r')
data = f.read()
s.sendto(data.encode(),communicationInfo)
print (time.time())

# data,address = s.recvfrom(10240)
# print(data.decode())


f.close()
s.close()