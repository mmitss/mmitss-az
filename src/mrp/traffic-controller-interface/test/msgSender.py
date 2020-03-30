import socket
import json

fileName = "test/schedule1.json"

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
solverPort = config["PortNumber"]["PrioritySOlver"]
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp,solverPort))

tciPort = config["PortNumber"]["TrafficControllerCommander"]
tci_commInfo = (hostIp, tciPort)

f = open(fileName, 'r')
data = f.read()
s.sendto(data.encode(),tci_commInfo)

f.close()
s.close()
