'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  MsgSenderConfig.py.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is an initial revision. MsgSenderConfig class performs following tasks:
    -> Reads configuration items from the config file: Self IPaddress and IP address of sourceDsrcDevice
    -> Name of the config file needs to be passed as a parameter while creating a new object (constructor)
'''

import json
class MsgSenderConfig:
    def __init__(self, msgSenderConfigFile:str):
        f = open(msgSenderConfigFile)
        config = json.load(f)
        self.coProcessorIP = config["MsgSenderConfig"]["coProcessorIP"]
        self.sourceDsrcDeviceIP = config["MsgSenderConfig"]["sourceDsrcDeviceIP"]
    def getCoProcessorIP(self):
        return self.coProcessorIP
    def getSourceDsrcDeviceIP(self):
        return self.sourceDsrcDeviceIP
