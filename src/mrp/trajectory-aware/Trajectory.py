'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    Trajectory.py
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:
    1. This class stores the time varying series of OnmapVehicle information.
    2. The attributes of this class are subdivided into two subclasses: TimePersistentAttributes and TimeVarying Attributes.
    3. The list of timeVarying attributes is stored in the trajectory along with the time persistent attributes.
    4. The module also calculates the distance travelled by the vehicle along its current trajectory.
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

# Import 3rdparty packages
import haversine

# Import local packages
# from src.common.BasicVehicle import BasicVehicle
# from src.common.Position3D import Position3D

from BasicVehicle import BasicVehicle
from Position3D import Position3D
from OnmapVehicle import OnmapVehicle


class Trajectory():
    def __init__(self, onmapVehicle:OnmapVehicle, trajectoryId: int):
        """
        Create a new trajectory for new OnmapVehicle object.
        """

        self.onmapVehicle                        =        onmapVehicle

        self.timePersistentAttributes            =        TimePersistentAttributes(self.onmapVehicle)
        self.timeVaryingAttributesList           =        [TimeVaryingAttributes(self.onmapVehicle)]
        self.notOnMapCount                       =        [False]
        self.timeOfLastUpdate                    =        time.time()
        self.trajectoryId                        =        trajectoryId
        self.trajectorySignalGroup               =        self.onmapVehicle.signalGroup

    def updateTrajectory(self, onmapVehicle:OnmapVehicle):
        """
        Update the current object with informtation from new OnmapVehicle object.
        """
        self.onmapVehicle                        =        onmapVehicle
        distanceTravelledAlongTrajectory_Meter = self.getDistanceTravelledAlongTrajectory(self.onmapVehicle)
        timeVaryingAttributes = TimeVaryingAttributes(self.onmapVehicle)
        timeVaryingAttributes.distanceTravelledAlongTrajectory_Meter = distanceTravelledAlongTrajectory_Meter

        self.timeVaryingAttributesList = self.timeVaryingAttributesList + [timeVaryingAttributes]
        if timeVaryingAttributes.positionOnMap == "inbound":
            self.trajectorySignalGroup = timeVaryingAttributes.signalGroup

        self.timeOfLastUpdate = time.time()

    def getDistanceTravelledAlongTrajectory(self, onmapVehicle:OnmapVehicle):
        """
        Calculates and returns the distance travelled by the vehicle (in meters) along the current trajectory based on the previous point in the trajectory. External package 'haversine' is used to calculate the distance between current point and the previous point.
        """
        indexOfLastEntry = len(self.timeVaryingAttributesList)-1
        
        previous_latitude = self.timeVaryingAttributesList[indexOfLastEntry].latitude_DecimalDegree
        previous_longitude = self.timeVaryingAttributesList[indexOfLastEntry].longitude_DecimalDegree
        previousPoint = (previous_latitude, previous_longitude)
        
        current_latitude = onmapVehicle.latitude_DecimalDegree
        current_longitude = onmapVehicle.longitude_DecimalDegree
        currentPoint = (current_latitude, current_longitude)

        distanceTravelledFromPreviousPoint_meter = haversine.haversine(previousPoint, currentPoint, unit=haversine.Unit.METERS)
        distanceTravelledTillPreviousPoint_meter = self.timeVaryingAttributesList[indexOfLastEntry].distanceTravelledAlongTrajectory_Meter
        distanceTravelledAlongTrajectory_Meter = distanceTravelledTillPreviousPoint_meter + distanceTravelledFromPreviousPoint_meter
        
        return distanceTravelledAlongTrajectory_Meter

    def updateAndGetNotOnMapStatus(self, currentOnMapStatus:bool):
        """
        Set the onMap flag of the trajectory to False.
        """
        if currentOnMapStatus == True:
            self.notOnMapCount = [False]
        else:
            self.notOnMapCount = self.notOnMapCount + [True]
        
        return sum(self.notOnMapCount)


    def getTrajectoryJson(self):
        """
        Returns a JSON string containing time-persistent attributes of the trajectory and list containing the sequence of time-varying attributes of the trajectory.
        """

        timeVaryingAttributesListJson = []
        for datapoint in self.timeVaryingAttributesList:
            timeVaryingAttributesListJson = timeVaryingAttributesListJson + [json.loads(datapoint.getTimeVaryingAttributesJson())]

        trajectoryJson = dict({
                                "MsgType": "Trajectory",
                                "Trajectory":
                                {
                                    "TrajectoryId": self.trajectoryId,
                                    "TimePersistentAttributes":json.loads(self.timePersistentAttributes.getTimePersistentAttributesJson()),
                                    "TimeVaryingAttributes": timeVaryingAttributesListJson,
                                    "TimeOfLastUpdate": self.timeOfLastUpdate,
                                    "OnMap": self.onmapVehicle.onMap
                                }
        })

        return json.dumps(trajectoryJson)

    def getCurrentDatapointJson(self):
        """
        Returns a JSON string that contains the information about the latet datapoint. The information contains both, time-persistent and time-varying attributes of the point.
        """
        currentDatapointJson = json.loads(self.onmapVehicle.getOnmapVehicleJson())
        currentDatapointJson["MsgType"] = "TrajectoryDatapoint"
        currentDatapointJson["TrajectoryId"] = self.trajectoryId
        currentDatapointJson["TrajectorySignalGroup"] = self.trajectorySignalGroup
        lengthOfTrajectory = len(self.timeVaryingAttributesList)
        currentDatapointJson["OnmapVehicle"]["DistanceTravelledAlongTrajectory_Meter"] = self.timeVaryingAttributesList[lengthOfTrajectory-1].distanceTravelledAlongTrajectory_Meter
        currentDatapointJson["OnMapStatus"] = self.onmapVehicle.onMap

        return json.dumps(currentDatapointJson)

