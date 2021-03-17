import socket
import json
import datetime
import time

fileName = "priorityRequest.json"

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
port = config["PortNumber"]["PriorityRequestServer"]
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp, port))

priorityRequestSolverPort = config["PortNumber"]["PrioritySolver"]
communicationInfo = (hostIp, priorityRequestSolverPort)

f = open(fileName, 'r')
data = f.read()
s.sendto(data.encode(), communicationInfo)
print("sent Priority Request list at time", time.time())

f.close()
s.close()