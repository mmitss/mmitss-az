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
              3 : "stop-And-Remain", # red light (Red ball) [Don't walk]
              4 : "pre-Movement", # not used in US
              5 : "permissive-Movement-Allowed", # permissive green (Green ball)
              6 : "protected-Movement-Allowed",  # protected green (e.g. left turn arrow) - Green Arrow (direction?) [also walk]
              7 : "permissive-clearance", # permissive yellow (clear intersection) - Yellow 
              8 : "protected-clearance", # protected yellow (clear intersection) - Yellow arrow  [ also ped clear= Flashing Don;t Walk]
              9 : "caution-Conflicting-Traffic", # flashing yellow (yield)
              } 
spat_signal_head = {"stop-And-Remain" : "red", "stop-Then-Proceed" : "red_flash", "protected-Movement-Allowed" : "green", "permissive-clearance" : "yellow", "protected-clearance" : "yellow",  "dark" : "dark"}
phase_status_map = { "dark" : '-', "red" : "R", "red_flash" : "F", "yellow" : "Y", "green" : "G"}
ped_status_map = { "dark" : "-", "red_flash" : '-', "red" : "DW", "yellow": "PC", "green" : "W"}
def phase_status_state(phase_status):
    for key in phase_status:
        if phase_status[key] == True:
            return key

