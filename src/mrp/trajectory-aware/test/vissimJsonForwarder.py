import json
import socket
import time

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("10.12.6.3",5051))
#f = open("Points.csv","w")
#f.write("timestamp,latitude,longitude\n")

while True:
    receivedData, address = s.recvfrom(2048)
    bsmJson = json.loads(receivedData.decode())
    bsmJson["Timestamp_posix"] = time.time()
    #bsmJson["BasicVehicle"]["size"]={"length_cm":3765, "width_cm":269}
    sendingData = json.dumps(bsmJson)
    #print("BSM Received at:" + str(time.time()))
    s.sendto(sendingData.encode(), ("10.12.6.3",20001))
    #f.write(str(bsmJson["Timestamp_posix"]) + "," + str(bsmJson["BasicVehicle"]["position"]["latitude_DecimalDegree"]) + "," + str(bsmJson["BasicVehicle"]["position"]["longitude_DecimalDegree"])+"\n")
#f.close()
