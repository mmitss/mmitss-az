'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  MapSpatBroadcasterConfig.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision. MapSpatBroadcasterConfig class does the following tasks:
    -> Read configuration data from a Json configuration file: controllerIP, selfIP, mapPayload, regionalID, and IntersectionID.
    -> Store the configuration data in own attributes.
    -> Provide API for getting the configuration data.
    -> Constructor requires the complete name (and path, if different from directory of this file) of the configuration file.
'''

import json

class MapSpatBroadcasterConfig:
    def __init__(self, nameOfConfigFile:str):
        f = open(nameOfConfigFile, 'r')
        configJson = json.load(f)
        self.intersectionID = configJson["MapSpatBroadcasterConfig"]["intersectionID"]
        self.regionalID = configJson["MapSpatBroadcasterConfig"]["regionalID"]
        self.mapPayload = configJson["MapSpatBroadcasterConfig"]["mapPayload"]
        self.controllerIP = configJson["MapSpatBroadcasterConfig"]["controllerIP"]
        self.mrpIP = configJson["MapSpatBroadcasterConfig"]["mrpIP"]

    def getIntersectionID(self):
        return self.intersectionID
    
    def getRegionalID(self):
        return self.regionalID

    def getMapPayload(self):
        return self.mapPayload

    def getControllerIP(self):
        return self.controllerIP

    def getMrpIP(self):
        return self.mrpIP