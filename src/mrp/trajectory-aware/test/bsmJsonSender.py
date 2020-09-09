''''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    bsmJsonSender.py
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:

'''

import time, datetime
import json
import socket

configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
config = (json.load(configFile))
hostIp = config["HostIp"]
decoderPort = config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]+50

trajectoryAwarePort = config["PortNumber"]["TrajectoryAware"]
trajectoryAware_commInfo = (hostIp, trajectoryAwarePort)

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind((hostIp,decoderPort))
print("Started sending BSM-JSON strings to TrajectoryAware")

csvFile = open("BsmLogDaisyMountain_GavilanPeak_02262020_161213.csv", "r")
line = csvFile.readline()
while line:
    line = csvFile.readline()
    if line=="":
        break
    data = line.split(",")
    bv_json =   json.dumps({
                        "MsgType": "BSM",
                        "Timestamp_verbose":data[2],
                        "Timestamp_posix":data[3],
                        "BasicVehicle": {  
                                            "temporaryID": data[4], 
                                            "secMark_Second": data[5], 
                                            "speed_MeterPerSecond": data[9], 
                                            "heading_Degree": data[10], 
                                            "type": "CAR",
                                            "position": 
                                            { 
                                                "latitude_DecimalDegree": data[6], 
                                                "longitude_DecimalDegree": data[7], 
                                                "elevation_Meter": data[8]
                                            },
                                            "size":
                                            {
                                                "length_cm": data[11],
                                                "width_cm": data[12]
                                            }
                                    }
                    })
    s.sendto(bv_json.encode(),trajectoryAware_commInfo)
    time.sleep(0.1)

csvFile.close()
s.close()