class TimePersistentAttributes():
    def __init__(self, onmapVehicle:OnmapVehicle):
        """
        Time persistent attributes are the ones which do not change over time, for a given vehicle. The attributes include: temporaryID, vehicleType, vehicleLength, vehicleWidth, ID of the approach from which the vehicle entered the map, and ID of the lane in which the vehicle entered the map. 
        """
        
        # BasicVehicle:
        self.temporaryID                         =       onmapVehicle.temporaryID
        self.vehicleType                         =       onmapVehicle.vehicleType
        self.length_cm                           =       onmapVehicle.length_cm
        self.width_cm                            =       onmapVehicle.width_cm

    def getTimePersistentAttributesJson(self):
        """
        Returns a JSON string containing the current values of time-persistent attributes of the trajectory. There is no message type for this JSON string.
        """
        timePersistentAttributesJson = dict({
                                                "temporaryID": self.temporaryID,
                                                "type": self.vehicleType,
                                                "length_cm": self.length_cm,
                                                "width_cm": self.width_cm,
                                            })

        return json.dumps(timePersistentAttributesJson)


class TimeVaryingAttributes():
    def __init__(self, onmapVehicle:OnmapVehicle):
        """
        Time-varying attributes are the ones which vary over time. These attributes contain: timestamp, position (latitude, longitude, elevation), speed, heading, position on map,  current approach ID, current lane ID, and if the vehicle is on inbound approach, then signal group associated with the lane on which the vehicle is on, in-queue status, distance between the vehicle's current position and the stopbar, estimated time to the stopbar, and distance travelled by the vehicle along this trajectory. 
        """
        # Time:
        self.timestamp_posix                     =       onmapVehicle.timestamp_posix
        
        # BasicVehicle:
        self.latitude_DecimalDegree              =       onmapVehicle.latitude_DecimalDegree
        self.longitude_DecimalDegree             =       onmapVehicle.longitude_DecimalDegree
        self.elevation_Meter                     =       onmapVehicle.elevation_Meter
        self.speed_MeterPerSecond                =       onmapVehicle.speed_MeterPerSecond
        self.heading_Degree                      =        onmapVehicle.heading_Degree
        
        # OnmapVehicle:
        self.positionOnMap                       =       onmapVehicle.positionOnMap
        self.inQueueStatus                       =       onmapVehicle.inQueueStatus
        self.approachId                          =       onmapVehicle.approachId
        self.laneId                              =       onmapVehicle.laneId
        self.signalGroup                         =       onmapVehicle.signalGroup
        self.distanceToStopBar_meter             =       onmapVehicle.distanceToStopBar_meter
        self.timeToStopBar_seconds               =       onmapVehicle.timeToStopBar_seconds
        self.local_x                             =       onmapVehicle.local_x
        self.local_y                             =       onmapVehicle.local_y
        self.local_z                             =       onmapVehicle.local_z


        self.distanceTravelledAlongTrajectory_Meter =       0.0

    def getTimeVaryingAttributesJson(self):
        """
        Returns a JSON string containing the current values of time-varying attributes of the trajectory. There is no MsgType for this JSON string
        """
        timeVaryingAttributesJson = dict({
                                            "timestamp_posix":self.timestamp_posix,
                                            "latitude_DecimalDegree":self.latitude_DecimalDegree,
                                            "longitude_DecimalDegree":self.longitude_DecimalDegree,
                                            "elevation_Meter":self.elevation_Meter,
                                            "speed_MeterPerSecond":self.speed_MeterPerSecond,
                                            "heading_Degree":self.heading_Degree,
                                            "positionOnMap":self.positionOnMap,
                                            "inQueueStatus":self.inQueueStatus,
                                            "approachId":self.approachId,
                                            "laneId":self.laneId,
                                            "signalGroup":self.signalGroup,
                                            "distanceToStopBar_meter":self.distanceToStopBar_meter,
                                            "timeToStopBar_seconds":self.timeToStopBar_seconds,
                                            "distanceTravelledAlongTrajectory_Meter":self.distanceTravelledAlongTrajectory_Meter,
                                            "local_x": self.local_x,
                                            "local_y": self.local_y,
                                            "local_z": self.local_z
                                        })
        return json.dumps(timeVaryingAttributesJson)

