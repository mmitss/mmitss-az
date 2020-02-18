import socket
import time
import json

hostIp = '10.12.6.108'
port = 20001
addr = (hostIp, port)
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(addr)

f = open("msg.json", 'r')
msg = f.read()
print("leadTime,snmp")

#while True:
s.sendto(msg.encode(), ("10.12.6.108", 20005))
requestTime = time.time()

data, addr = s.recvfrom(512)
deliveryTime = time.time()

leadTime = deliveryTime - requestTime

dataJson = json.loads(data.decode())
if dataJson["nextPhases"] == [0]:
    snmp = False
else: snmp = True

print(str(round(leadTime,5)) + "," + str(snmp))
    




    #time.sleep(5)

