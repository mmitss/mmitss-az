'''
**********************************************************************************

© 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
    granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

OnmapVehicle.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

Description:
------------
1. This class is inherited from the BasicVehicle class. The responsibility of this class is to store the information of a vehicle that is found to be on map.
2. In addition to the attributes of the BasicVehicle class, this class stores the following information:
    -> Current information:
        -> Current approach
        -> Current lane
        -> Current position on map (Inbound, Outbound, InsideIntersectionBox)

    -> If the vehicle is an inbound vehicle:
        -> Signal group
        -> Distance to the stopbar (meters)
        -> Time to the stop bar (seconds)
        -> In-queue status flag

    -> Historical information:
        -> Starting approach
        -> Starting lane
        -> Latitude, longitude, and elevation at previous timestep
'''

# Set the path to read common packages
import os, sys
# currentDirectory = os.popen("pwd").read()
# packagesDirectory = currentDirectory[:currentDirectory.find("/src")]
# sys.path.append(packagesDirectory)

# Import system packages
import json

# Import local packages
# from src.common.BasicVehicle import BasicVehicle
# from src.common.Position3D import Position3D
from BasicVehicle import BasicVehicle
from Position3D import Position3D
from MapEngineInterface import MapEngineInterface

class OnmapVehicle(BasicVehicle):
    def __init__(self, basicVehicle:BasicVehicle, locateVehicleOnMapStatus:dict, timestamp_posix:float):
        """
        Stores the information about BasicVehicle that is on the map at a particular timestamp. The locateVehicleOnMapStatus can be obtained using MAPEngineInterface module. This class is inherited from the BasicVehicle class.
        """
        # Timestamp
        self.timestamp_posix                     =      timestamp_posix

        # Basic attributes:
        self.temporaryID                         =      basicVehicle.temporaryID
        self.vehicleType                         =      basicVehicle.vehicleType
        self.secMark_Second                      =      basicVehicle.secMark_Second
        self.length_cm                           =      basicVehicle.length_cm
        self.width_cm                            =      basicVehicle.width_cm
        self.latitude_DecimalDegree              =      basicVehicle.latitude_DecimalDegree
        self.longitude_DecimalDegree             =      basicVehicle.longitude_DecimalDegree
        self.elevation_Meter                     =      basicVehicle.elevation_Meter
        self.heading_Degree                      =      basicVehicle.heading_Degree
        self.speed_MeterPerSecond                =      basicVehicle.speed_MeterPerSecond
        
        # Onmap attribuets:
        self.onMap                               =      locateVehicleOnMapStatus["Vehicle"]["OnMap"]
        self.positionOnMap                       =      locateVehicleOnMapStatus["Vehicle"]["PositionOnMap"]
        self.inQueueStatus                       =      locateVehicleOnMapStatus["Vehicle"]["InboundStatus"]["InQueueStatus"]
        self.approachId                          =      locateVehicleOnMapStatus["Vehicle"]["InboundStatus"]["ApproachId"]
        self.laneId                              =      locateVehicleOnMapStatus["Vehicle"]["InboundStatus"]["LaneId"]
        self.signalGroup                         =      locateVehicleOnMapStatus["Vehicle"]["InboundStatus"]["SignalGroup"]
        self.distanceToStopBar_meter             =      locateVehicleOnMapStatus["Vehicle"]["InboundStatus"]["DistanceToStopBar_Meter"]
        self.timeToStopBar_seconds               =      locateVehicleOnMapStatus["Vehicle"]["InboundStatus"]["TimeToStopBar_Seconds"]
        self.local_x                             =      locateVehicleOnMapStatus["Vehicle"]["LocalCoordinates"]["x"]
        self.local_y                             =      locateVehicleOnMapStatus["Vehicle"]["LocalCoordinates"]["y"]
        self.local_z                             =      locateVehicleOnMapStatus["Vehicle"]["LocalCoordinates"]["z"]

    def getOnmapVehicleJson(self):        
        """
        Returns a JSON string containing the structured information about the OnmapVehicle. The information is divided into two parts: BasicVehicle and OnmapVehicle. The BasicVehicle section stores the information coming from the BasicVehicle object, and the OnmapVehicle section stores the information about the vehicle's position and status while it is on the map. The OnmapVehicle information mainly comes from the LocateVehicleOnMapStatus message that is received from the MapEngineInterface.
        """
        onmapVehicleJson = dict(
                                    {
                                        "MsgType": "OnmapVehicleJson",
                                        "Timestamp_posix": self.timestamp_posix,
                                        "BasicVehicle": json.loads(self.BasicVehicle2json())["BasicVehicle"],
                                        "OnmapVehicle": 
                                        {
                                            "OnMap": self.onMap,
                                            "PositionOnMap": self.positionOnMap,
                                            "ApproachId":self.approachId,                          
                                            "LaneId": self.laneId,                   
                                            "InboundStatus":
                                            {
                                                "InQueueStatus": self.inQueueStatus,                       
                                                "SignalGroup": self.signalGroup,                         
                                                "DistanceToStopbar_Meter": self.distanceToStopBar_meter,           
                                                "TimeToStopbar_Second": self.timeToStopBar_seconds           
                                            },
                                            "LocalCoordinates": 
                                            {
                                                "x": self.local_x,                    
                                                "y": self.local_y,                            
                                                "z": self.local_z                
                                            },
                                        }
                                    }
                                )
        return json.dumps(onmapVehicleJson)
        
if __name__ == "__main__":

    import time

    mapEngineInterface = MapEngineInterface()

    timestamp_posix_1 = time.time()

    # Non-critical BasicVehicle attributes
    temporaryId = 5822735
    secMark = 2600
    elevation = 539
    length = 400
    width = 300
    vehicleType = "transit"

    #################################################
    # Critical BasicVehicle attributes - Timestep 01
    latitude_1 =    33.843127
    longitude_1 = -112.135191
    position_1 = Position3D(latitude_1, longitude_1, elevation)
    speed_1 = 10.5
    heading_1 = 90.0
    basicVehicle_1 = BasicVehicle(temporaryId, secMark, position_1, speed_1, heading_1, vehicleType, length, width)
    # Check if the test vehicle is on the map. (NOTE: MapEngine needs to be running)
    locateVehicleOnMapStatus_1 = mapEngineInterface.requestLocateVehicleOnMapStatus(basicVehicle_1)

    onmapVehicle = OnmapVehicle(basicVehicle_1, locateVehicleOnMapStatus_1, timestamp_posix_1)
   

    # Get onmapVehicle json string:
    onmapVehicleJson = onmapVehicle.getOnmapVehicleJson()
    print(onmapVehicleJson)