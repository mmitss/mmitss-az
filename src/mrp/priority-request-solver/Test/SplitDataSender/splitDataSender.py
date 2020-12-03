import socket
import json
import datetime
import time

fileName = "SplitData.json"

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
port = config["PortNumber"]["SignalCoordination"]
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp,port))

prirorityRequestSolverPort = config["PortNumber"]["PrioritySolver"]
communicationInfo = (hostIp, prirorityRequestSolverPort)

f = open(fileName, 'r')
data = f.read()
print(data)
s.sendto(data.encode(),communicationInfo)
# print (time.time())
# print(data.encode())


f.close()
s.close()