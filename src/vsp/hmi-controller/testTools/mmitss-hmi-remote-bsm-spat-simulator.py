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
import J2735Helper

from Position3D import Position3D
from BasicVehicle import BasicVehicle

# Create an object of UtcHelper class
utcHelper = J2735Helper.J2735Helper()

hmi_controllerIP = '127.0.0.1'
hmi_controllerPort = 20009
hmi_controller = (hmi_controllerIP, hmi_controllerPort)

bsm_spat_simIP = '127.0.0.1'
bsm_spat_simPort = 10004
bsm_spat_sim = (bsm_spat_simIP, bsm_spat_simPort)
inactiveVehPhases = [1,3,5,7]
inactivePedPhases = [1,3,5,7]

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
s.bind((bsm_spat_sim))

directory_path = os.getcwd()
f = open('HMIControllerSimulatorData.2.csv', 'r')
#f = open(directory_path + '/src/obu/controllerSimulatorforHMI/HMIControllerSimulatorData.1.csv', 'r')

ticks_init = time.time()

f.readline() 
f.readline() 
f.readline() # there are three informational lines at the top of the data file (top is category, second is data_array cound, third is data lable)
for line in f :
    line = line.replace('\n','')
    data_array = line.split(',')
    secMark = int(data_array[0])
   
    index_remoteVehicle = 10
    numRemoteVehicles = int(data_array[index_remoteVehicle])

    for vehicle in range(0, numRemoteVehicles): # assuming up to 5 remote vehicles for now
        rv_tempID = data_array[index_remoteVehicle + 1 + vehicle*7]
        rv_vehicleType = data_array[index_remoteVehicle + 2 + vehicle*7]
        rv_latitude_DecimalDegree= round(float(data_array[index_remoteVehicle + 3 + vehicle*7]), 8)
        rv_longitude_DecimalDegree= round(float(data_array[index_remoteVehicle + 4 + vehicle*7]), 8)
        rv_elevation_Meter= round(float(data_array[index_remoteVehicle + 5 + vehicle*7]), 1)
        rv_heading_Degree= round(float(data_array[index_remoteVehicle + 6 + vehicle*7]), 4)
        rv_speed_Meterpersecond= float(data_array[index_remoteVehicle + 7 + vehicle*7])
        rv_speed_mph= int((float(rv_speed_Meterpersecond) * 2.23694))
        
        if vehicle < numRemoteVehicles:          
            interfaceJsonString = json.dumps({
                "MsgType": "BSM", 
                "BasicVehicle" : 
                        {
                            "temporaryID" : rv_tempID,
                            "secMark_Second" : secMark,
                            "position" : 
                                { 
                                    "latitude_DecimalDegree" : rv_latitude_DecimalDegree,
                                    "longitude_DecimalDegree" : rv_longitude_DecimalDegree,
                                    "elevation_Meter" : rv_elevation_Meter
                                },
                            "speed_MeterPerSecond" : rv_speed_Meterpersecond,
                            "heading_Degree" : rv_heading_Degree,
                            "type" : rv_vehicleType
                        }
            })
            s.sendto(interfaceJsonString.encode(),hmi_controller)
            print("sent remote bsm at time: ", time.time() - ticks_init)
            time.sleep(0.1/(numRemoteVehicles+1))

    #infrastructure SPaT data
    #signal phase status 
    numSPaT = 8 # currently we have one SPaT value for each signal phase. 
    index_spat = 67
    SPaT = []
    spat_timeStamp = str(data_array[index_spat])
    spat_regionalID =  int(data_array[index_spat + 1])
    spat_intersectionID = int(data_array[index_spat + 2])
    spat_msgCnt = int(data_array[index_spat + 3])
    spat_minutesOfYear = int(data_array[index_spat + 4])
    spat_msOfMinute = int(data_array[index_spat + 5])
    # spat_status = int(data_array[index_spat + XX]) Not included in new real spat data
    
    index_phase_spat = 73
    for spat in range(0, numSPaT):
        spat_currState = int(data_array[index_phase_spat + spat*4])
        spat_minEndTime = int(data_array[index_phase_spat + 1 + spat*4]) # minEndTime is in 10ths of a second
        spat_maxEndTime = int(data_array[index_phase_spat + 2 + spat*4]) # maxEndTime is in 10ths of a second
        spat_elapsedTime = int(data_array[index_phase_spat + 3 + spat*4]) # elapsedTime is in 10ths of a second 
        SPaT.append({"phaseNo" : spat+1, "currState" : spat_currState, "minEndTime" : spat_minEndTime, "maxEndTime": spat_maxEndTime, "elapsedTime" : spat_elapsedTime})
    
    #ped phase status
    numSPaTPed = 8 # currently we have one ped for each phase, but only 2, 4, 6, and 8 are real peds
    pedSPaT = []
    index_ped_spat = 105
    for spat in range(0, numSPaT):
        spat_currState = int(data_array[index_ped_spat + spat*4])
        spat_minEndTime = int(data_array[index_ped_spat + 1 + spat*4]) # minEndTime is in 10ths of a second
        spat_maxEndTime = int(data_array[index_ped_spat + 2 + spat*4]) # maxEndTime is in 10ths of a second
        spat_elapsedTime = int(data_array[index_ped_spat + 3 + spat*4]) # elapsedTime is in 10ths of a second 
        pedSPaT.append({"phaseNo" : spat+1, "currState" : spat_currState, "minEndTime" : spat_minEndTime, "maxEndTime": spat_maxEndTime, "elapsedTime" : spat_elapsedTime})
    
    interfaceJsonString = json.dumps({
           "MsgType": "SPaT",   
	        "Spat":
            {
                "IntersectionState":
                {
                    "regionalID": spat_regionalID,
                    "intersectionID": spat_intersectionID
                },
                "msgCnt": spat_msgCnt,
                "minuteOfYear": spat_minutesOfYear,
                "msOfMinute": spat_msOfMinute,
                "phaseState" : SPaT,
                "pedPhaseState" : pedSPaT

            }
    })

    # Drop inactiveVehPhases
    interfaceJsonString = utcHelper.drop_inactive_phases(interfaceJsonString, inactiveVehPhases, inactivePedPhases)


    # Drop inactivePedPhases

    #convert SPaT time to utc for maxEnd and minEnd times
    interfaceJsonString = utcHelper.modify_spat_json_to_utc_timemark(interfaceJsonString)
    s.sendto(interfaceJsonString.encode(),hmi_controller)
    print("sent spat at time: ", time.time() - ticks_init)
    time.sleep(0.1/(numRemoteVehicles + 1))
s.close() 