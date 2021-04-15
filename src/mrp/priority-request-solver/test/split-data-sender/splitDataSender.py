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
splitDataSenderSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
splitDataSenderSocket.bind((hostIp, port))

prirorityRequestSolverPort = config["PortNumber"]["PrioritySolver"]
communicationInfo = (hostIp, prirorityRequestSolverPort)


def getJsonString(fileName):
    f = open(fileName, 'r')
    data = f.read()
    f.close()

    return data


data = getJsonString(fileName)
splitDataSenderSocket.sendto(data.encode(), communicationInfo)
print("Send split data at time", time.time())

while(True):
    # Receive data on the splitDataSender socket
    data, address = splitDataSenderSocket.recvfrom(10240)
    data = data.decode()
    # Load the received data into a json object
    receivedMessage = json.loads(data)

    if receivedMessage["MsgType"] == "CoordinationPlanRequest":
        data = getJsonString(fileName)
        splitDataSenderSocket.sendto(data.encode(), communicationInfo)
        print("Send split data at time", time.time())

splitDataSenderSocket.close()
