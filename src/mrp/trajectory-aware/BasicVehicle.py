"""

**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  cv-work-zone-controller.py
  Created by: Niraj Altekar
  University of Arizona   
  College of Engineering

The purpose of the BasicVehicle class is to represent the key information about
a vehicle that is available from a Basic Safety Message in a connected vehicle 
system. 

Attributes:
    tempID -    the temporary ID that is assigned by each vehicle
    secMark -   a time stamp indicating when the data was sent to the system in units 
                of milliseconds within a minute
    position -  a (longitude, latitude, elevation) position represented in WGS-84 GPS
                coordinates [using the Position3D class]
    speed -     the speed of the vehicle in meters/second
    heading -   the vehicle heading in degrees from North using the WGS-84 GPS system
                where east is defined as a positive direction (e.g. 90 degrees is due east)


Future enhancements incude:
    - 

Larry Head
June 25, 2019

"""
import os, sys
currentDirectory = os.popen("pwd").read()
packagesDirectory = currentDirectory[:currentDirectory.find("/src")]
sys.path.append(packagesDirectory)

import json
import time, datetime

# from src.common.Position3D import Position3D
from Position3D import Position3D


class BasicVehicle(object):
    def __init__(self, tempID, secMark, position, speed, heading, vehicleType, length, width):
        self.temporaryID = tempID
        self.secMark_Second = secMark
        # i'm sure there is a better way to deal with a Position Object
        self.longitude_DecimalDegree = position.longitude_DecimalDegree
        self.latitude_DecimalDegree = position.latitude_DecimalDegree
        self.elevation_Meter = position.elevation_Meter
        self.speed_MeterPerSecond = speed
        self.heading_Degree = heading
        self.vehicleType = vehicleType
        self.length_cm = length
        self.width_cm = width

    def BasicVehicle2json(self):
        bv_dict =   {
                        "MsgType": "BSM",
                        "Timestamp_verbose":str(datetime.datetime.now()),
                        "Timestamp_posix":time.time(),
                        "BasicVehicle": {  
                                            "temporaryID": self.temporaryID, 
                                            "secMark_Second": self.secMark_Second, 
                                            "speed_MeterPerSecond": self.speed_MeterPerSecond, 
                                            "heading_Degree": self.heading_Degree, 
                                            "type": self.vehicleType,
                                            "position": 
                                            { 
                                                "latitude_DecimalDegree": self.latitude_DecimalDegree, 
                                                "longitude_DecimalDegree": self.longitude_DecimalDegree, 
                                                "elevation_Meter": self.elevation_Meter
                                            },
                                            "size":
                                            {
                                                "length_cm": self.length_cm,
                                                "width_cm": self.width_cm
                                            }
                                    }
                    }
        return json.dumps(bv_dict, sort_keys=True, indent = 4 )

    def json2BasicVehicle(self, jsonBasicVehicle):
        vehicle_dict = (jsonBasicVehicle)
        self.temporaryID = int(vehicle_dict["BasicVehicle"]["temporaryID"])
        self.secMark_Second = float(vehicle_dict["BasicVehicle"]["secMark_Second"])
        self.heading_Degree = float(vehicle_dict["BasicVehicle"]["heading_Degree"])
        self.vehicleType = vehicle_dict["BasicVehicle"]["type"]
        self.speed_MeterPerSecond = float(vehicle_dict["BasicVehicle"]["speed_MeterPerSecond"])
        self.latitude_DecimalDegree = float(vehicle_dict["BasicVehicle"]["position"]['latitude_DecimalDegree'])
        self.longitude_DecimalDegree = float(vehicle_dict["BasicVehicle"]['position']['longitude_DecimalDegree'])
        self.elevation_Meter = float(vehicle_dict["BasicVehicle"]['position']['elevation_Meter'])
        self.length_cm = int(vehicle_dict["BasicVehicle"]['size']['length_cm'])
        self.width_cm = int(vehicle_dict["BasicVehicle"]['size']['width_cm'])


if __name__ == '__main__':

    
    from src.common.Position3D import Position3D
    # simple self test code
    host_position = Position3D(32.1234567, -112.1234567, 723)
    hostVehicle = BasicVehicle(1234, 0, host_position, 15.3, 92.8, "transit", 300,200)
    print(hostVehicle.temporaryID, hostVehicle.secMark_Second, hostVehicle.longitude_DecimalDegree, 
        hostVehicle.latitude_DecimalDegree, hostVehicle.elevation_Meter, hostVehicle.speed_MeterPerSecond, hostVehicle.heading_Degree, 
        hostVehicle.vehicleType, hostVehicle.length_cm, hostVehicle.width_cm)
    basicVehicle_json = hostVehicle.BasicVehicle2json()
    print(basicVehicle_json)

    newPosition = Position3D(0,0,0)
    newHostVehicle = BasicVehicle(0,0,newPosition,0,0,"passengerVehicle", 300, 200)
    newHostVehicle.json2BasicVehicle(basicVehicle_json)
    print(newHostVehicle.temporaryID, newHostVehicle.secMark_Second, newHostVehicle.longitude_DecimalDegree, 
        newHostVehicle.latitude_DecimalDegree, newHostVehicle.elevation_Meter, newHostVehicle.speed_MeterPerSecond, newHostVehicle.heading_Degree, 
        newHostVehicle.vehicleType)


    #test to read a json basic vehicle file that Niraj created

    # read file
    with open('BasicVehicleTestOutput.json', 'r') as testfile:
        data=testfile.read()
    testPosition = Position3D(0,0,0)
    testHostVehicle = BasicVehicle(0,0,testPosition, 0, 0, 'nullVehicle',0,0)
    testHostVehicle.json2BasicVehicle(data)
    print(testHostVehicle.temporaryID, testHostVehicle.secMark_Second, testHostVehicle.longitude_DecimalDegree, 
        testHostVehicle.latitude_DecimalDegree, testHostVehicle.elevation_Meter, testHostVehicle.speed_MeterPerSecond, testHostVehicle.heading_Degree, 
        testHostVehicle.vehicleType)



        