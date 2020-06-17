import socket
import json

fileName = "snmpGetRequest.json"

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
msgSenderReceiverPort = 5001
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp,msgSenderReceiverPort))

snmpEnginePort = config["PortNumber"]["SnmpEngine"]
snmpEngine_commInfo = (hostIp, snmpEnginePort)

f = open(fileName, 'r')
data = f.read()
s.sendto(data.encode(),snmpEngine_commInfo)

data,address = s.recvfrom(2048)
print(data.decode())


f.close()
s.close()