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

from src.common.Position3D import Position3D
from src.common.BasicVehicle import BasicVehicle


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

f = open('/User/Larry/Desktop/dev/mmitss-private/mmitss/src/obu/controllerSimulatorforHMI/HMIControllerSimulatorData.1.csv', 'r')

f.readline()
while (f.readline()):
    line = f.readline()
    line = line.replace('\n','')
    data_array = line.split(',')
    secMark = int(data_array[0])
    #this is all vehicle information
    print("second = ", secMark)
    tempID = int(data_array[1])
    latitude_DecimalDegree= float(data_array[2])
    longitude_DecimalDegree= float(data_array[3])
    elevation_Meter= float(data_array[4])
    heading_Degree= float(data_array[5])
    speed_Meterpersecond= float(data_array[6])
    speed_mph= round((float(speed_Meterpersecond) * 2.23694),2)
    #vehicleType= "Truck"
    #vehicleLane= int(data_array[6])

    # Create a Basic Vehicle that represents the host vehicle
    positon = Position3D(latitude_DecimalDegree, longitude_DecimalDegree, elevation_Meter)
    hostVehicle = BasicVehicle(tempID, secMark, position, speed_mph, heading_Degree)

    #this is all reduced speed zone information
    numOtherVehicles = int(data_array[7])
    
    statusOfLanes = []
    for lane in range(0, int(numLanes)):
        laneStatus = LaneStatus(lane)
        
        laneStatus.lane = int(data_array[7 + lane*8 + 1])
        
        laneStatus.laneSpeedLimit_mph= float(data_array[7 + lane*8 + 2])
        laneStatus.laneSpeedChangeDistance = float(data_array[7 + lane*8 + 3])
        
        laneStatus.workersPresent = bool_map[data_array[7 + lane*8 + 4]]

        laneStatus.laneClosed = bool_map[data_array[7 + lane*8 + 5]]
        laneStatus.laneClosedDistance = float(data_array[7 + lane*8 + 6])
        laneStatus.taperRight = bool_map[data_array[7 + lane*8 + 7]]
        laneStatus.taperLeft = bool_map[data_array[7 + lane*8 + 8]]

        statusOfLanes.append(laneStatus.laneStatus2dict())

    activeEventID = 'activeEventID'
    activeDescriptiveName = 'activeDescriptiveName'

    eventID1 = 'eventID1'
    eventID2 = 'eventID2'
    eventID3 = 'eventID3'
    eventID4 = 'eventID4'

    descriptiveName1 = "descriptiveName1"
    descriptiveName2 = "descriptiveName2"
    descriptiveName3 = "descriptiveName3"
    descriptiveName4 = "descriptiveName4"

    activeMap = dict({"eventID": activeEventID, "descriptiveName": activeDescriptiveName})
    availableMaps = dict({"availableMaps":[                        
                            {"eventID": eventID1, "descriptiveName": descriptiveName1},
                            {"eventID": eventID2, "descriptiveName": descriptiveName2},
                            {"eventID": eventID3, "descriptiveName": descriptiveName3},
                            {"eventID": eventID4, "descriptiveName": descriptiveName4}
                        ]})

    interfaceJsonString = json.dumps({
        "controller_hmi_interface":
        {
            "BasicVehicle" :
            {
                "heading_Degree" : heading_Degree,
                "position" :
                {
                    "elevation_Meter" : elevation_Meter,
                    "latitude_DecimalDegree" : latitude_DecimalDegree,
                    "longitude_DecimalDegree" : longitude_DecimalDegree
                },
                "secMark_Second" : secMark,
                "speed_MeterPerSecond" : speed_Meterpersecond,
                "temporaryID" : 1,
                "vehicleType" : 100,
                "lane": vehicleLane,
                "speed_mph": speed_mph
            },
            "mapCache":
            {
                "activeMap": activeMap,
                "availableMaps": availableMaps
            },
            "lanes":
                statusOfLanes #this is a list of lane dictionaries
        }
    })
    s.sendto(interfaceJsonString.encode(),hmi)
    time.sleep(1)
s.close() 