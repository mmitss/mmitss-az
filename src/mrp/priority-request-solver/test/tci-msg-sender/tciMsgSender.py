import socket
import json
import datetime
import time

currentStatusFileName = "currPhase.json"
currentSignalPlanFileName = "signalPlan.json"

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
port = config["PortNumber"]["TrafficControllerInterface"]
tciSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
tciSocket.bind((hostIp, port))

priorityRequestSolverPort = config["PortNumber"]["PrioritySolver"]
priorityRequestSolverCommunicationInfo = (hostIp, priorityRequestSolverPort)

priorityRequestSolverToTCIInterfacePort = config["PortNumber"]["PrioritySolverToTCIInterface"]
priorityRequestSolverToTCIInterfaceCommunicationInfo = (
    hostIp, priorityRequestSolverToTCIInterfacePort)


def getJsonString(fileName):

    f = open(fileName, 'r')
    data = f.read()
    f.close()

    return data


data = getJsonString(currentSignalPlanFileName)
tciSocket.sendto(data.encode(), priorityRequestSolverCommunicationInfo)
print("Sent Signal Plan to Solver at time: ", time.time())

while(True):
    # Receive data on the TCI socket
    data, address = tciSocket.recvfrom(10240)
    data = data.decode()
    # Load the received data into a json object
    receivedMessage = json.loads(data)

    if receivedMessage["MsgType"] == "Schedule":
        if receivedMessage["Schedule"] == "Clear":
            print("Received a clear request at time: ", time.time())
        else:
            print("Received a new schedule at time: ", time.time())

    elif receivedMessage["MsgType"] == "CurrNextPhaseRequest":
        # print("Received CurrNextPhaseRequest at time: ", time.time())
        data = getJsonString(currentStatusFileName)
        tciSocket.sendto(
            data.encode(), priorityRequestSolverToTCIInterfaceCommunicationInfo)
        print("Sent Current Phase Status to Solver at time: ", time.time())

    elif receivedMessage["MsgType"] == "TimingPlanRequest":
        # print("Received TimingPlanRequest at time ", time.time())
        data = getJsonString(currentSignalPlanFileName)
        tciSocket.sendto(data.encode(), priorityRequestSolverCommunicationInfo)
        print("Sent Signal Plan to Solver at time: ", time.time())


tciSocket.close()