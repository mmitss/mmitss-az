'''
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  MMITSS-host-prg-simulator.py
  Created by: Larry Head
  University of Arizona   
  College of Engineering

  
  Operational Description:

  This application does the following tasks:
  1. Reads host vehicle and prg data from a .csv file 
  2. Builds json representing the structure sent from prg to mmitss-hmi-controller
  3. Sends the json to the mmitss-hmi-controller
'''

import socket
import json
import time
import os

from Position3D import Position3D
from BasicVehicle import BasicVehicle


hmi_controllerIP = '127.0.0.1'
hmi_controllerPort = 20009
hmi_controller = (hmi_controllerIP, hmi_controllerPort)

prg_simIP = '127.0.0.1'
prg_simPort = 20004
prg_sim = (prg_simIP, prg_simPort)

bool_map = {"TRUE": True, "True": True, "FALSE": False, "False": False, "false" : False, "true" : True} # this could be come the SPaT phaseStatus data map
spat_state = {0 : "unknown", # based on the MOvementPhaseState from the SAE J2735 2016 standard - not comment in MovementPhaseState is that these are not used with UPER encoding (???)
              1 : "dark", 
              2 : "stop-Then-Proceed", # flashing red (flashing Red ball)
              3 : "stop-And-Remain", # red light (Red ball) [Don't walk]
              4 : "pre-Movement", # not used in US
              5 : "permissive-Movement-Allowed", # permissive green (Green ball)
              6 : "protected-Movement-Allowed",  # protected green (e.g. left turn arrow) - Green Arrow (direction?) [also walk]
              7 : "permissive-clearance", # permissive yellow (clear intersection) - Yellow 
              8 : "protected-clearance", # protected yellow (clear intersection) - Yellow arrow  [ also ped clear= Flashing Don;t Walk]
              9 : "caution-Conflicting-Traffic", # flashing yellow (yield)
              } 
spat_signal_head = {"stop-And-Remain" : "red", "stop-Then-Proceed" : "red_flash", "protected-Movement-Allowed" : "green", "permissive-clearance" : "yellow", "protected-clearance" : "yellow",  "dark" : "dark"}


priority_responseStatus = {0 : "unknown", 
                           1 : "requested",
                           2 : "processing",
                           3 : "watchOtherTraffic",
                           4 : "granted",
                           5 : "rejected",
                           6 : "maxPresence",
                           7 : "reserviceLocked"}

basicVehicleRoles = {0 : "basicVehicle",
                    9 : "truck",
                    13 : "ev-fire",
                    16 : "transit"}

# Create a socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Bind the created socket to the server information.
s.bind((prg_sim))

directory_path = os.getcwd()
#f = open(directory_path + '/src/obu/controllerSimulatorforHMI/prg-simulator-data.csv', 'r')
f = open('HMIControllerSimulatorData.2.csv', 'r')

f.readline() 
f.readline() 
f.readline() # there are three informational lines at the top of the data file (top is category, second is data_array cound, third is data lable)
for line in f :
    line = line.replace('\n','')
    data_array = line.split(',')
    secMark = int(data_array[0])
    #this is host vehicle information
    print("second = ", secMark)
    hv_tempID = int(data_array[1])
    hv_vehicleType = data_array[2]
    hv_latitude_DecimalDegree= round(float(data_array[3]), 8)
    hv_longitude_DecimalDegree= round(float(data_array[4]), 8)
    hv_elevation_Meter= round(float(data_array[5]), 1)
    hv_heading_Degree= round(float(data_array[6]), 4)
    hv_speed_Meterpersecond= float(data_array[7])
    hv_speed_mph= int((float(hv_speed_Meterpersecond) * 2.23694))
   
    # Create a Basic Vehicle that represents the host vehicle
    hv_position = Position3D(hv_latitude_DecimalDegree, hv_longitude_DecimalDegree, hv_elevation_Meter)
    hostVehicle = BasicVehicle(hv_tempID, secMark, hv_position, hv_speed_mph, hv_heading_Degree, hv_vehicleType)

    #need to acquire current lane and current lane signal group
    hv_currentLane = int(data_array[8])
    hv_currentLaneSignalGroup = int(data_array[9])

 # infrastructure map data
    index_maps = 46
    numReceivedMaps = int(data_array[index_maps])
    availableMaps = []
    if availableMaps == 0 :
        availableMaps == None
    else :
        for receivedMap in range(0, 5): # assuming up to 5 maps have been received 
            map_intersectionID = int(data_array[index_maps + 1 + receivedMap*4])
            map_DescriptiveName = data_array[index_maps + 2 + receivedMap*4]
            map_active = str(bool_map[data_array[index_maps + 3 + receivedMap*4]])
            map_age = int(data_array[index_maps + 4 + receivedMap*4])
            if receivedMap < numReceivedMaps:
                availableMaps.append({"IntersectionID": map_intersectionID, "DescriptiveName": map_DescriptiveName, "active": map_active, "age" : map_age})                        

 
    #acquire priority status data
    index_priority = 137 # index is the column in the csv file
    activeRequestTable = []
    onMAP = data_array[index_priority]
    requestSent = data_array[index_priority + 1]
    numActiveRequests = int(data_array[index_priority + 2])
    if numActiveRequests == 0 :
        activeRequestTable = None
    else :
        for request in range(0, numActiveRequests): 
            vehicleID = int(data_array[index_priority + 3 + request*8])
            requestID = int(data_array[index_priority + 4 + request*8])
            msgCount = int(data_array[index_priority + 5 + request*8])
            inBoundLaneID = int(data_array[index_priority + 6 + request*8])
            basicVehicleRole = int(data_array[index_priority + 7 + request*8])
            vehicleETA = round(float(data_array[index_priority + 8 + request*8]), 1)
            duration = round(float(data_array[index_priority + 9 + request*8]), 1)
            priorityRequestStatus = int(data_array[index_priority + 10 + request*8])
            activeRequestTable.append({"vehicleID" : vehicleID, 
                                        "requestID" : requestID,
                                        "msgCount" : msgCount,
                                        "inBoundLane" : inBoundLaneID,
                                        "basicVehicleRole" : basicVehicleRole,
                                        "vehicleETA" : vehicleETA,
                                        "duration" : duration,
                                        "priorityRequestStatus" : priorityRequestStatus})




    interfaceJsonString = json.dumps({
        "PriorityRequestGeneratorStatus":
        {
            "hostVehicle" :
            {
                "secMark_Second" : secMark,
                "vehicleID" : hv_tempID,
                "vehicleType" : hv_vehicleType,
                "position" :
                {
                    "elevation_Meter" : hv_elevation_Meter,
                    "latitude_DecimalDegree" : hv_latitude_DecimalDegree,
                    "longitude_DecimalDegree" : hv_longitude_DecimalDegree
                },
                "heading_Degree" : hv_heading_Degree,
                "speed_MeterPerSecond": hv_speed_Meterpersecond,
                "laneID": hv_currentLane, 
                "signalGroup" : hv_currentLaneSignalGroup,
                "priorityStatus" : {"OnMAP" : onMAP, "requestSent" : requestSent}
            },
            "infrastructure": 
            {
                "availableMaps": availableMaps,
                "currentPhase" : hv_currentLaneSignalGroup, # data for signal head, min, and max
                "activeRequestTable" : activeRequestTable
            },
        }
    })
    s.sendto(interfaceJsonString.encode(),hmi_controller)
    time.sleep(0.1)
s.close() 