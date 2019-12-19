import socket
import json
import datetime
import os

appLocation = 1 # 0 = vehicle; 1 = infrastructure
flushLogFileEvery = 6001 # Seconds

def main():
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = config["HostIp"]
    port = config["PortNumber"]["DataCollector"]
    dataCollectorAddress = (hostIp, port)
    s.bind(dataCollectorAddress)
    appStartTimestamp = ('{:%m%d%Y_%H%M%S}'.format(datetime.datetime.now()))

    if appLocation == 0: # Running on vehicle-side
        print ("Data collector running on vehicle-side coprocessor\nLogging remote BSMs from WirelessMsgDecoder & host BSMs from HostBsmDecoder \n")

        surroundingBsmLogFile = open(("surroundingBsmLog_" + appStartTimestamp + ".csv"), 'w')
        surroundingBsmLogFile.write("timestamp,temporaryId,secMark,latitude,longitude,elevation,speed,heading\n")
        surroundingBsmLogCounter = 1

        hostBsmLogFile = open(("hostBsmLog_" + appStartTimestamp + ".csv"), 'w')
        hostBsmLogFile.write("timestamp,temporaryId,secMark,latitude,longitude,elevation,speed,heading\n")
        hostBsmLogCounter = 1

        while True:
            data, address = s.recvfrom(4096)
            jsonData = json.loads(data.decode())
            if address[1] == config["PortNumber"]["OBUBSMReceiver"]: # then the BSM is from the host vehicle
                hostBsmLogFile.write(bsmJsonToCsv(jsonData))
                hostBsmLogCounter = hostBsmLogCounter+1
                if hostBsmLogCounter > flushLogFileEvery: # flush every 10 minutes
                    hostBsmLogFile.flush()
                    os.fsync()
                    hostBsmLogCounter = 1
            elif address[1] == config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]: # then the BSM is from the remote vehicle
                surroundingBsmLogFile.write(bsmJsonToCsv(jsonData))
                surroundingBsmLogCounter = surroundingBsmLogCounter+1
                if surroundingBsmLogCounter > flushLogFileEvery: # flush every 10 minutes
                    surroundingBsmLogFile.flush()
                    os.fsync()
                    surroundingBsmLogCounter = 1
        s.close()
        hostBsmLogFile.close()
        surroundingBsmLogFile.close()

    elif appLocation == 1: # Running on infrastructure-side
        print ("Data collector running on infrastructure-side coprocessor\nLogging SPAT from the controller & BSMs from WirelessMsgDecoder\n")
        spatLogFile = open(("spatLog_" + appStartTimestamp + ".csv"), 'w')
        spatLogFile.write("timestamp,regionalId,intersectionId,msgCount,moy,msom," + 
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
        spatLogCounter = 1

        surroundingBsmLogFile = open(("surroundingBsmLog_" + appStartTimestamp + ".csv"), 'w')
        surroundingBsmLogFile.write("timestamp,temporaryId,secMark,latitude,longitude,elevation,speed,heading\n")
        surroundingBsmLogCounter = 1

        while True:
            data, address = s.recvfrom(4096)
            jsonData = json.loads(data.decode())
            if address[1] == config["PortNumber"]["MapSPaTBroadcaster"]: # then this message is a SPAT message
                spatLogFile.write(spatJsonToCsv(jsonData))
                spatLogCounter = spatLogCounter+1
                if spatLogCounter > flushLogFileEvery: # flush every 1 minutes
                    spatLogFile.flush()
                    os.fsync()
                    spatLogCounter = 1
            elif address[1] == config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]: # then this message is a bsm message from 
                surroundingBsmLogFile.write(bsmJsonToCsv(jsonData))
                surroundingBsmLogCounter = surroundingBsmLogCounter+1
                if surroundingBsmLogCounter > flushLogFileEvery: # flush every 1 minutes
                    surroundingBsmLogFile.flush()
                    os.fsync()
                    surroundingBsmLogCounter = 1
        s.close()
        spatLogFile.close()
        surroundingBsmLogFile.close()

def bsmJsonToCsv(jsonData:json):
    timestamp = str(datetime.datetime.now())
    temporaryId = jsonData["BasicVehicle"]["temporaryID"]
    secMark = jsonData["BasicVehicle"]["secMark_Second"]
    latitude = jsonData["BasicVehicle"]["position"]["latitude_DecimalDegree"]
    longitude = jsonData["BasicVehicle"]["position"]["longitude_DecimalDegree"]
    elevation = jsonData["BasicVehicle"]["position"]["elevation_Meter"]
    speed = jsonData["BasicVehicle"]["speed_MeterPerSecond"]
    heading = jsonData["BasicVehicle"]["heading_Degree"]
    
    csv = timestamp + "," + temporaryId + "," + secMark + "," + latitude + "," + longitude + "," + elevation + "," + speed + "," + heading + "\n"
    return csv

def spatJsonToCsv(jsonData:json):
    timestamp = str(datetime.datetime.now())
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

    csv = (timestamp + "," + regionalId + "," + intersectionId + "," + msgCnt + "," + moy + "," + msom + "," +
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
    main()