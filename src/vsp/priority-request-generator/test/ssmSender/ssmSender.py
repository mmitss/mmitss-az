import socket
import json
import datetime
import time

fileName = "ssm.json"

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
port = config["PortNumber"]["MessageDistributor"]
ssmSendingSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ssmSendingSocket.bind((hostIp,port))

priorityRequestGeneratorPort = config["PortNumber"]["PriorityRequestGenerator"]
communicationInfo = (hostIp, priorityRequestGeneratorPort)
ssmSendingTime = 0.0

while True:
    if time.time()-ssmSendingTime >=1.0:
        f = open(fileName, 'r')
        data = f.read()
        ssmSendingSocket.sendto(data.encode(),communicationInfo)
        ssmSendingTime = time.time()
        print("sent ssm at time", time.time())

f.close()
ssmSendingSocket.close()