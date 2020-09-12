import socket
import json

# Read a config file into a json object:
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
configFile.close()

hostIp = config["HostIp"]
priorityRequestGeneratorPort = config["PortNumber"]["PriorityRequestGenerator"]
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp,priorityRequestGeneratorPort))

while True:
    data,address = s.recvfrom(4096)
    # print(data.decode())
    # Load the received data into a json object
    receivedMsg = json.loads(data)
    print(receivedMsg)
s.close()