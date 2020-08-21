'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    TrajectoryListManager.py
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:
    The object of this class manages a list of trajectories.

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

# Import 3rdparty packages:
from apscheduler.schedulers.background import BackgroundScheduler
from apscheduler.triggers import date
import atexit


# Import local packages
# from src.common.BasicVehicle import BasicVehicle
# from src.common.Position3D import Position3D
from BasicVehicle import BasicVehicle
from Position3D import Position3D
from OnmapVehicle import OnmapVehicle
from Trajectory import Trajectory

class TrajectoryListManager:
    def __init__(self, inactiveTrajectoryHoldingPeriod_sec:int):
        self.backgroundScheduler = BackgroundScheduler() 
        self.backgroundScheduler.start()
        atexit.register(lambda: self.backgroundScheduler.shutdown(wait=False))
        self.trajectoryList = []
        self.inactiveTrajectoryHoldingPeriod_sec = inactiveTrajectoryHoldingPeriod_sec
        self.trajectoryId = 1

    def addTrajectory(self, onmapVehicle:OnmapVehicle):

        trajectory = Trajectory(onmapVehicle, self.trajectoryId)
        self.trajectoryList = self.trajectoryList + [trajectory]
        self.trajectoryId = self.trajectoryId + 1
        if self.trajectoryId > 65535:
            self.trajectoryId = 1
        trajectoryIndex = self.getTrajectoryIndex(onmapVehicle.temporaryID)

        return trajectoryIndex

    def getTrajectoryIndex(self, vehicleId:int):
        for index in range(0,len(self.trajectoryList)):
            if self.trajectoryList[index].timePersistentAttributes.temporaryID == vehicleId:
                return index
            else: 
                index = index + 1
        return -1
        
    def updateTrajectoryByIndex(self, trajectoryIndex: int, onmapVehicle:OnmapVehicle):
        self.trajectoryList[trajectoryIndex].updateTrajectory(onmapVehicle)


    def getTrajectoryByIndex(self, trajectoryIndex: int):
        return self.trajectoryList[trajectoryIndex]

    def removeTrajectoryByVehicleId(self, vehicleId: int):
        for index in range(0,len(self.trajectoryList)):
            if self.trajectoryList[index].timePersistentAttributes.temporaryID == vehicleId:
                self.trajectoryList.pop(index)
                # print("Removed trajectory of vehicle ID: " + str(vehicleId))
                break
            else: 
                index = index + 1

    def scheduleTrajectoryRemoval(self, vehicleId: int):
        trigger = date.DateTrigger(run_date=(datetime.datetime.now()+datetime.timedelta(seconds=self.inactiveTrajectoryHoldingPeriod_sec)))
        self.backgroundScheduler.add_job(self.removeTrajectoryByVehicleId, args = [vehicleId], trigger = trigger)
        # print("Scheduled removal of trajectory of vehicle ID: " + str(vehicleId))

if __name__ == "__main__":

    from MapEngineInterface import MapEngineInterface
    mapEngineInterface = MapEngineInterface()

    tlm = TrajectoryListManager(10000)

    timestamp_posix, temporaryId, secMark, elevation, length, width, vehicleType, latitude, longitude, position, speed, heading, basicVehicle, locateVehicleOnMapStatus = [0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0],[0,0,0,0]

    # BSM-01
    timestamp_posix[0] = 302223140.0
    temporaryId[0] = 5822735
    secMark[0] = 2600
    elevation[0] = 587
    length[0] = 400
    width[0] = 300
    vehicleType[0] = "transit"
    latitude[0] = 33.842811
    longitude[0] = -112.136100
    position[0] = Position3D(latitude[0], longitude[0], elevation[0])
    speed[0] = 10.5
    heading[0] = 90.0
    basicVehicle[0] = BasicVehicle(temporaryId[0], secMark[0], position[0], speed[0], heading[0], vehicleType[0], length[0], width[0])
    
    # BSM-02
    timestamp_posix[1] = 302223145.0
    temporaryId[1] = 5822735
    secMark[1] = 2600
    elevation[1] = 587
    length[1] = 400
    width[1] = 300
    vehicleType[1] = "transit"
    latitude[1] = 33.842807
    longitude[1] = -112.135813
    position[1] = Position3D(latitude[1], longitude[1], elevation[1])
    speed[1] = 10.5
    heading[1] = 90.0
    basicVehicle[1] = BasicVehicle(temporaryId[1], secMark[1], position[1], speed[1], heading[1], vehicleType[1], length[1], width[1])

    # BSM-03
    timestamp_posix[2] = 302223150.0
    temporaryId[2] = 5822736
    secMark[2] = 2600
    elevation[2] = 587
    length[2] = 400
    width[2] = 300
    vehicleType[2] = "transit"
    latitude[2] = 33.842684
    longitude[2] = -112.137057
    position[2] = Position3D(latitude[2], longitude[2], elevation[2])
    speed[2] = 10.5
    heading[2] = 90.0
    basicVehicle[2] = BasicVehicle(temporaryId[2], secMark[2], position[2], speed[2], heading[2], vehicleType[2], length[2], width[2])
    
    # BSM-04
    timestamp_posix[3] = 302223155.0
    temporaryId[3] = 5822736
    secMark[3] = 2600
    elevation[3] = 587
    length[3] = 400
    width[3] = 300
    vehicleType[3] = "transit"
    latitude[3] = 33.842684
    longitude[3] = -112.137057
    position[3] = Position3D(latitude[3], longitude[3], elevation[3])
    speed[3] = 10.5
    heading[3] = 90.0
    basicVehicle[3] = BasicVehicle(temporaryId[3], secMark[3], position[3], speed[3], heading[3], vehicleType[3], length[3], width[3])
    
    for i in range(0,4):
        # Check if the test vehicle is on the map. (NOTE: MapEngine needs to be running)
        timestamp_posix_curr = timestamp_posix[i]
        basicVehicle_curr = basicVehicle[i]
        locateVehicleOnMapStatus_curr = mapEngineInterface.requestLocateVehicleOnMapStatus(basicVehicle_curr) 
        # Create a new onmapVehicle:
        if locateVehicleOnMapStatus_curr["Vehicle"]["OnMap"]==True:
            trajectoryIndex = tlm.getTrajectoryIndex(basicVehicle_curr.temporaryID)
            if trajectoryIndex == -1:
                onmapVehicle_curr = OnmapVehicle(basicVehicle_curr, locateVehicleOnMapStatus_curr, timestamp_posix_curr)
                tlm.addTrajectory(onmapVehicle_curr)
            else:
                onmapVehicle_curr = OnmapVehicle(basicVehicle_curr, locateVehicleOnMapStatus_curr, timestamp_posix_curr)
                tlm.trajectoryList[trajectoryIndex].updateTrajectory(onmapVehicle_curr)
    
    # Make the first trajectory out of map:
    tlm.trajectoryList[0].deactivateTrajectory()

    currentTrajectories = tlm.getCurrentTrajectoriesJson()
    print(currentTrajectories)
    pass