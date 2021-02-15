import socket
import json

configFile = open("./../message_distributor_config.json", 'r')
config = json.load(configFile)
configFile.close()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((config["bsm_additional_clients"]["Transit"][0]["ip_address"],config["bsm_additional_clients"]["Transit"][0]["port"]))

while True: 
    data, addr = s.recvfrom(1024)
    print(data.decode())

s.close()