if __name__ == "__main__":

    from MapEngineInterface import MapEngineInterface

    mapEngineInterface = MapEngineInterface()

    # Non-critical BasicVehicle attributes
    temporaryId = 5822735
    secMark = 2600
    elevation = 587
    length = 400
    width = 300
    vehicleType = "transit"

    #################################################
    # Critical BasicVehicle attributes - Timestep 01
    timestamp_posix_1 = 302223140.0
    latitude_1 = 33.842683
    longitude_1 = -112.137056
    position_1 = Position3D(latitude_1, longitude_1, elevation)
    speed_1 = 10.5
    heading_1 = 90.0
    basicVehicle_1 = BasicVehicle(temporaryId, secMark, position_1, speed_1, heading_1, vehicleType, length, width)
    # Check if the test vehicle is on the map. (NOTE: MapEngine needs to be running)
    locateVehicleOnMapStatus_1 = mapEngineInterface.requestLocateVehicleOnMapStatus(basicVehicle_1)
    if locateVehicleOnMapStatus_1["Vehicle"]["OnMap"] == True:
        # Create a new onmapVehicle:
        onmapVehicle = OnmapVehicle(basicVehicle_1,locateVehicleOnMapStatus_1, timestamp_posix_1)
        trajectory = Trajectory(onmapVehicle,1)
        print(trajectory.getCurrentDatapointJson())

    # Critical BasicVehicle attributes - Timestep 02
    timestamp_posix_2 = 302223145.0
    latitude_2 = 33.842773
    longitude_2 = -112.136265
    position_2 = Position3D(latitude_2, longitude_2, elevation)
    speed_2 = 7.5
    heading_2 = 90.0
    basicVehicle_2 = BasicVehicle(temporaryId, secMark, position_2, speed_2, heading_2, vehicleType, length, width)
    # Check if the test vehicle is on the map. (NOTE: MapEngine needs to be running)
    locateVehicleOnMapStatus_2 = mapEngineInterface.requestLocateVehicleOnMapStatus(basicVehicle_2)
    if locateVehicleOnMapStatus_2["Vehicle"]["OnMap"] == True:
        # Create a new onmapVehicle:
        onmapVehicle = OnmapVehicle(basicVehicle_2,locateVehicleOnMapStatus_2, timestamp_posix_2)
        trajectory.updateTrajectory(onmapVehicle)
        print(trajectory.getCurrentDatapointJson())

    # Critical BasicVehicle attributes - Timestep 03
    timestamp_posix_3 = 302223150.0
    latitude_3 = 33.843455
    longitude_3 = -112.135159
    position_3 = Position3D(latitude_3, longitude_3, elevation)
    speed_3 = 12.5
    heading_3 = 0.0
    basicVehicle_3 = BasicVehicle(temporaryId, secMark, position_3, speed_3, heading_3, vehicleType, length, width)
    # Check if the test vehicle is on the map. (NOTE: MapEngine needs to be running)
    locateVehicleOnMapStatus_3 = mapEngineInterface.requestLocateVehicleOnMapStatus(basicVehicle_3)
    if locateVehicleOnMapStatus_3["Vehicle"]["OnMap"] == True:
        # Create a new onmapVehicle:
        onmapVehicle = OnmapVehicle(basicVehicle_3,locateVehicleOnMapStatus_3, timestamp_posix_3)
        trajectory.updateTrajectory(onmapVehicle)
        print(trajectory.getCurrentDatapointJson())

    # Deactivate the trajectory:
    trajectory.deactivateTrajectory()

    # Get trajectory in a JSON format:
    trajectoryJson = trajectory.getTrajectoryJson()
    #print(trajectoryJson)
