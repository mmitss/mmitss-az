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
  1. Listents to port 20009 to receive updated from message transiever (remote bsm and spat data) and host vehicle (bsm, map, and priority status)
  2. Builds json representing the HMI states to send to the HMI
  3. Sends the json to the HMI
'''

import socket
import json
import time
import os
import J2735Helper

from Position3D import Position3D
from BasicVehicle import BasicVehicle

DEBUG = False

try :
    configfile = open('mmitss-phase3-hmi-config.json', 'r')
except :
    print("Unable to open mmitss-phase3-hmi-config.json")
    exit()

config = json.load(configfile)
controllerIP = config["HostIp"] #actual configuraiton data (should be from global config)
#controllerIP = '127.0.0.1' #use for simulation testing
controllerPort = config["PortNumber"]["HMIController"]

controller = (controllerIP, controllerPort)

hmiIP = '127.0.0.1' #hmi runs on the same computer/laptop
hmiPort = 20010
hmi = (hmiIP, hmiPort)

# Create an object of UtcHelper class (this converts timeMark (SAE J2735 object) to deciseconds for display
utcHelper = J2735Helper.J2735Helper()

bool_map = {"TRUE": True, "True": True, "FALSE": False, "False": False} # this could be come the SPaT phaseStatus data map
spat_map_active = False
spat_map_ID = -1 # map ID's are positive integers, so if no map is active set this to -1 to avoid any positive intergre number map

spat_state = {0 : "unknown", # based on the MOvementPhaseState from the SAE J2735 2016 standard - not comment in MovementPhaseState is that these are not used with UPER encoding (???)
              1 : "dark", 
              2 : "stop-Then-Proceed", # flashing red (flashing Red ball)
              "red" : "stop-And-Remain", # red light (Red ball) [Don't walk]
              4 : "pre-Movement", # not used in US
              5 : "permissive-Movement-Allowed", # permissive green (Green ball)
              "green" : "protected-Movement-Allowed",  # protected green (e.g. left turn arrow) - Green Arrow (direction?) [also walk]
              "permissive_yellow" : "permissive-clearance", # permissive yellow (clear intersection) - Yellow 
              "yellow" : "protected-clearance", # protected yellow (clear intersection) - Yellow arrow  [ also ped clear= Flashing Don;t Walk]
              9 : "caution-Conflicting-Traffic", # flashing yellow (yield)
              "do_not_walk": "stop-And-Remain",
              "ped_clear": "protected-clearance",
              "walk": "protected-Movement-Allowed",
              3 : "stop-And-Remain", # red light (Red ball) [Don't walk]
              6 : "protected-Movement-Allowed",  # protected green (e.g. left turn arrow) - Green Arrow (direction?) [also walk]
              7 : "permissive-clearance", # permissive yellow (clear intersection) - Yellow 
              8 : "protected-clearance", # protected yellow (clear intersection) - Yellow arrow  [ also ped clear= Flashing Don;t Walk]
              
              } 
spat_signal_head = {"stop-And-Remain" : "red", "stop-Then-Proceed" : "red_flash", "protected-Movement-Allowed" : "green", "permissive-Movement-Allowed" : "green",
    "permissive-clearance" : "yellow", "protected-clearance" : "yellow",  "dark" : "dark", "unknown" : "unknown",
    "pre-Movement" : "unknown", "caution-Conflicting-Traffic" : "yellow"}
phase_status_map = { "dark" : '-', "red" : "R", "red_flash" : "F", "yellow" : "Y", "green" : "G", "unknown" : "-"}
ped_status_map = { "dark" : "-", "red_flash" : '-', "red" : "DW", "yellow": "PC", "green" : "W", "unknown" : "-"}

def phase_status_state(phase_status):
    for key in phase_status:
        if phase_status[key] == True:
            return key

def signal_head(currentPhase, phase_status):
    current_phase_status = {"red" : False, "red_flash" : False, "yellow" : False, "green" : False, "green_arrow" : False, 
                            "minEndTime" : phase_status["minEndTime"],
                            "maxEndTime" : phase_status["maxEndTime"], "dark" : False}
    if currentPhase == 0 : #there is no SPaT data
        current_phase_status["dark"] = True
        current_phase_status["minEndTime"] = '--'
        current_phase_status["maxEndTime"] = '--'
    else :
        current_phase_status[spat_signal_head[spat_state[phase_status['currState']]]] = True
    return current_phase_status

# mapping of enumerated J2735 variables to meaningful text names for priority_responseStatus and basicVehicleRoles
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
                    13 : "ev",
                    16 : "transit",
                    11: "coord"}

vehicleTypes = {2: "emergencyVehicle",
                6: "transit",
                9: "truck"}


def manageRemoteVehicleList(remoteBSMjson, remoteVehicleList) :
    # get the id of the new BSM data and make it a positive numnber (is an unsigned int)
    vehicleID = abs(remoteBSMjson["BasicVehicle"]["temporaryID"])
    vehicleInformation = remoteBSMjson["BasicVehicle"]
    vehicleInformation["temporaryID"] = abs(vehicleInformation["temporaryID"])
    # cpp message uses key "type" instead of "vehicleType"
    vehicleInformation['vehicleType'] = remoteBSMjson["BasicVehicle"]["type"]
    vehicleInformation.pop('type')
    vehicleUpdateTime = time.time()
    # if there are no vehicles in the list, add the current vehicle 
    if len(remoteVehicleList) == 0 : 
        remoteVehicleList.append({"vehicleID" : abs(vehicleID), "vehicleInformation" : {"BasicVehicle" : vehicleInformation}, "vehicleUpdateTime" : vehicleUpdateTime})
        return remoteVehicleList
    # update existing vehicles
    rv_updated = False
    for rv in remoteVehicleList :
        if rv["vehicleID"] == abs(vehicleID) :
            #print("rv data: ", rv["vehicleInformation"])
            rv["vehicleInformation"] = {"BasicVehicle" : vehicleInformation}
            rv["vehicleUpdateTime"] = vehicleUpdateTime
            rv_updated = True
    if not rv_updated : #vehicle wasn't in the list of active vehicles, add it to the list
        remoteVehicleList.append({"vehicleID" : abs(vehicleID), "vehicleInformation" : {"BasicVehicle" : vehicleInformation}, "vehicleUpdateTime" : vehicleUpdateTime})
    return remoteVehicleList

def removeOldRemoteVehicles(remoteVehicleList) :
    tick = time.time()
    newRemoteVehicleList = [rv for rv in remoteVehicleList if not ((tick - rv["vehicleUpdateTime"]) > 0.5)]
    return newRemoteVehicleList

def changeSPaTTimes2Strings(SPaT) :
    newSPaT = SPaT
    for phase in range(0,8) :
        newSPaT[phase]['maxEndTime'] = str(round(float(SPaT[phase]['maxEndTime'])/10., 1))
        newSPaT[phase]['minEndTime'] = str(round(float(SPaT[phase]['minEndTime'])/10., 1))
        #newSPaT[phase]['startTime'] = str(SPaT[phase]['startTime'])
        #newSPaT[phase]['elapsedTime'] = str(round(float(SPaT[phase]['elapsedTime'])/10., 1))
    return newSPaT
    
# initialize all the data
#host vehicle data
secMark = 0
hv_tempID = int(0)
hv_vehicleType = " "
hv_latitude_DecimalDegree= round(0.0, 8)
hv_longitude_DecimalDegree= round(0.0, 8)
hv_elevation_Meter= round(0.0, 1)
hv_heading_Degree= round(0.0, 4)
hv_speed_Meterpersecond= float(0)
hv_speed_mph= int((float(hv_speed_Meterpersecond) * 2.23694))
hv_currentLane = int(0)
hv_currentLaneSignalGroup = int(0)
onMAP = False
requestSent = False
availableMaps = []
activeRequestTable = []

#remote vehicle data
rv_tempID = int(0)
rv_vehicleType = " "
rv_latitude_DecimalDegree= round(0.0, 8)
rv_longitude_DecimalDegree= round(0.0, 8)
rv_elevation_Meter= round(0.0, 1)
rv_heading_Degree= round(0.0, 4)
rv_speed_Meterpersecond= float(0.0)
rv_speed_mph= int((float(rv_speed_Meterpersecond) * 2.23694))
remoteVehicleList = []

#SPaT data minitialization and reset
markSPaTtime = 0.0
def reset_SPaT() :
    current_phase_status = {"red" : False, "red_flash" : False, "yellow" : False, "green" : False, "green_arrow" : False, "minEndTime" : '--',
                            "maxEndTime" : '--', "dark" : True} 
    phase_table = []
    for phase in range(0,8) :
        phase_table.append({"phase" : phase, 
                            "phase_status" : '--', 
                            "ped_status" : '--'})
    return current_phase_status, phase_table

spat_regionalID = int(0)
spat_intersectionID = int(0)
spat_msgCnt = int(0)
spat_minutesOfYear = int(0)
spat_msOfMinute = int(0)
spat_status = int(0)
phase_table = []
current_phase_status = {"red" : False, "red_flash" : False, "yellow" : False, "green" : False, "green_arrow" : False, "minEndTime" : '--',
                            "maxEndTime" : '--', "dark" : True}
phase_table = []
for phase in range(0,8) :
    phase_table.append({"phase" : phase, 
                            "phase_status" : '--', 
                            "ped_status" : '--'})

current_phase_status, phase_table = reset_SPaT() 

# setup timing variables for debugging
if DEBUG == True :
    import csv
    import datetime 
    tick_bsm = time.time()
    tick_SPaT = time.time()
    tick_priorityUpdate = time.time()
    # Create a timestamp which will be appended at the end of name of a file storing received data.
    timestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))
    # Create complete file name: example: msgLog_<timestamp>
    fileName = "controllerLog_" + timestamp + ".csv"
    # Open a file with created timestamp to store the received data. (Message log file).
    dataLog = open(fileName,'a+')
    dataLog.close()

# Create a socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Bind the created socket to the server information.
try :
    s.bind((controller))
except :
    print("Unable to open network:", controllerIP, ":", controllerPort)
    exit()


while True:
    
    #receive data from mmitss components
    line, addr = s.recvfrom(20480) 
    line = line.decode()
    sourceIP, sourcePort = addr

    try:

        if sourcePort == 10002:
            # process the remote vehicle and SPaT data
            # print('remote bsm and spat data', line)

            # load the json
            remoteInterfacejson = json.loads(line)

            if remoteInterfacejson["MsgType"] =='BSM' :
                # if debugging, output the time since the last message
                if DEBUG == True :
                    newremoteBSM = time.time()
                    dataLog = open(fileName,'a+')
                    dataLog.write('remoteBSM,' + str(newremoteBSM - tick_bsm) + '\n')
                    dataLog.close()
                    tick_bsm = newremoteBSM

                #translate remote basic vehicle data
                manageRemoteVehicleList(remoteInterfacejson, remoteVehicleList)
                #remoteVehicles.append(remoteInterfacejson["BSM"]) #how do I want to deal with a collection of remote vehicle data???? Currently, they get reported when host vehicle gets updated

            elif remoteInterfacejson["MsgType"] == 'SPaT' :
                
                #check to make sure it is spat
                # if debugging, output the time since the last message
                if DEBUG == True :
                    newSPaT = time.time()
                    dataLog = open(fileName,'a+')
                    dataLog.write('SPaT,' + str(newSPaT - tick_SPaT) + '\n')
                    dataLog.close()
                    tick_SPaT = newSPaT
                if spat_map_active :
                    markSPaTtime = time.time()
                    SPaT_data = json.loads(utcHelper.modify_spat_json_to_deciSecFromNow(json.dumps(remoteInterfacejson)))
                    # Fill inactive phases with "0"
                    vehPhasesLength = len(SPaT_data["Spat"]["phaseState"])
                    activeVehPhases = [phase["phaseNo"] for phase in SPaT_data["Spat"]["phaseState"]]
                    
                    pedPhasesLength = len(SPaT_data["Spat"]["pedPhaseState"])
                    activePedPhases = [phase["phaseNo"] for phase in SPaT_data["Spat"]["pedPhaseState"]]
                    
                    vehPhaseIndexer = 0
                    for vehPhase in range(8):
                        phaseNo = vehPhase + 1
                        if phaseNo not in activeVehPhases:
                            phaseDict = [dict({
                                                "phaseNo": phaseNo,
                                                "currState": 0,
                                                "startTime": -1,
                                                "minEndTime": -1,
                                                "maxEndTime": -1,
                                                "elapsedTime": -1
                                            })]
                            SPaT_data["Spat"]["phaseState"] += phaseDict
                    SPaT_data["Spat"]["phaseState"] = sorted(SPaT_data["Spat"]["phaseState"], key=lambda k: k['phaseNo']) 

                    
                    for pedPhase in range(8):
                        phaseNo = pedPhase + 1
                        if phaseNo not in activePedPhases:
                            phaseDict = [dict({
                                                "phaseNo": phaseNo,
                                                "currState": 0,
                                                "startTime": -1,
                                                "minEndTime": -1,
                                                "maxEndTime": -1,
                                                "elapsedTime": -1
                                            })]
                            SPaT_data["Spat"]["pedPhaseState"] += phaseDict
                    SPaT_data["Spat"]["pedPhaseState"] = sorted(SPaT_data["Spat"]["pedPhaseState"], key=lambda k: k['phaseNo']) 

                    SPaT = []
                    pedSPaT = []
                    if SPaT_data["Spat"]["IntersectionState"]["intersectionID"] == spat_map_ID :
                        spat_regionalID = int(SPaT_data["Spat"]["IntersectionState"]["regionalID"])
                        spat_intersectionID = int(SPaT_data["Spat"]["IntersectionState"]["intersectionID"])
                        spat_msgCnt = int(SPaT_data["Spat"]["msgCnt"])
                        spat_minutesOfYear = int(SPaT_data["Spat"]["minuteOfYear"])
                        spat_msOfMinute = int(SPaT_data["Spat"]["msOfMinute"])
                        #spat_status = int(SPaT_data["Spat"]["status"])
                        SPaT = SPaT_data["Spat"]["phaseState"]
                        SPaT = changeSPaTTimes2Strings(SPaT)
                        pedSPaT = SPaT_data["Spat"]["pedPhaseState"]
                        pedSPaT = changeSPaTTimes2Strings(pedSPaT)

                        # don't send raw spat data to hmi, send current phase state in red, yellow, green as True/False
                        
                        if hv_currentLaneSignalGroup == 0 :
                            current_phase_status = signal_head(hv_currentLaneSignalGroup, SPaT[hv_currentLaneSignalGroup])
                        else :
                            current_phase_status = signal_head(hv_currentLaneSignalGroup, SPaT[hv_currentLaneSignalGroup-1])
                    

                        # add the 8-phase signal and ped status data
                        phase_table = []
                        for phase in range(0,8):
                            phase_state = signal_head(hv_currentLaneSignalGroup, SPaT[phase])
                            ped_state = signal_head(hv_currentLaneSignalGroup, pedSPaT[phase])
                            phase_table.append({"phase" : phase, 
                                                "phase_status" : phase_status_map[phase_status_state(phase_state)], 
                                                "ped_status" : ped_status_map[phase_status_state(ped_state)]})
                else : # no active map for displaying spat data
                    current_phase_status, phase_table = reset_SPaT()
            else : 
                print('ERROR: remote vehicle or SPaT data expected')

            #publish the data to the HMI


        elif sourcePort == 20004 :

            # print('host vehicle and infrastructure data', line)
            
            # load the json
            hostAndInfrastructureData = json.loads(line)
            if DEBUG == True :
                newPriorityUpdate = time.time()
                dataLog = open(fileName,'a+')
                dataLog.write('priorityUpdate,' + str(newPriorityUpdate - tick_priorityUpdate) + '\n')
                dataLog.close()
                tick_priorityUpdate = newPriorityUpdate
                
            

            # process the host vehicle and infrastructure data
            hv_tempID = abs(int(hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["vehicleID"]))
            hv_vehicleTypeEnum = hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["vehicleType"]
            hv_latitude_DecimalDegree= round(hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["latitude_DecimalDegree"], 8)
            hv_longitude_DecimalDegree= round(hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["longitude_DecimalDegree"], 8)
            hv_elevation_Meter= round(hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["elevation_Meter"], 1)
            hv_heading_Degree= round(hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["heading_Degree"], 4)
            hv_speed_Meterpersecond= float(hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["speed_MeterPerSecond"])
            hv_speed_mph= int((float(hv_speed_Meterpersecond) * 2.23694))
        
            # Create a Basic Vehicle that represents the host vehicle
            hv_position = Position3D(hv_latitude_DecimalDegree, hv_longitude_DecimalDegree, hv_elevation_Meter)
            hostVehicle = BasicVehicle(hv_tempID, secMark, hv_position, hv_speed_mph, hv_heading_Degree, hv_vehicleType)

            # convert the vehicle type to a string
            vehicleType = vehicleTypes.get(hv_vehicleTypeEnum)
            if vehicleType :
                hv_vehicleType = vehicleType
            else :
                hv_vehicleType = hv_vehicleTypeEnum

            #need to acquire current lane and current lane signal group
            hv_currentLane = int(hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["laneID"])
            hv_currentLaneSignalGroup = int(hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["signalGroup"])
            onMAP = bool_map[hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["priorityStatus"]["OnMAP"]]

            requestSent = bool_map[hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["hostVehicle"]["priorityStatus"]["requestSent"]]

            availableMaps = hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["infrastructure"]["availableMaps"]

            # determine the active map so that only meaningful SPaT data will be displayed 
            spat_map_active = False
            spat_map_ID = -1 
            if availableMaps == [] or availableMaps == None :
                spat_map_active = False
                spat_map_ID = -1
            else :
                for map in availableMaps :
                    if map["active"] ==  "True" : # the value is sent as a string = True (but excel makes str TRUE in the simulator for test)
                        spat_map_ID = map["IntersectionID"]
                        spat_map_active = True  
                        

            activeRequestTable = hostAndInfrastructureData["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"]
            if activeRequestTable == None :
                activeRequestTable = []
            for request in activeRequestTable :
                responseStatusEnum = request["priorityRequestStatus"]
                #use .get in clase vehicle class is not in dictionary mapping class to text name, else send class enum
                responseStatus = priority_responseStatus.get(responseStatusEnum)
                if responseStatus :
                    request["priorityRequestStatus"] = responseStatus
                else :
                    request["priorityRequestStatus"] = responseStatusEnum

                vehicleRoleEnum = request["basicVehicleRole"]

                #use .get in clase vehicle class is not in dictionary mapping class to text name, else send class enum
                vehicleRole = basicVehicleRoles.get(vehicleRoleEnum)
                if vehicleRole : 
                    request["basicVehicleRole"] = vehicleRole
                else :
                    request["basicVehicleRole"] = vehicleRoleEnum 
                    
                # make sure vehicle ID is displayed as a positive number 
                request["vehicleID"] = abs(request["vehicleID"])


            # prepare the list of remote vehicles for display
            remoteVehicleList = removeOldRemoteVehicles(remoteVehicleList)
            remoteVehicles = []
            for rv in remoteVehicleList :
                remoteVehicles.append(rv["vehicleInformation"])

            #check to see if SPaT data is stale (defined to be older than 0.5 seconds)
            #if (time.time() - markSPaTtime) > 0.5 :
            #   print("current time: ", time.time(), "lastSPaT time:", markSPaTtime, "time difference: ", time.time() - markSPaTtime)
            #   current_phase_status, phase_table = reset_SPaT()

            #update the HMI with new data (assuming the 10 Hz host vehilce data is the update trigger)
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
            # print('update hmi: ', interfaceJsonString)

        else :
            print('ERROR: data received from unknown source')

    except Exception as e:
        print("[{}]".format(time.time()))
        print(e)
        print(line)
        print("\n")

    
s.close() 
