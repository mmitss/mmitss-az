'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  V2XDataCollector.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
'''

import json
import time, datetime
import os
import shutil
from J2735Helper import J2735Helper

class V2XDataCollector:
    def __init__(self, environment:str):

        self.environment = environment

        self.loggingDirectory = None
        self.initializationTimestamp = None

        self.hostBsmLogfile = None
        self.remoteBsmLogfile = None
        self.spatLogfile = None
        self.srmLogfile = None
        self.ssmLogfile = None
        self.msgCountsLogfile = None

        configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
        config = (json.load(configFile))
        configFile.close()

        self.hostBsmDecoderPort = config["PortNumber"]["HostBsmDecoder"]
        
        if self.environment == "vehicle":
            self.baseName = "vehicle"
        else:
            self.baseName = config["IntersectionName"]

        self.path = "/nojournal/bin/v2x-data"
        
        if not os.path.exists(self.path + "/archive"):
            os.makedirs(self.path + "/archive")

        self.archive_leftover_directories()
        self.initialize_logfiles()

        self.j2735Helper = J2735Helper()

        
    

    def initialize_logfiles(self):
        self.initializationTimestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))
        self.loggingDirectory = self.path + "/" + self.baseName + "_" + self.initializationTimestamp + "/"
        
        os.makedirs(self.loggingDirectory)

        self.initialize_msgCountsLogfile()
        self.initialize_bsmLogfile("remote")
        self.initialize_spatLogfile()
        self.initialize_srmLogfile()
        self.initialize_ssmLogfile()

        if self.environment == "vehicle":
            self.initialize_bsmLogfile("host")


    def initialize_msgCountsLogfile(self):
        msgCountsLogfileName = self.loggingDirectory + self.baseName + "_" + "msgCountsLog_" + self.initializationTimestamp + ".csv"
        self.msgCountsLogfile = open(msgCountsLogfileName, 'w', buffering=1)
        
        if self.environment == "roadside":
            self.msgCountsLogfile.write("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,interval_sec,msg_source,msg_type,msg_count,msg_served,msg_rejected\n")

        elif self.environment == "vehicle":
            self.msgCountsLogfile.write("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,interval_sec,msg_source,msg_type,msg_count\n")

    def initialize_bsmLogfile(self, origin):
        bsmLogfileName = self.loggingDirectory + self.baseName + "_" + origin + "BsmLog_" + self.initializationTimestamp + ".csv"
        
        if self.environment == "vehicle":
            csvHeader = ("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,"
                                    + "temporaryId,secMark,latitude,longitude,elevation,speed,heading,type,length,width\n")
        elif self.environment == "roadside":
            csvHeader = ("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,"
                                    + "temporaryId,secMark,latitude,longitude,elevation,speed,heading,type,length,width,onmap_status,position_on_map,current_approach,current_lane,current_signal_group,dist_to_stopbar\n")

        if origin == "host":
            self.hostBsmLogfile = open(bsmLogfileName, 'w')
            self.hostBsmLogfile.write(csvHeader)
        
        elif origin == "remote":
            self.remoteBsmLogfile = open(bsmLogfileName, 'w')
            self.remoteBsmLogfile.write(csvHeader)

    def initialize_spatLogfile(self):
        spatLogfileName = self.loggingDirectory + self.baseName + "_" + "spatLog_" + self.initializationTimestamp + ".csv"
        self.spatLogfile = open(spatLogfileName, 'w')
        csvHeader = ("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,regionalId,intersectionId,msgCount,moy,msom," + 
                        "v1_currState,v1_minEndTime,v1_maxEndTime,v1_elapsedTime," +
                        "v2_currState,v2_minEndTime,v2_maxEndTime,v2_elapsedTime," +
                        "v3_currState,v3_minEndTime,v3_maxEndTime,v3_elapsedTime," +
                        "v4_currState,v4_minEndTime,v4_maxEndTime,v4_elapsedTime," +
                        "v5_currState,v5_minEndTime,v5_maxEndTime,v5_elapsedTime," +
                        "v6_currState,v6_minEndTime,v6_maxEndTime,v6_elapsedTime," +
                        "v7_currState,v7_minEndTime,v7_maxEndTime,v7_elapsedTime," +
                        "v8_currState,v8_minEndTime,v8_maxEndTime,v8_elapsedTime," +

                        "p1_currState,p1_minEndTime,p1_maxEndTime,p1_elapsedTime," +
                        "p2_currState,p2_minEndTime,p2_maxEndTime,p2_elapsedTime," +
                        "p3_currState,p3_minEndTime,p3_maxEndTime,p3_elapsedTime," +
                        "p4_currState,p4_minEndTime,p4_maxEndTime,p4_elapsedTime," +
                        "p5_currState,p5_minEndTime,p5_maxEndTime,p5_elapsedTime," +
                        "p6_currState,p6_minEndTime,p6_maxEndTime,p6_elapsedTime," +
                        "p7_currState,p7_minEndTime,p7_maxEndTime,p7_elapsedTime," +
                        "p8_currState,p8_minEndTime,p8_maxEndTime,p8_elapsedTime" + "\n")
        self.spatLogfile.write(csvHeader)

    def initialize_srmLogfile(self):
        srmLogfileName = self.loggingDirectory + self.baseName + "_" + "srmLog_" + self.initializationTimestamp + ".csv"
        self.srmLogfile = open(srmLogfileName, 'w', buffering=1)
        csvHeader = ("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose" + "," 
                                + "timestamp_posix" + ","
                                + "minuteOfYear" + "," 
                                + "msOfMinute" + "," 
                                + "msgCount" + "," 
                                + "regionalID" + "," 
                                + "intersectionID" + "," 
                                + "priorityRequestType" + "," 
                                + "basicVehicleRole" + "," 
                                + "laneID" + "," 
                                + "eTA_Minute" + "," 
                                + "eTA_Second" + "," 
                                + "eTA_Duration" + "," 
                                + "vehicleID" + "," 
                                + "latitude" + "," 
                                + "longitude" + "," 
                                + "elevation" + "," 
                                + "heading" + "," 
                                + "speed" + "," 
                                + "vehicleType"
                                + "\n")
        self.srmLogfile.write(csvHeader)

    def initialize_ssmLogfile(self):
        ssmLogfileName = self.loggingDirectory + self.baseName + "_" + "ssmLog_" + self.initializationTimestamp + ".csv"
        self.ssmLogfile = open(ssmLogfileName, 'w', buffering=1)
        csvHeader = ("log_timestamp_verbose" + "," 
                    + "log_timestamp_posix" + "," 
                    + "timestamp_verbose" + "," 
                    + "timestamp_posix" + "," 
                    + "minuteOfYear" + "," 
                    + "msOfMinute" + "," 
                    + "sequenceNumber" + "," 
                    + "updateCount" + "," 
                    + "regionalID" + "," 
                    + "noOfRequest" + "," 
                    + "intersectionID" + 
                    "," + "r1_vehicleID,r1_msgCount,r1_basicVehicleRole,r1_inBoundLaneID,r1_ETA_Minute,r1_ETA_Second,r1_ETA_Duration,r1_priorityRequestStatus" + 
                    "," + "r2_vehicleID,r2_msgCount,r2_basicVehicleRole,r2_inBoundLaneID,r2_ETA_Minute,r2_ETA_Second,r2_ETA_Duration,r2_priorityRequestStatus" + 
                    "," + "r3_vehicleID,r3_msgCount,r3_basicVehicleRole,r3_inBoundLaneID,r3_ETA_Minute,r3_ETA_Second,r3_ETA_Duration,r3_priorityRequestStatus" + 
                    "," + "r4_vehicleID,r4_msgCount,r4_basicVehicleRole,r4_inBoundLaneID,r4_ETA_Minute,r4_ETA_Second,r4_ETA_Duration,r4_priorityRequestStatus" + 
                    "," + "r5_vehicleID,r5_msgCount,r5_basicVehicleRole,r5_inBoundLaneID,r5_ETA_Minute,r5_ETA_Second,r5_ETA_Duration,r5_priorityRequestStatus\n")
        self.ssmLogfile.write(csvHeader)

    def write_msgCount(self, msgCounts:json):
        csvRow = self.msgCounts_json_to_csv(msgCounts)
        self.msgCountsLogfile.write(csvRow)

    def write_bsm(self, bsmJson:json, senderPort:int):        
        csvRow = self.bsm_json_to_csv(bsmJson)
        if ((self.environment == "vehicle") and (senderPort == self.hostBsmDecoderPort)):
            self.hostBsmLogfile.write(csvRow)
        else:
            self.remoteBsmLogfile.write(csvRow)

    def write_spat(self, spatJson:json):
        csvRow = self.spat_json_to_csv(spatJson)
        self.spatLogfile.write(csvRow)

    def write_srm(self, srmJson:json):
        csvRow = self.srm_json_to_csv(srmJson)
        self.srmLogfile.write(csvRow)        

    def write_ssm(self, ssmJson:json):
        csvRow = self.ssm_json_to_csv(ssmJson)
        self.ssmLogfile.write(csvRow)

    def msgCounts_json_to_csv(self, jsonData:json):
        log_timestamp_posix = str(time.time())
        log_timestamp_verbose = str(datetime.datetime.now())
        timestamp_posix = str(jsonData["MsgInformation"]["Timestamp_posix"])
        timestamp_verbose = str(jsonData["MsgInformation"]["Timestamp_verbose"])
        timeInterval = str(jsonData["MsgInformation"]["TimeInterval"])
        msgSource = str(jsonData["MsgInformation"]["MsgSource"])
        msgType = str(jsonData["MsgInformation"]["MsgCountType"])
        msgCount = str(jsonData["MsgInformation"]["MsgCount"])
                
        csv = (log_timestamp_verbose + "," +
               log_timestamp_posix + "," +
               timestamp_verbose + "," +
               timestamp_posix + "," + 
               timeInterval + "," +
               msgSource + "," +
               msgType + "," +
               msgCount)

        if self.environment == "roadside":
            msgServed = str(jsonData["MsgInformation"]["MsgServed"])
            msgRejected = str(jsonData["MsgInformation"]["MsgRejected"])
            csv = csv + "," + msgServed + "," + msgRejected

        csv = csv + "\n"

        return csv

    def bsm_json_to_csv(self, jsonData:json):
        log_timestamp_verbose = str(datetime.datetime.now())
        log_timestamp_posix = str(time.time())
        timestamp_verbose = str(jsonData["Timestamp_verbose"])
        timestamp_posix = str(jsonData["Timestamp_posix"])
        temporaryId = str(jsonData["BasicVehicle"]["temporaryID"])
        secMark = str(jsonData["BasicVehicle"]["secMark_Second"])
        latitude = str(jsonData["BasicVehicle"]["position"]["latitude_DecimalDegree"])
        longitude = str(jsonData["BasicVehicle"]["position"]["longitude_DecimalDegree"])
        elevation = str(jsonData["BasicVehicle"]["position"]["elevation_Meter"])
        speed = str(jsonData["BasicVehicle"]["speed_MeterPerSecond"])
        heading = str(jsonData["BasicVehicle"]["heading_Degree"])
        vehType = str(jsonData["BasicVehicle"]["type"])
        length = str(jsonData["BasicVehicle"]["size"]["length_cm"])
        width = str(jsonData["BasicVehicle"]["size"]["width_cm"])

        if self.environment == "roadside":
            onMap = str(jsonData["OnmapVehicle"]["onMap"])
            approachId = str(jsonData["OnmapVehicle"]["approachId"])
            laneId = str(jsonData["OnmapVehicle"]["laneId"])
            signalGroup = str(jsonData["OnmapVehicle"]["signalGroup"])
            distanceToStopbar = str(jsonData["OnmapVehicle"]["distanceToStopbar"])
            locationOnMap = str(jsonData["OnmapVehicle"]["locationOnMap"])
            
            csv = (log_timestamp_verbose + "," 
                + log_timestamp_posix + "," 
                + timestamp_verbose + "," 
                + timestamp_posix + "," 
                + temporaryId + "," 
                + secMark + "," 
                + latitude + "," 
                + longitude + "," 
                + elevation + "," 
                + speed + "," 
                + heading + "," 
                + vehType + ","
                + length + "," 
                + width + "," 
                + onMap + "," 
                + locationOnMap + ","
                + approachId + ","  
                + laneId + ","  
                + signalGroup + ","  
                + distanceToStopbar + "\n")
                
        elif self.environment == "vehicle":
            csv = (log_timestamp_verbose + "," 
                    + log_timestamp_posix + "," 
                    + timestamp_verbose + "," 
                    + timestamp_posix + "," 
                    + temporaryId + "," 
                    + secMark + "," 
                    + latitude + "," 
                    + longitude + "," 
                    + elevation + "," 
                    + speed + "," 
                    + heading + "," 
                    + vehType + ","
                    + length + "," 
                    + width + "\n")

        return csv

    def spat_json_to_csv(self, jsonData:json):
        log_timestamp_verbose = str(datetime.datetime.now())
        log_timestamp_posix = str(time.time())
        timestamp_verbose = str(jsonData["Timestamp_verbose"])
        timestamp_posix = str(jsonData["Timestamp_posix"])
        regionalId = str(jsonData["Spat"]["IntersectionState"]["regionalID"])
        intersectionId = str(jsonData["Spat"]["IntersectionState"]["intersectionID"])
        msgCnt = str(jsonData["Spat"]["msgCnt"])
        moy = str(jsonData["Spat"]["minuteOfYear"])
        msom = str(jsonData["Spat"]["msOfMinute"])

        vCurrStates = ["inactive" for i in range (8)]
        vMinEndTimes = ["-1" for i in range (8)]
        vMaxEndTimes = ["-1" for i in range (8)]
        vElapsedTimes = ["-1" for i in range (8)]

        pCurrStates = ["inactive" for i in range (8)]
        pMinEndTimes = ["-1" for i in range (8)]
        pMaxEndTimes = ["-1" for i in range (8)]
        pElapsedTimes = ["-1" for i in range (8)]

        for vehPhase in jsonData["Spat"]["phaseState"]:
            phaseNo = vehPhase["phaseNo"]
            phaseIndex = phaseNo-1
            vCurrStates[phaseIndex] = vehPhase["currState"]
            vMinEndTimes[phaseIndex] = str(vehPhase["minEndTime"])
            vMaxEndTimes[phaseIndex] = str(vehPhase["minEndTime"])
            vElapsedTimes[phaseIndex] = str(vehPhase["elapsedTime"])

        for pedPhase in jsonData["Spat"]["pedPhaseState"]:
            phaseNo = pedPhase["phaseNo"]
            phaseIndex = phaseNo-1
            pCurrStates[phaseIndex] = pedPhase["currState"]
            pMinEndTimes[phaseIndex] = str(pedPhase["minEndTime"])
            pMaxEndTimes[phaseIndex] = str(pedPhase["minEndTime"])
            pElapsedTimes[phaseIndex] = str(pedPhase["elapsedTime"])

        csv = (log_timestamp_verbose + "," + log_timestamp_posix + "," + timestamp_verbose + "," + timestamp_posix + "," + 
                regionalId + "," + intersectionId + "," + msgCnt + "," + moy + "," + msom + "," +

                vCurrStates[0] + "," + vMinEndTimes[0] + "," + vMaxEndTimes[0] + "," + vElapsedTimes[0] + "," +
                vCurrStates[1] + "," + vMinEndTimes[1] + "," + vMaxEndTimes[1] + "," + vElapsedTimes[1] + "," +
                vCurrStates[2] + "," + vMinEndTimes[2] + "," + vMaxEndTimes[2] + "," + vElapsedTimes[2] + "," +
                vCurrStates[3] + "," + vMinEndTimes[3] + "," + vMaxEndTimes[3] + "," + vElapsedTimes[3] + "," +
                vCurrStates[4] + "," + vMinEndTimes[4] + "," + vMaxEndTimes[4] + "," + vElapsedTimes[4] + "," +
                vCurrStates[5] + "," + vMinEndTimes[5] + "," + vMaxEndTimes[5] + "," + vElapsedTimes[5] + "," +
                vCurrStates[6] + "," + vMinEndTimes[6] + "," + vMaxEndTimes[6] + "," + vElapsedTimes[6] + "," +
                vCurrStates[7] + "," + vMinEndTimes[7] + "," + vMaxEndTimes[7] + "," + vElapsedTimes[7] + "," +

                pCurrStates[0] + "," + pMinEndTimes[0] + "," + pMaxEndTimes[0] + "," + pElapsedTimes[0] + "," +
                pCurrStates[1] + "," + pMinEndTimes[1] + "," + pMaxEndTimes[1] + "," + pElapsedTimes[1] + "," +
                pCurrStates[2] + "," + pMinEndTimes[2] + "," + pMaxEndTimes[2] + "," + pElapsedTimes[2] + "," +
                pCurrStates[3] + "," + pMinEndTimes[3] + "," + pMaxEndTimes[3] + "," + pElapsedTimes[3] + "," +
                pCurrStates[4] + "," + pMinEndTimes[4] + "," + pMaxEndTimes[4] + "," + pElapsedTimes[4] + "," +
                pCurrStates[5] + "," + pMinEndTimes[5] + "," + pMaxEndTimes[5] + "," + pElapsedTimes[5] + "," +
                pCurrStates[6] + "," + pMinEndTimes[6] + "," + pMaxEndTimes[6] + "," + pElapsedTimes[6] + "," +
                pCurrStates[7] + "," + pMinEndTimes[7] + "," + pMaxEndTimes[7] + "," + pElapsedTimes[7] + "," + "\n")
        
        return csv

    def srm_json_to_csv(self, jsonData:json):
        log_timestamp_verbose = str(datetime.datetime.now())
        log_timestamp_posix = str(time.time())
        timestamp_verbose = str(jsonData["Timestamp_verbose"])
        timestamp_posix = str(jsonData["Timestamp_posix"])
        minuteOfYear = str(jsonData["SignalRequest"]["minuteOfYear"])
        msOfMinute = str(jsonData["SignalRequest"]["msOfMinute"])
        msgCount = str(jsonData["SignalRequest"]["msgCount"])
        regionalID = str(jsonData["SignalRequest"]["regionalID"])
        intersectionID = str(jsonData["SignalRequest"]["intersectionID"])
        priorityRequestType = str(jsonData["SignalRequest"]["priorityRequestType"])
        basicVehicleRole = str(jsonData["SignalRequest"]["basicVehicleRole"])
        laneID = str(jsonData["SignalRequest"]["inBoundLane"]["LaneID"])
        eTA_Minute = str(jsonData["SignalRequest"]["expectedTimeOfArrival"]["ETA_Minute"])
        eTA_Second = str(jsonData["SignalRequest"]["expectedTimeOfArrival"]["ETA_Second"])
        eTA_Duration = str(jsonData["SignalRequest"]["expectedTimeOfArrival"]["ETA_Duration"])
        vehicleID = str(jsonData["SignalRequest"]["vehicleID"])
        latitude = str(jsonData["SignalRequest"]["position"]["latitude_DecimalDegree"])
        longitude = str(jsonData["SignalRequest"]["position"]["longitude_DecimalDegree"])
        elevation = str(jsonData["SignalRequest"]["position"]["elevation_Meter"])
        heading = str(jsonData["SignalRequest"]["heading_Degree"])
        speed = str(jsonData["SignalRequest"]["speed_MeterPerSecond"])
        vehicleType = str(jsonData["SignalRequest"]["vehicleType"])
        
        csv = (log_timestamp_verbose + "," 
                + log_timestamp_posix + "," 
                + timestamp_verbose + "," 
                + timestamp_posix + ","
                + minuteOfYear + "," 
                + msOfMinute + "," 
                + msgCount + "," 
                + regionalID + "," 
                + intersectionID + "," 
                + priorityRequestType + "," 
                + basicVehicleRole + "," 
                + laneID + "," 
                + eTA_Minute + "," 
                + eTA_Second + "," 
                + eTA_Duration + "," 
                + vehicleID + "," 
                + latitude + "," 
                + longitude + "," 
                + elevation + "," 
                + heading + "," 
                + speed + "," 
                + vehicleType
                + "\n")
        return csv

    def ssm_json_to_csv(self, jsonData:json):
        log_timestamp_verbose = str(datetime.datetime.now())
        log_timestamp_posix = str(time.time())
        timestamp_verbose = str(jsonData["Timestamp_verbose"])
        timestamp_posix = str(jsonData["Timestamp_posix"])
        noOfRequest = (jsonData["noOfRequest"])
        minuteOfYear = str(jsonData["SignalStatus"]["minuteOfYear"])
        msOfMinute = str(jsonData["SignalStatus"]["msOfMinute"])
        sequenceNumber = str(jsonData["SignalStatus"]["sequenceNumber"])
        updateCount = str(jsonData["SignalStatus"]["updateCount"])
        regionalID = str(jsonData["SignalStatus"]["regionalID"])
        intersectionID = str(jsonData["SignalStatus"]["intersectionID"])

        static_csv = (log_timestamp_verbose + "," 
                        + log_timestamp_posix + "," 
                        + timestamp_verbose + "," 
                        + timestamp_posix + "," 
                        + minuteOfYear + "," 
                        + msOfMinute + "," 
                        + sequenceNumber + "," 
                        + updateCount + "," 
                        + regionalID + "," 
                        + str(noOfRequest) + "," 
                        + intersectionID)

        dynamic_csv = ""

        for request in range(0,noOfRequest):
            vehicleID = str(jsonData["SignalStatus"]["requestorInfo"][request]["vehicleID"])
            msgCount = str(jsonData["SignalStatus"]["requestorInfo"][request]["msgCount"])
            basicVehicleRole = str(jsonData["SignalStatus"]["requestorInfo"][request]["basicVehicleRole"])
            inBoundLaneID = str(jsonData["SignalStatus"]["requestorInfo"][request]["inBoundLaneID"])
            ETA_Minute = str(jsonData["SignalStatus"]["requestorInfo"][request]["ETA_Minute"])
            ETA_Second = str(jsonData["SignalStatus"]["requestorInfo"][request]["ETA_Second"])
            ETA_Duration = str(jsonData["SignalStatus"]["requestorInfo"][request]["ETA_Duration"])
            priorityRequestStatus = str(jsonData["SignalStatus"]["requestorInfo"][request]["priorityRequestStatus"])

            request_csv = ("," + vehicleID +
                            "," + msgCount +
                            "," + basicVehicleRole +
                            "," + inBoundLaneID +
                            "," + ETA_Minute +
                            "," + ETA_Second +
                            "," + ETA_Duration +
                            "," + priorityRequestStatus )
            dynamic_csv = dynamic_csv + request_csv

        csv = static_csv + dynamic_csv + "\n"
        return csv
    
    def decode_and_store_data(self, data:bytes, senderPort:int):
        try:
            receivedMsg = json.loads(data.decode())
            if receivedMsg["MsgType"] == "BSM":
                self.write_bsm(receivedMsg,senderPort)
            elif receivedMsg["MsgType"] == "SPaT":
                spatString = self.j2735Helper.modify_spat_json_to_deciSecFromNow(json.dumps(receivedMsg))
                self.write_spat(json.loads(spatString))
            elif receivedMsg["MsgType"] == "SRM":
                self.write_srm(receivedMsg)
            elif receivedMsg["MsgType"] == "SSM":
                self.write_ssm(receivedMsg)
            elif receivedMsg["MsgType"] == "MsgCount":
                self.write_msgCount(receivedMsg)
        except:
            print("Failed decoding of received message at: " + str(time.time()))

    def close_logfiles(self):

        if not self.msgCountsLogfile.closed:
            self.msgCountsLogfile.close()

        if not self.remoteBsmLogfile.closed:
            self.remoteBsmLogfile.close()

        if not self.spatLogfile.closed:
            self.spatLogfile.close()

        if not self.srmLogfile.closed:
            self.srmLogfile.close()

        if not self.ssmLogfile.closed:
            self.ssmLogfile.close()
        
        if self.environment == "vehicle":
            if not self.hostBsmLogfile.closed:
                self.hostBsmLogfile.close()

        self.archive_current_directory()
        
    def archive_current_directory(self):
        shutil.move(self.loggingDirectory, (self.path + "/archive/"))

    def archive_leftover_directories(self):
        directories = list(os.walk(self.path))[0][1]
        directories.remove("archive")
        if len(directories) > 0:
            for directory in directories:
                shutil.move((self.path + "/" + directory), (self.path + "/archive/"))
                directories = list(os.walk(self.path))[0][1]
                directories.remove("archive")

if __name__ == "__main__":
    pass