def signal_head(phase_status):
    current_phase_status = {"red" : False, "red_flash" : False, "yellow" : False, "green" : False, "green_arrow" : False, "minEndTime" : phase_status["minEndTime"],
                            "maxEndTime" : phase_status["maxEndTime"], "dark" : False}
    current_phase_status[spat_signal_head[phase_status['currState']]] = True
    return current_phase_status

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

    #this is all remote vehicle information

    index_remoteVehicle = 10
    numRemoteVehicles = int(data_array[index_remoteVehicle])
    
    remoteVehicles = []
    for vehicle in range(0, 5): # assuming up to 5 remote vehicles for now
        rv_tempID = data_array[index_remoteVehicle + vehicle*7]
        rv_vehicleType = data_array[index_remoteVehicle + 2 + vehicle*7]
        rv_latitude_DecimalDegree= round(float(data_array[index_remoteVehicle + 3 + vehicle*7]), 8)
        rv_longitude_DecimalDegree= round(float(data_array[index_remoteVehicle + 4 + vehicle*7]), 8)
        rv_elevation_Meter= round(float(data_array[index_remoteVehicle + 5 + vehicle*7]), 1)
        rv_heading_Degree= round(float(data_array[index_remoteVehicle + 6 + vehicle*7]), 4)
        rv_speed_Meterpersecond= float(data_array[index_remoteVehicle + 7 + vehicle*7])
        rv_speed_mph= int((float(rv_speed_Meterpersecond) * 2.23694))

        rv_position = Position3D(rv_latitude_DecimalDegree, rv_longitude_DecimalDegree, rv_elevation_Meter)
        remoteVehicle = BasicVehicle(rv_tempID, secMark, rv_position, rv_speed_mph, rv_heading_Degree, rv_vehicleType)
        
        if vehicle < numRemoteVehicles:
             bv_dict = remoteVehicle.BasicVehicle2Dict()
             remoteVehicles.append(bv_dict)

 # infrastructure map data
    index_maps = 46
    numReceivedMaps = int(data_array[index_maps])
    availableMaps = []
    for receivedMap in range(0, 5): # assuming up to 5 maps have been received 
        map_intersectionID = int(data_array[index_maps + 1 + receivedMap*4])
        map_DescriptiveName = data_array[index_maps + 2 + receivedMap*4]
        map_active = bool_map[data_array[index_maps + 3 + receivedMap*4]]
        map_age = int(data_array[index_maps + 4 + receivedMap*4])
        if receivedMap < numReceivedMaps:
            availableMaps.append({"IntersectionID": map_intersectionID, "DescriptiveName": map_DescriptiveName, "active": map_active, "age" : map_age})                       

 # infrastructure SPaT data
    #signal phase status 
    numSPaT = 8 # currently we have one SPaT value for each signal phase. 
    index_spat = 67
    SPaT = []
    #spat_currentPhase = int(data_array[index_spat]) - 1 # phases are stored 0 to 7 (instead of 1 to 8)
    for spat in range(0, numSPaT):
        spat_phase = data_array[index_spat + spat*6]
        spat_currState = spat_state[int(data_array[index_spat + 1 + spat*6])]
        spat_startTime = round(float(data_array[index_spat + 2 + spat*6])/10., 1) # starttime is in 10ths of a second - show only one decimal point
        spat_minEndTime = round(float(data_array[index_spat + 3 + spat*6])/10., 1) # minEndTime is in 10ths of a second
        spat_maxEndTime = round(float(data_array[index_spat + 4 + spat*6])/10., 1) # maxEndTime is in 10ths of a second
        spat_elapsedTime = round(float(data_array[index_spat + 5 + spat*6])/10., 1) # elapsedTime is in 10ths of a second 
        SPaT.append({"phase" : spat_phase, "currState" : spat_currState, "minEndTime" : spat_minEndTime, "maxEndTime": spat_maxEndTime})

    #ped phase status
    numSPaTPed = 8 # currently we have one ped for each phase, but only 2, 4, 6, and 8 are real peds
    pedSPaT = []
    index_ped_spat = 115
    for spat in range(0, numSPaT):
        spat_phase = data_array[index_ped_spat + spat*6]
        spat_currState = spat_state[int(data_array[index_ped_spat + 1 + spat*6])]
        spat_startTime = round(float(data_array[index_ped_spat + 2 + spat*6])/10., 1) # starttime is in 10ths of a second - show only one decimal point
        spat_minEndTime = round(float(data_array[index_ped_spat + 3 + spat*6])/10., 1) # minEndTime is in 10ths of a second
        spat_maxEndTime = round(float(data_array[index_ped_spat + 4 + spat*6])/10., 1) # maxEndTime is in 10ths of a second
        spat_elapsedTime = round(float(data_array[index_ped_spat + 5 + spat*6])/10., 1) # elapsedTime is in 10ths of a second 
        pedSPaT.append({"phase" : spat_phase, "currState" : spat_currState, "minEndTime" : spat_minEndTime, "maxEndTime": spat_maxEndTime})

    # don't send raw spat data to hmi, send current phase state in red, yellow, green as True/False
    current_phase_status = signal_head(SPaT[hv_currentLaneSignalGroup]) #use the vehicles current signal group (phase)

    # add the 8-phase signal and ped status data
    phase_table = []
    for phase in range(0,8):
        phase_state = signal_head(SPaT[phase])
        ped_state = signal_head(pedSPaT[phase])
        phase_table.append({"phase" : phase, 
                            "phase_status" : phase_status_map[phase_status_state(phase_state)], 
                            "ped_status" : ped_status_map[phase_status_state(ped_state)]})


    #acquire priority status data
    index_priority = 163 # index is the column in the csv file
    activeRequestTable = []
    onMAP = bool_map[data_array[index_priority]]
    requestSent = bool_map[data_array[index_priority + 1]]
    for request in range(0, 5): # 5 test requests of data
        numActiveRequests = int(data_array[index_priority + 2])
        vehicleID = int(data_array[index_priority + 3])
        requestID = int(data_array[index_priority + 4])
        msgCount = int(data_array[index_priority + 5])
        inBoundLaneID = int(data_array[index_priority + 6])
        basicVehicleRole = basicVehicleRoles[int(data_array[index_priority + 7])]
        vehicleETA = round(float(data_array[index_priority + 8]), 1)
        duration = round(float(data_array[index_priority + 9]), 1)
        priorityRequestStatus = priority_responseStatus[int(data_array[index_priority + 10])]
        if request < numActiveRequests:
            activeRequestTable.append({"vehicleID" : vehicleID, 
                                      "requestID" : requestID,
                                      "msgCount" : msgCount,
                                      "inBoundLane" : inBoundLaneID,
                                      "basicVehicleRole" : basicVehicleRole,
                                      "vehicleETA" : vehicleETA,
                                      "duration" : duration})




    interfaceJsonString = json.dumps({
        "mmitss_hmi_interface":
        {
            "hostVehicle" :
            {
                "secMark_Second" : secMark,
                "temporaryID" : hv_tempID,
                "vehicleType" : hv_vehicleType,
                "position" :
                {
                    "elevation_Meter" : hv_elevation_Meter,
                    "latitude_DecimalDegree" : hv_latitude_DecimalDegree,
                    "longitude_DecimalDegree" : hv_longitude_DecimalDegree
                },
                "heading_Degree" : hv_heading_Degree,
                "speed_mph": hv_speed_mph,
                "lane": hv_currentLane, 
                "signalGroup" : hv_currentLaneSignalGroup,
                "priority" : {"OnMAP" : onMAP, "requestSent" : requestSent}
            },
            "remoteVehicles" :
                remoteVehicles,
            
            "infrastructure": 
            {
                "availableMaps": availableMaps,
                "currentPhase" : current_phase_status, # data for signal head, min, and max
                "phaseStates" : phase_table, #data for 8-phase display table
                "activeRequestTable" : activeRequestTable
            },
        }
    })
    s.sendto(interfaceJsonString.encode(),hmi)
    time.sleep(0.1)
s.close() 