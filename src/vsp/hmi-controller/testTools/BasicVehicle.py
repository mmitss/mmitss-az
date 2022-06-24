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

from Position3D import Position3D
import json


class BasicVehicle():
    def __init__(self, tempID, secMark, position, speed, heading, vehicleType):
        self.temporaryID = tempID
        self.secMark_Second = secMark
        # i'm sure there is a better way to deal with a Position Object
        self.longitude_DecimalDegree = position.longitude_DecimalDegree
        self.latitude_DecimalDegree = position.latitude_DecimalDegree
        self.elevation_Meter = position.elevation_Meter
        self.speed_MeterPerSecond = speed
        self.heading_Degree = heading
        self.vehicleType = vehicleType

    def BasicVehicle2json(self):
        bv_dict =   self.BasicVehicle2Dict()
        bv_json = json.dumps(bv_dict, sort_keys=True, indent = 4 )
        return bv_json

    def BasicVehicle2Dict(self):
        bv_dict =   {"BasicVehicle": {  "temporaryID": self.temporaryID, 
                                        "secMark_Second": self.secMark_Second, 
                                        "speed_MeterPerSecond": self.speed_MeterPerSecond, 
                                        "heading_Degree": self.heading_Degree, 
                                        "vehicleType": self.vehicleType,
                                        "position": { 
                                            "latitude_DecimalDegree": self.latitude_DecimalDegree, 
                                            "longitude_DecimalDegree": self.longitude_DecimalDegree, 
                                            "elevation_Meter": self.elevation_Meter}
                                    }
                    }
        return bv_dict

    def json2BasicVehicle(self, jsonBasicVehicle):
        vehicle_dict = json.loads(jsonBasicVehicle)
        self.temporaryID = int(vehicle_dict["BasicVehicle"]["temporaryID"])
        self.secMark_Second = int(vehicle_dict["BasicVehicle"]["secMark_Second"])
        self.heading_Degree = float(vehicle_dict["BasicVehicle"]["heading_Degree"])
        self.vehicleType = vehicle_dict["BasicVehicle"]["vehicleType"]
        self.speed_MeterPerSecond = float(vehicle_dict["BasicVehicle"]["speed_MeterPerSecond"])
        self.latitude_DecimalDegree = float(vehicle_dict["BasicVehicle"]["position"]['latitude_DecimalDegree'])
        self.longitude_DecimalDegree = float(vehicle_dict["BasicVehicle"]['position']['longitude_DecimalDegree'])
        self.elevation_Meter = float(vehicle_dict["BasicVehicle"]['position']['elevation_Meter'])
        return self


if __name__ == '__main__':
    from Position3D import Position3D
    # simple self test code
    host_position = Position3D(32.1234567, -112.1234567, 723)
    hostVehicle = BasicVehicle(1234, 0, host_position, 15.3, 92.8, "transit")
    print(hostVehicle.temporaryID, hostVehicle.secMark_Second, hostVehicle.longitude_DecimalDegree, 
        hostVehicle.latitude_DecimalDegree, hostVehicle.elevation_Meter, hostVehicle.speed_MeterPerSecond, hostVehicle.heading_Degree, 
        hostVehicle.vehicleType)
    basicVehicle_json = hostVehicle.BasicVehicle2json()
    print(basicVehicle_json)

    newPosition = Position3D(0,0,0)
    newHostVehicle = BasicVehicle(0,0,newPosition,0,0,"passengerVehicle")
    newHostVehicle = newHostVehicle.json2BasicVehicle(basicVehicle_json)
    print(newHostVehicle.temporaryID, newHostVehicle.secMark_Second, newHostVehicle.longitude_DecimalDegree, 
        newHostVehicle.latitude_DecimalDegree, newHostVehicle.elevation_Meter, newHostVehicle.speed_MeterPerSecond, newHostVehicle.heading_Degree, 
        newHostVehicle.vehicleType)


    #test to read a json basic vehicle file that Niraj created

    # read file
    with open('BasicVehicleTestOutput.json', 'r') as testfile:
        data=testfile.read()
    testPosition = Position3D(0,0,0)
    testHostVehicle = BasicVehicle(0,0,testPosition, 0, 0, 'nullVehicle')
    testHostVehicle = testHostVehicle.json2BasicVehicle(data)
    print(testHostVehicle.temporaryID, testHostVehicle.secMark_Second, testHostVehicle.longitude_DecimalDegree, 
        testHostVehicle.latitude_DecimalDegree, testHostVehicle.elevation_Meter, testHostVehicle.speed_MeterPerSecond, testHostVehicle.heading_Degree, 
        testHostVehicle.vehicleType)



        