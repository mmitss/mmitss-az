import socket
import json
import datetime
import time
import os
import sh

def initializeBsmLogFile(FileName):
    bsmFilename = "./../datalogs/BsmLog" + FileName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"
    bsmLogFile = open(bsmFilename, 'w')
    bsmLogFile.write("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,temporaryId,secMark,latitude,longitude,elevation,speed,heading,length,width\n")
    return bsmLogFile, bsmFilename

def initializeSrmLogFile(FileName):
    srmFilename = "./../datalogs/SrmLog" + FileName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"
    srmLogFile = open(srmFilename, 'w')
    srmLogFile.write(("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose" + "," 
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
    return srmLogFile, srmFilename

# Columns will be generated only for first five requests: After that the same pattern will be followed.
def initializeSsmLogFile(FileName): 
    ssmFilename = "./../datalogs/SsmLog" + FileName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"
    ssmLogFile = open(ssmFilename, 'w')
    ssmLogFile.write("log_timestamp_verbose" + "," 
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
                                                
    return ssmLogFile, ssmFilename

def initializeSpatLogFile(FileName):
    # Filename should be intersection name if running on intersection side
    currentSpatFilename = "./../datalogs/spatLog_" + FileName + "_" + ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now())) + ".csv"
    spatLogFile = open(currentSpatFilename, 'w')
    spatLogFile.write("log_timestamp_verbose,log_timestamp_posix,timestamp_verbose,timestamp_posix,regionalId,intersectionId,msgCount,moy,msom," + 
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
    return spatLogFile, currentSpatFilename

def receiveProcessAndStoreIntersectionDataLocally(socket, spatLogFile, surroundingBsmLogFile, srmLogFile, ssmLogFile):
    data, address = socket.recvfrom(4096)
    jsonData = json.loads(data.decode())
    if jsonData["MsgType"]=="SPaT": # then this message is a SPAT message
        spatLogFile.write(spatJsonToCsv(jsonData))
    elif jsonData["MsgType"]=="BSM": # then this message is a BSM message
        surroundingBsmLogFile.write(bsmJsonToCsv(jsonData))
    elif jsonData["MsgType"]=="SRM": # then this message is a SRM message
        srmLogFile.write(srmJsonToCsv(jsonData))
    elif jsonData["MsgType"]=="SSM": # then this message is a SSM message
        ssmLogFile.write(ssmJsonToCsv(jsonData))

def receiveProcessAndStoreVehicleDataLocally(socket, hostBsmDecoderPort, hostBsmLogFile, surroundingBsmLogFile, spatLogFile, srmLogFile, ssmLogFile):
    data, address = socket.recvfrom(4096)
    jsonData = json.loads(data.decode())
    if address[1]==hostBsmDecoderPort: # then this message is a BSM from the host vehicle
        hostBsmLogFile.write(bsmJsonToCsv(jsonData))

    elif jsonData["MsgType"]=="BSM": # then this message is a BSM from surrounding connected vehicle
        surroundingBsmLogFile.write(bsmJsonToCsv(jsonData))

    elif jsonData["MsgType"]=="SPaT": 
        spatLogFile.write(spatJsonToCsv(jsonData))

    elif jsonData["MsgType"]=="SSM":
        ssmLogFile.write(ssmJsonToCsv(jsonData))
        
    elif jsonData["msgType"]=="SRM":
        srmLogFile.write(srmJsonToCsv(jsonData))


def transferToCyVerseAndDeleteLocal(CyVerse_DirectoryPath, currentLocalFilename):
    (sh.icd(CyVerse_DirectoryPath)) # Go to correct CyVerse directory for storing SPAT data
    sh.iput(currentLocalFilename) # Upload all files of this intersection to current directory of CyVerse
    fileSize = os.path.getsize(currentLocalFilename)
    sh.rm(currentLocalFilename) # Remove the files of this intersection from local storage
    return fileSize

def convertSizeBytesToAppropriateUnits(sizeBytes):
    if sizeBytes < 1000:
        unit = "bytes"
        size = sizeBytes
    elif sizeBytes < 1000000:
        unit = "KB"
        size = sizeBytes/1000
    elif sizeBytes < 1000000000:
        unit = "MB"
        size = sizeBytes/1000000
    else:
        unit = "GB"
        size = sizeBytes/1000000000
    return size, unit

def generateEmail(transferSize, unit):
    message = """\
Subject: IAM Data Transfer

Hello,

{} {} of data was transferred from MCDOT-IAM-Server to CyVerse at {}.

This is an auto-generated email. Please do not reply.

Thanks.""".format(transferSize, unit, str(datetime.datetime.now()))
    return message

def bsmJsonToCsv(jsonData:json):
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
            + length + "," 
            + width + "\n")
    return csv

def srmJsonToCsv(jsonData:json):
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

def ssmJsonToCsv(jsonData:json):
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

def spatJsonToCsv(jsonData:json):
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

if __name__ == "__main__":
    pass
