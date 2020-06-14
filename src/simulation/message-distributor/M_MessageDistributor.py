import json
import socket
import sys
from MessageDistributor import MessageDistributor

configFile = open(sys.argv[1], 'r')
config = json.load(configFile)
configFile.close()

msgDist = MessageDistributor(config)

receivingSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
receivingSocket.bind((config["msg_distributor_ip"], config["msg_distributor_port"]))

while True:
    data, addr = receivingSocket.recvfrom(1024)
    msg = json.loads(data.decode())
    msg = msgDist.timestampMessage(msg)
    messageType = msgDist.distributeMsgToInfrastructureAndGetType(msg)
    if messageType == "BSM":
        msgDist.distributeBsmToClients(msg)

receivingSocket.close()