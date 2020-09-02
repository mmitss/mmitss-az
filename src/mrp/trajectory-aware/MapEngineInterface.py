'''
**********************************************************************************

© 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
    granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    MapEngineInterface.py
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:
    This class facilitates the interaction between the MapEngine (C++ component) and other Python components. The MapEngine needs to be running to utilize this class.
'''

# Set the path to read common packages
import os, sys
# currentDirectory = os.popen("pwd").read()
# packagesDirectory = currentDirectory[:currentDirectory.find("/src")]
# sys.path.append(packagesDirectory)

# Import system packages
import json
import socket
import haversine

# Import local packages
# from src.common.BasicVehicle import BasicVehicle
# from src.common.geoCoord import GeoCoord
# from src.common.Position3D import Position3D
from BasicVehicle import BasicVehicle
from geoCoord import GeoCoord
from Position3D import Position3D

class MapEngineInterface:
    def __init__(self):
        """
        Facilitates the interaction between Python components and the MapEngine that is built in C++.
        """
        # Other attributes:
        self.mapEngine_commInfo, self.mapEngineInterface_commInfo, self.wirelessRange_Meter, self.referenceLocation = self.readConfig()
        self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  
        self.s.bind(self.mapEngineInterface_commInfo)

        self.geo = GeoCoord(self.referenceLocation)

        # Initialize geocoord object for calculation of local coordinates:


    def readConfig(self):
        """
        Reads the mmitss-phase3-master-config file and extracts the network communication information. The function returns two tuples: one containing the IP address and port of that can be used by this module to communicate with other componenet, and the second tuple contains the IP address and port of the MapEngine.
        """
        
        configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
        config = (json.load(configFile))
        configFile.close()
        hostIp = config["HostIp"]
        interfacePort = config["PortNumber"]["TrajectoryAware_MapEngineInterface"]
        mapEngineInterface_commInfo = (hostIp, interfacePort)

        mapEnginePort = config["PortNumber"]["MapEngine"]
        mapEngine_commInfo = (hostIp, mapEnginePort)

        wirelessRange_Meter = 300

        referenceLatitude_DecimalDegree = config["IntersectionReferencePoint"]["Latitude_DecimalDegree"]
        referenceLongitude_DecimalDegree = config["IntersectionReferencePoint"]["Longitude_DecimalDegree"]
        referenceElevation_Meter = config["IntersectionReferencePoint"]["Elevation_Meter"]

        referenceLocation = Position3D(referenceLatitude_DecimalDegree, referenceLongitude_DecimalDegree, referenceElevation_Meter)

        return mapEngine_commInfo, mapEngineInterface_commInfo, wirelessRange_Meter, referenceLocation

    def getLocalCoordinates(self, latitude_DecimalDegree, longitude_DecimalDegree, elevation_Meter):

        # Convert the GPS coordinates into earth-centered-earth-fixed coordinates
        ex, ey, ez = self.geo.lla2ecef(Position3D(latitude_DecimalDegree, longitude_DecimalDegree, elevation_Meter))

        # Now convert the earth-centered-earth-fixed coordinates into local coordinates:
        x, y, z = self.geo.ecef2local(ex, ey, ez)

        return x,y,z
    
    
    def requestLocateVehicleOnMapStatus(self, basicVehicle:BasicVehicle):
        """
        Sends the formulated JSON string of LocateVehicleMapRequest to the MapEngine, receives the respose from the MapEngine (LocateVehicleOnMapStatus), and returns a dictionary containing the locateVehicleOnMapStatus.
        """

        def formulateLocateVehicleOnMapRequest(basicVehicle:BasicVehicle):
            """
            From the provided BasicVehicle object, formulates a request (for locating the vehicle on the map) that can be accepted by the MapEngine. The JSOn string of the request is returned from the function.
            """
            locateVehicleOnMapRequest = {
                                            "MsgType":"LocateVehicleOnMapRequest",
                                            "Vehicle":
                                            {
                                                "Latitude" : basicVehicle.latitude_DecimalDegree,
                                                "Longitude" : basicVehicle.longitude_DecimalDegree,
                                                "Elevation" : basicVehicle.elevation_Meter,
                                                "Speed" : basicVehicle.speed_MeterPerSecond,
                                                "Heading" : basicVehicle.heading_Degree
                                            }
                                        }
      
            
            locateVehicleOnMapRequest = json.dumps(locateVehicleOnMapRequest)
            return locateVehicleOnMapRequest

        request = formulateLocateVehicleOnMapRequest(basicVehicle)

        self.s.sendto(request.encode(),self.mapEngine_commInfo)
        data, addr = self.s.recvfrom(2048)    
        locateVehicleOnMapStatus = json.loads(data.decode())

        local_x, local_y, local_z = self.getLocalCoordinates(basicVehicle.latitude_DecimalDegree, 
                                                                basicVehicle.longitude_DecimalDegree, 
                                                                basicVehicle.elevation_Meter)
       
        locateVehicleOnMapStatus["Vehicle"]["LocalCoordinates"] = {
            "x": local_x,
            "y": local_y,
            "z": local_z
        }

        return locateVehicleOnMapStatus

if __name__ == "__main__":

    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
    configFile.close()

    hostIp = config["HostIp"]
    solverPort = config["PortNumber"]["TrajectoryAware"]
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((hostIp,solverPort))

    mapEnginePort = config["PortNumber"]["MapEngine"]
    mapEngine_commInfo = (hostIp, mapEnginePort)

