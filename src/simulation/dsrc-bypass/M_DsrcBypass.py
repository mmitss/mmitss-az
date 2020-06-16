import socket
import binascii
import json

# Read a config file by creating an object of the time MapSpatBroadcasterConfig
configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((config["SourceDsrcDeviceIp"], 1516))

wireless_decoder = ((config["HostIp"],config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]))

while True:
    data, addr = s.recvfrom(1024)
    data = binascii.unhexlify(data)
    s.sendto(data, wireless_decoder)
s.close()