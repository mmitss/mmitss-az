import socket
import json

configFile = open("./../message_distributor_config.json", 'r')
config = json.load(configFile)
configFile.close()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((config["intersections"][0]["ip_address"],config["intersections"][0]["bsm_client_port"]))

while True: 
    data, addr = s.recvfrom(1024)
    print(data.decode())

s.close()
