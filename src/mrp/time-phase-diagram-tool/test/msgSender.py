import socket
import json
import time


# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
port = config["PortNumber"]["PrioritySolver"]
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp,port))

timePhaseDiagramToolPort = config["PortNumber"]["TimePhaseDiagramTool"]
communicationInfo = (hostIp, timePhaseDiagramToolPort)


data = json.dumps({"MsgType": "TimePhaseDiagram", "OptimalSolutionStatus": True})
s.sendto(data.encode(),communicationInfo)
print (time.time())
print(data)

s.close()