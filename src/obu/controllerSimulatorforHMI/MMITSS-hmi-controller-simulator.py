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
spat_state = {0 : "unknown", # based on the MOvementPhaseState from the SAE J2735 2016 standard - not comment in MovementPhaseState is that these are not used with UPER encoding (???)
              1 : "dark", 
              2 : "stop-Then-Proceed", # flashing red (flashing Red ball)
              3 : "stop-And-Remain", # red light (Red ball)
              4 : "pre-Movement", # not used in US
              5 : "permissive-Movement-Allowed", # permissive green (Green ball)
              6 : "protected-Movement-Allowed",  # protected green (e.g. left turn arrow) - Green Arrow (direction?)
              7 : "permissive-clearance", # permissive yellow (clear intersection) - Yellow
              8 : "protected-clearance", # protected yellow (clear intersection) - Yellow arrow (direction? - look at heading?)
              9 : "caution-Conflicting-Traffic", # flashign yellow (yield)
              } 

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
    hostVehicle = BasicVehicle(hv_tempID, secMark, hv_position, hv_speed_mph, hv_heading_Degree, hv_vehicleType)

    #need to acquire current lane and current lane signal group
    hv_currentLane = 2
    hv_currentLaneSignalGroup = 2

    #this is all remote vehicle information

    index_remoteVehicle = 9
    numRemoteVehicles = int(data_array[8])
    
    remoteVehicles = []
    for vehicle in range(0, 5): # assuming up to 5 remote vehicles for now
        rv_tempID = data_array[index_remoteVehicle + vehicle*7]
        rv_vehicleType = data_array[index_remoteVehicle + 1 + vehicle*7]
        rv_latitude_DecimalDegree= float(data_array[index_remoteVehicle + 2 + vehicle*7])
        rv_longitude_DecimalDegree= float(data_array[index_remoteVehicle + 3 + vehicle*7])
        rv_elevation_Meter= float(data_array[index_remoteVehicle + 4 + vehicle*7])
        rv_heading_Degree= float(data_array[index_remoteVehicle + 5 + vehicle*7])
        rv_speed_Meterpersecond= float(data_array[index_remoteVehicle + 6 + vehicle*7])
        rv_speed_mph= round((float(rv_speed_Meterpersecond) * 2.23694),2)

        rv_position = Position3D(rv_latitude_DecimalDegree, rv_longitude_DecimalDegree, rv_elevation_Meter)
        remoteVehicle = BasicVehicle(rv_tempID, secMark, rv_position, rv_speed_mph, rv_heading_Degree, rv_vehicleType)
        
        if vehicle < numRemoteVehicles:
             bv_dict = remoteVehicle.BasicVehicle2Dict()
             remoteVehicles.append(bv_dict)


 # infrastructure map data

    numReceivedMaps = data_array[44]
    availableMaps = []
    for receivedMap in range(0, 5): # assuming up to 5 maps have been received 
        map_intersectionID = data_array[45 + receivedMap*4]
        map_DescriptiveName = data_array[46 + receivedMap*4]
        map_active = bool_map[data_array[47 + receivedMap*4]]
        map_age = data_array[48 + receivedMap*4]
        if receivedMap < numRemoteVehicles:
            availableMaps.append({"IntersectionID": map_intersectionID, "DescriptiveName": map_DescriptiveName, "active": map_active, "age" : map_age})                       
 
 # infrastructure SPaT data

    numSPaT = 8 # currently we have one SPaT value for each signal phase. This needs to be per lane of the active map.
    SPaT = []
    spat_currentPhase = data_array[65]
    for spat in range(0, numSPaT):
        spat_phase = data_array[66 + spat*6]
        spat_currState = spat_state[int(data_array[67 + spat*6])]
        spat_startTime = data_array[68 + spat*6]
        spat_minEndTime = data_array[69 + spat*6]
        spat_maxEndTime = data_array[70 + spat*6]
        spat_elapsedTime = data_array[71 + spat*6]
        SPaT.append({"phase" : spat_phase, "currState" : spat_currState, "minEndTime" : spat_minEndTime, "maxEndTime": spat_maxEndTime})



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
                "availableMaps": availableMaps,
                "currentPhase" : spat_currentPhase,
                "phaseStates" : SPaT,
            },
        }
    })
    s.sendto(interfaceJsonString.encode(),hmi)
    time.sleep(1)
s.close() 