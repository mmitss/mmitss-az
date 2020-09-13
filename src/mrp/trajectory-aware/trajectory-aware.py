'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    M_TrajectoryAware.py
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:
    This is a wrapper module that utilizes the other submodules to do following tasks:
    1. Receive messages over UDP socket.
    2. If the message is a BSM, then call appropriate methods of the submodules to process and store trajectories.
'''

# Set the path to read common packages
import os, sys
# currentDirectory = os.popen("pwd").read()
# packagesDirectory = currentDirectory[:currentDirectory.find("/src")]
# sys.path.append(packagesDirectory)

# Import system packages
import time, datetime
import json
import socket


# Import local packages:
# from src.common.Position3D import Position3D
# from src.common.BasicVehicle import BasicVehicle
from Position3D import Position3D
from BasicVehicle import BasicVehicle
from TrajectoryListManager import TrajectoryListManager
from MapEngineInterface import MapEngineInterface
from OnmapVehicle import OnmapVehicle
from Trajectory import Trajectory

def main():

    UPDATE_TRAJECTORY_AFTER_SEC = 0.1
    REMOVE_INACTIVE_TRAJECTORIES_AFTER_SEC = 5
    NOT_ON_MAP_COUNT_THRESHOLD = 10

    tlm = TrajectoryListManager(REMOVE_INACTIVE_TRAJECTORIES_AFTER_SEC)
    
    mapEngineInterface = MapEngineInterface()

    # Read a config file by creating an object of the time MapSpatBroadcasterConfig
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
    configFile.close()


    hostIp = config["HostIp"]
    trajectoryAwarePort = config["PortNumber"]["TrajectoryAware"]
    trajectoryAware_commInfo = (hostIp, trajectoryAwarePort)

    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(trajectoryAware_commInfo)

    dataCollectorIp = config["DataCollectorIP"]
    dataCollectorPort = config["PortNumber"]["DataCollector"]
    dataCollector_commInfo = (dataCollectorIp, dataCollectorPort)
    #print(dataCollector_commInfo)
    basicVehicle = BasicVehicle(0,0,Position3D(0,0,0),0,0,0,0,0)

    while True:
        data, address = s.recvfrom(5120)
        receivedMsg = json.loads(data.decode())
        if receivedMsg["MsgType"]=="BSM":
            
            # Create the BasicVehicle object from the received data.
            basicVehicle.json2BasicVehicle(receivedMsg)
            # Extract the timestamp from the received message.
            timestamp_posix = receivedMsg["Timestamp_posix"]
            
            # Check if the vehicle is on the map:
            locateVehicleOnMapStatus = mapEngineInterface.requestLocateVehicleOnMapStatus(basicVehicle)
            vehicleOnMap = locateVehicleOnMapStatus["Vehicle"]["OnMap"]
            print(str(time.time()) + "\t" + str(vehicleOnMap))
            # First check if the trajectory exists for this vehicle, by getting the trajectory index:
            trajectoryIndex = tlm.getTrajectoryIndex(basicVehicle.temporaryID)
            
            if trajectoryIndex != -1: # That is if the trajectory EXISTS for this vehicle:
                
                # Check when was the last BSM received from this vehicle:
                lastUpdated = tlm.trajectoryList[trajectoryIndex].timeOfLastUpdate
                if (time.time() - lastUpdated > UPDATE_TRAJECTORY_AFTER_SEC): # this is if the gap between current BSM and previous BSM from this vehicle is more than the UPDATE_TRAJECTORY_AFTER_SEC:
                    if vehicleOnMap:
                        notOnMapStatus = tlm.trajectoryList[trajectoryIndex].updateAndGetNotOnMapStatus(True)
                        onmapVehicle = OnmapVehicle(basicVehicle, locateVehicleOnMapStatus, timestamp_posix)
                        tlm.updateTrajectoryByIndex(trajectoryIndex, onmapVehicle)
                        currentDatapoint = tlm.trajectoryList[trajectoryIndex].getCurrentDatapointJson()
                        s.sendto(currentDatapoint.encode(),dataCollector_commInfo)
                        #print(currentDatapoint)
                    else: 
                        notOnMapStatus = tlm.trajectoryList[trajectoryIndex].updateAndGetNotOnMapStatus(False)
                        #print(notOnMapStatus)
                        if notOnMapStatus == NOT_ON_MAP_COUNT_THRESHOLD:
                            tlm.scheduleTrajectoryRemoval(basicVehicle.temporaryID)
            else:
                if vehicleOnMap: 
                    onmapVehicle = OnmapVehicle(basicVehicle, locateVehicleOnMapStatus, timestamp_posix)
                    trajectoryIndex = tlm.addTrajectory(onmapVehicle)
                    currentDatapoint = tlm.getTrajectoryByIndex(trajectoryIndex).getCurrentDatapointJson()
                    #print(currentDatapoint)
                    s.sendto(currentDatapoint.encode(),dataCollector_commInfo)

    s.close()

if __name__ == "__main__":
    main()
