"""
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  cv-work-zone-controller.py
  Created by: Larry Head
  University of Arizona   
  College of Engineering

The purpose of the Position3D class is to represent a position on the earth in WGS-84 GPS coordinates 
by logitude, latitude, and elevation. 

Future enhancements incude:
    - computing the difference in position between two points 
    - checking bounds on position to ensure a proper translations
    - projection on to a local plane base on a reference GPS location


"""

import json

class Position3D:
    def __init__(self, latitude, longitude, elevation):
        self.latitude_DecimalDegree = latitude
        self.longitude_DecimalDegree = longitude
        self.elevation_Meter = elevation
        
    def Position3D2json(self):
        return json.dumps({"Position3D": { "latitude": self.latitude_DecimalDegree, 
                                "longitude": self.longitude_DecimalDegree, 
                                "elevation": self.elevation_Meter}}, sort_keys=False, indent = 4)

    def json2Position3D(self, json3Dposition):
        positionDict = json.loads(json3Dposition)
        self.latitude_DecimalDegree = positionDict['Position3D']['latitude']
        self.longitude_DecimalDegree = positionDict['Position3D']['longitude']
        self.elevation_DecimalDegree = positionDict['Position3D']['elevation']
        return self


     

if __name__ == '__main__':
    # simple self test code
    myPosition = Position3D(32.12345678, -112.12345678, 723)
    print(myPosition.latitude_DecimalDegree, myPosition.longitude_DecimalDegree, myPosition.elevation_Meter)
    
    #test Position3D to json
    myPosition_json = myPosition.Position3D2json()
    print(myPosition_json)
    
    #test jason 2 Position3D, remember to create an object first
    host = Position3D(0, 0, 0)
    host.json2Position3D(myPosition_json)
    print(host.latitude_DecimalDegree, host.longitude_DecimalDegree, host.elevation_DecimalDegree)
    
