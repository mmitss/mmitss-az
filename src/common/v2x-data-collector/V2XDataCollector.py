import json
import time, datetime
import os

class V2XDataCollector:
    def __init__(self, environment:str):

        self.environment = environment
        self.hostBsmLogFile = None
        self.remoteBsmLogfile = None
        self.spatLogfile = None
        self.srmLogfile = None
        self.ssmLogflie = None

        configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
        config = (json.load(configFile))
        configFile.close()

        self.msgDecoderPort = config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]
        self.hostBsmDecoderPort = config["PortNumber"]["HostBsmDecoder"]
        
        if self.environment == "vehicle":
            self.baseName = "vehicle"
        else:
            self.baseName = config["IntersectionName"]

        self.initialize_logfiles()
    
    def initialize_logfiles(self):
        timestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))
        directory = "/nojournal/bin/v2x_data/" + self.baseName + "_" + timestamp + "/"
        os.makedirs(directory)

        remoteBsmLogfileName = directory + self.baseName + "_" + "RemoteBsmLog_" + timestamp + ".csv"
        self.remoteBsmLogfile = open(remoteBsmLogfileName, 'w')
        self.remoteBsmLogfile.write("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,"
                                        + "temporaryId,secMark,latitude,longitude,elevation,speed,heading,type,length,width\n")

        spatLogfileName = directory + self.baseName + "_" + "SpatLog_" + timestamp + ".csv"
        self.spatLogfile = open(spatLogfileName, 'w')
        self.spatLogfile.write("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,regionalId,intersectionId,msgCount,moy,msom," + 
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


        srmLogfileName = directory + self.baseName + "_" + "SrmLog_" + timestamp + ".csv"
        self.srmLogfile = open(srmLogfileName, 'w')
        self.srmLogfile.write(("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose" + "," 
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
                                + "\n"))

        ssmLogfileName = directory + self.baseName + "_" + "SsmLog_" + timestamp + ".csv"
        self.ssmLogfile = open(ssmLogfileName, 'w')
        self.ssmLogfile.write("log_timestamp_verbose" + "," 
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

        if self.environment == "vehicle":
            hostBsmLogfileName = directory + self.baseName + "_" + "HostBsmLog_" + timestamp + ".csv"
            self.hostBsmLogfile = open(hostBsmLogfileName, 'w')
            self.remoteBsmLogfile.write("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,"
                                        + "temporaryId,secMark,latitude,longitude,elevation,speed,heading,type,length,width\n")

    def close_logfiles(self):
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

    def write_bsm(self, bsmJson:json, senderPort:int):
        
        csvRow = self.bsmJsonToCsv(bsmJson)
        if ((self.environment == "vehicle") and (senderPort == self.hostBsmDecoderPort)):
            self.hostBsmLogFile.write(csvRow)
        else:
            self.remoteBsmLogfile.write(csvRow)

    def write_spat(self, spatJson:json):
        csvRow = self.spatJsonToCsv(spatJson)
        self.spatLogfile.write(csvRow)

    def write_srm(self, srmJson:json):
        csvRow = self.srmJsonToCsv(srmJson)
        self.spatLogfile.write(csvRow)        

    def write_ssm(self, ssmJson:json):
        csvRow = self.ssmJsonToCsv(ssmJson)
        self.spatLogfile.write(csvRow)

    def bsmJsonToCsv(self, jsonData:json):
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

    def spatJsonToCsv(self, jsonData:json):
        log_timestamp_verbose = str(datetime.datetime.now())
        log_timestamp_posix = str(time.time())
        timestamp_verbose = str(jsonData["Timestamp_verbose"])
        timestamp_posix = str(jsonData["Timestamp_posix"])
        regionalId = str(jsonData["Spat"]["IntersectionState"]["regionalID"])
        intersectionId = str(jsonData["Spat"]["IntersectionState"]["intersectionID"])
        msgCnt = str(jsonData["Spat"]["msgCnt"])
        moy = str(jsonData["Spat"]["minuteOfYear"])
        msom = str(jsonData["Spat"]["msOfMinute"])

        v1_currState = str(jsonData["Spat"]["phaseState"][0]["currState"])
        v1_minEndTime = str(jsonData["Spat"]["phaseState"][0]["minEndTime"])
        v1_maxEndTime = str(jsonData["Spat"]["phaseState"][0]["maxEndTime"])
        v1_elapsedTime = str(jsonData["Spat"]["phaseState"][0]["elapsedTime"])

        v2_currState = str(jsonData["Spat"]["phaseState"][1]["currState"])
        v2_minEndTime = str(jsonData["Spat"]["phaseState"][1]["minEndTime"])
        v2_maxEndTime = str(jsonData["Spat"]["phaseState"][1]["maxEndTime"])
        v2_elapsedTime = str(jsonData["Spat"]["phaseState"][1]["elapsedTime"])

        v3_currState = str(jsonData["Spat"]["phaseState"][2]["currState"])
        v3_minEndTime = str(jsonData["Spat"]["phaseState"][2]["minEndTime"])
        v3_maxEndTime = str(jsonData["Spat"]["phaseState"][2]["maxEndTime"])
        v3_elapsedTime = str(jsonData["Spat"]["phaseState"][2]["elapsedTime"])

        v4_currState = str(jsonData["Spat"]["phaseState"][3]["currState"])
        v4_minEndTime = str(jsonData["Spat"]["phaseState"][3]["minEndTime"])
        v4_maxEndTime = str(jsonData["Spat"]["phaseState"][3]["maxEndTime"])
        v4_elapsedTime = str(jsonData["Spat"]["phaseState"][3]["elapsedTime"])

        v5_currState = str(jsonData["Spat"]["phaseState"][4]["currState"])
        v5_minEndTime = str(jsonData["Spat"]["phaseState"][4]["minEndTime"])
        v5_maxEndTime = str(jsonData["Spat"]["phaseState"][4]["maxEndTime"])
        v5_elapsedTime = str(jsonData["Spat"]["phaseState"][4]["elapsedTime"])

        v6_currState = str(jsonData["Spat"]["phaseState"][5]["currState"])
        v6_minEndTime = str(jsonData["Spat"]["phaseState"][5]["minEndTime"])
        v6_maxEndTime = str(jsonData["Spat"]["phaseState"][5]["maxEndTime"])
        v6_elapsedTime = str(jsonData["Spat"]["phaseState"][5]["elapsedTime"])

        v7_currState = str(jsonData["Spat"]["phaseState"][6]["currState"])
        v7_minEndTime = str(jsonData["Spat"]["phaseState"][6]["minEndTime"])
        v7_maxEndTime = str(jsonData["Spat"]["phaseState"][6]["maxEndTime"])
        v7_elapsedTime = str(jsonData["Spat"]["phaseState"][6]["elapsedTime"])

        v8_currState = str(jsonData["Spat"]["phaseState"][7]["currState"])
        v8_minEndTime = str(jsonData["Spat"]["phaseState"][7]["minEndTime"])
        v8_maxEndTime = str(jsonData["Spat"]["phaseState"][7]["maxEndTime"])
        v8_elapsedTime = str(jsonData["Spat"]["phaseState"][7]["elapsedTime"])

        p1_currState = str(jsonData["Spat"]["pedPhaseState"][0]["currState"])
        p1_minEndTime = str(jsonData["Spat"]["pedPhaseState"][0]["minEndTime"])
        p1_maxEndTime = str(jsonData["Spat"]["pedPhaseState"][0]["maxEndTime"])
        p1_elapsedTime = str(jsonData["Spat"]["pedPhaseState"][0]["elapsedTime"])

        p2_currState = str(jsonData["Spat"]["pedPhaseState"][1]["currState"])
        p2_minEndTime = str(jsonData["Spat"]["pedPhaseState"][1]["minEndTime"])
        p2_maxEndTime = str(jsonData["Spat"]["pedPhaseState"][1]["maxEndTime"])
        p2_elapsedTime = str(jsonData["Spat"]["pedPhaseState"][1]["elapsedTime"])

        p3_currState = str(jsonData["Spat"]["pedPhaseState"][2]["currState"])
        p3_minEndTime = str(jsonData["Spat"]["pedPhaseState"][2]["minEndTime"])
        p3_maxEndTime = str(jsonData["Spat"]["pedPhaseState"][2]["maxEndTime"])
        p3_elapsedTime = str(jsonData["Spat"]["pedPhaseState"][2]["elapsedTime"])

        p4_currState = str(jsonData["Spat"]["pedPhaseState"][3]["currState"])
        p4_minEndTime = str(jsonData["Spat"]["pedPhaseState"][3]["minEndTime"])
        p4_maxEndTime = str(jsonData["Spat"]["pedPhaseState"][3]["maxEndTime"])
        p4_elapsedTime = str(jsonData["Spat"]["pedPhaseState"][3]["elapsedTime"])

        p5_currState = str(jsonData["Spat"]["pedPhaseState"][4]["currState"])
        p5_minEndTime = str(jsonData["Spat"]["pedPhaseState"][4]["minEndTime"])
        p5_maxEndTime = str(jsonData["Spat"]["pedPhaseState"][4]["maxEndTime"])
        p5_elapsedTime = str(jsonData["Spat"]["pedPhaseState"][4]["elapsedTime"])

        p6_currState = str(jsonData["Spat"]["pedPhaseState"][5]["currState"])
        p6_minEndTime = str(jsonData["Spat"]["pedPhaseState"][5]["minEndTime"])
        p6_maxEndTime = str(jsonData["Spat"]["pedPhaseState"][5]["maxEndTime"])
        p6_elapsedTime = str(jsonData["Spat"]["pedPhaseState"][5]["elapsedTime"])

        p7_currState = str(jsonData["Spat"]["pedPhaseState"][6]["currState"])
        p7_minEndTime = str(jsonData["Spat"]["pedPhaseState"][6]["minEndTime"])
        p7_maxEndTime = str(jsonData["Spat"]["pedPhaseState"][6]["maxEndTime"])
        p7_elapsedTime = str(jsonData["Spat"]["pedPhaseState"][6]["elapsedTime"])

        p8_currState = str(jsonData["Spat"]["pedPhaseState"][7]["currState"])
        p8_minEndTime = str(jsonData["Spat"]["pedPhaseState"][7]["minEndTime"])
        p8_maxEndTime = str(jsonData["Spat"]["pedPhaseState"][7]["maxEndTime"])
        p8_elapsedTime = str(jsonData["Spat"]["pedPhaseState"][7]["elapsedTime"])

        csv = (log_timestamp_verbose + "," + log_timestamp_posix + "," + timestamp_verbose + "," + timestamp_posix + "," + 
                regionalId + "," + intersectionId + "," + msgCnt + "," + moy + "," + msom + "," +

                v1_currState + "," + v1_minEndTime + "," + v1_maxEndTime + "," + v1_elapsedTime + "," +
                v2_currState + "," + v2_minEndTime + "," + v2_maxEndTime + "," + v2_elapsedTime + "," +
                v3_currState + "," + v3_minEndTime + "," + v3_maxEndTime + "," + v3_elapsedTime + "," +
                v4_currState + "," + v4_minEndTime + "," + v4_maxEndTime + "," + v4_elapsedTime + "," +
                v5_currState + "," + v5_minEndTime + "," + v5_maxEndTime + "," + v5_elapsedTime + "," +
                v6_currState + "," + v6_minEndTime + "," + v6_maxEndTime + "," + v6_elapsedTime + "," +
                v7_currState + "," + v7_minEndTime + "," + v7_maxEndTime + "," + v7_elapsedTime + "," +
                v8_currState + "," + v8_minEndTime + "," + v8_maxEndTime + "," + v8_elapsedTime + "," +

                p1_currState + "," + p1_minEndTime + "," + p1_maxEndTime + "," + p1_elapsedTime + "," +
                p2_currState + "," + p2_minEndTime + "," + p2_maxEndTime + "," + p2_elapsedTime + "," +
                p3_currState + "," + p3_minEndTime + "," + p3_maxEndTime + "," + p3_elapsedTime + "," +
                p4_currState + "," + p4_minEndTime + "," + p4_maxEndTime + "," + p4_elapsedTime + "," +
                p5_currState + "," + p5_minEndTime + "," + p5_maxEndTime + "," + p5_elapsedTime + "," +
                p6_currState + "," + p6_minEndTime + "," + p6_maxEndTime + "," + p6_elapsedTime + "," +
                p7_currState + "," + p7_minEndTime + "," + p7_maxEndTime + "," + p7_elapsedTime + "," +
                p8_currState + "," + p8_minEndTime + "," + p8_maxEndTime + "," + p8_elapsedTime + "\n")
        
        return csv

    def srmJsonToCsv(self, jsonData:json):
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

    def ssmJsonToCsv(self, jsonData:json):
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
                self.write_spat(receivedMsg)
            elif receivedMsg["MsgType"] == "SRM":
                self.write_srm(receivedMsg)
            elif receivedMsg["MsgType"] == "SSM":
                self.write_ssm(receivedMsg)
        except:
            print("Failed decoding of received message at: " + str(time.time()))    

if __name__ == "__main__":
    pass
