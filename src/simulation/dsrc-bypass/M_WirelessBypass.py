'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  M_WirelessBypass.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Description:
  In the simulation environment of MMITSS, it is often infeasible to route messages 
  through actual wireless devices such as RSU or OBU. M_WirelessBypass application 
  serves as a bypass that takes in the message created by the MessageEncoder

'''
import socket
import binascii
import json

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