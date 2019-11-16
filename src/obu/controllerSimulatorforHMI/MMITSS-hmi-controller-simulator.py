'''
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  MMITSS-hmi-controller-simulator.py
  Created by: Larry Head
  University of Arizona   
  College of Engineering

  
  Operational Description:

  This application does the following tasks:
  1. Reads data from a .csv file 
  2. Builds json representing the HMI states to send to the HMI
  3. Sends the json to the HMI
'''

import socket
import json
import time
import os

from Position3D import Position3D
from BasicVehicle import BasicVehicle


controllerIP = '127.0.0.1'
controllerPort = 5001
controller = (controllerIP, controllerPort)

hmiIP = '127.0.0.1'
hmiPort = 5002
hmi = (hmiIP, hmiPort)

bool_map = {"TRUE": True, "True": True, "FALSE": False, "False": False} # this could be come the SPaT phaseStatus data map

# Create a socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Bind the created socket to the server information.
s.bind((controller))

directory_path = os.getcwd()
f = open(directory_path + '/src/obu/controllerSimulatorforHMI/HMIControllerSimulatorData.1.csv', 'r')

f.readline()
f.readline() 
f.readline() # there are three informational lines at the top of the data file (top is category, second is data_array cound, third is data lable)
while (f.readline()):
    line = f.readline()
    line = line.replace('\n','')
    data_array = line.split(',')
    secMark = int(data_array[0])
    #this is all vehicle information
    print("second = ", secMark)
    hv_tempID = int(data_array[1])
    hv_vehicleType = data_array[2]
    hv_latitude_DecimalDegree= float(data_array[32])
    hv_longitude_DecimalDegree= float(data_array[4])
    hv_elevation_Meter= float(data_array[5])
    hv_heading_Degree= float(data_array[6])
    hv_speed_Meterpersecond= float(data_array[7])
    hv_speed_mph= round((float(hv_speed_Meterpersecond) * 2.23694),2)
   
    # Create a Basic Vehicle that represents the host vehicle
    hv_position = Position3D(hv_latitude_DecimalDegree, hv_longitude_DecimalDegree, hv_elevation_Meter)
    hostVehicle = BasicVehicle(hv_tempID, secMark, hv_position, hv_speed_mph, hv_heading_Degree, 'transit')

    #this is all remote vehicle information
    numRemoteVehicles = int(data_array[8])
    
    remoteVehicles = []
    for vehicle in range(0, 5): # assuming up to 5 remote vehicles for now
        rv_tempID = data_array[9 + vehicle*7]
        rv_vehicleType = data_array[10 + vehicle*7]
        rv_latitude_DecimalDegree= float(data_array[11 + vehicle*7])
        rv_longitude_DecimalDegree= float(data_array[12 + vehicle*7])
        rv_elevation_Meter= float(data_array[13 + vehicle*7])
        rv_heading_Degree= float(data_array[14 + vehicle*7])
        rv_speed_Meterpersecond= float(data_array[15 + vehicle*7])
        rv_speed_mph= round((float(rv_speed_Meterpersecond) * 2.23694),2)

        rv_position = Position3D(rv_latitude_DecimalDegree, rv_longitude_DecimalDegree, rv_elevation_Meter)
        remoteVehicle = BasicVehicle(rv_tempID, secMark, rv_position, rv_speed_mph, rv_heading_Degree, rv_vehicleType)
        
        if vehicle < numRemoteVehicles:
             bv_dict = remoteVehicle.BasicVehicle2Dict()
             remoteVehicles.append(bv_dict)


 # infrastructure map data
    availableMaps = dict({"availableMaps":[                        
                            {"IntersectionID": 101, "DescriptiveName": "Daisy Mountain and Gavilan Peak", "active": False, "age" : 100},
                            {"IntersectionID": 102, "DescriptiveName": "Daisy Mountain and Dedication", "active": False, "age" : 200},
                            {"IntersectionID": 103, "DescriptiveName": "Gavilan Peak and Boulder Creek High School", "active": True, "age" : 0},
                            {"IntersectionID": 104, "DescriptiveName": "Gavilan Peak and Memorial Way", "active": False, "age" : 333},
                            {"IntersectionID": 105, "DescriptiveName": "Daisy Mountain and Hastings", "active": False, "age": 270 }
                        ]})

    interfaceJsonString = json.dumps({
        "mmitss_hmi_interface":
        {
            "hostVehicle" :
            {
                "heading_Degree" : hv_heading_Degree,
                "position" :
                {
                    "elevation_Meter" : hv_elevation_Meter,
                    "latitude_DecimalDegree" : hv_latitude_DecimalDegree,
                    "longitude_DecimalDegree" : hv_longitude_DecimalDegree
                },
                "secMark_Second" : secMark,
                "speed_MeterPerSecond" : hv_speed_Meterpersecond,
                "temporaryID" : hv_tempID,
                "vehicleType" : hv_vehicleType,
                "lane": 1, # this is desireable data
                "speed_mph": hv_speed_mph
            },
            "remoteVehicles" :
                remoteVehicles,
            
            "infrastructure": 
            {
                 "mapCache":
            {
                "availableMaps": availableMaps
            },
            }
            
        }
    })
    s.sendto(interfaceJsonString.encode(),hmi)
    time.sleep(1)
s.close() 