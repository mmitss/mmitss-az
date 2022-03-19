'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  CoordinatedPhase.py  
  Created by: Debashis Das
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision. CoordinatedPhase class does the following tasks:
    - stores information about phaseNo, vehicleType, vehicleID, basicVehicleRole, ETA, coordinationSplit, priorityRequestType and requestUpdateTime
    - provides an API for creating a dictionary for each Coordinated phase (optional function)
'''


class CoordinatedPhase:
    def __init__(self):
        self.phaseNo = 0
        self.vehicleType = 0
        self.vehicleID = 0
        self.basicVehicleRole = 0
        self.ETA = 0.0
        self.ETA_Minute = 0  # minute of the year
        self.ETA_Second = 0  # DSecond (millisecond)
        self.coordinationSplit = 0.0
        self.priorityRequestType = 0
        self.requestUpdateTime = 0.0

    def asDict(self):
        return {"requestedPhase": self.phaseNo, "vehicleType": self.vehicleType, "vehicleID": self.vehicleID, "basicVehicleRole": self.basicVehicleRole, "ETA": self.ETA, "ETA_Minute": self.ETA_Minute, "ETA_Second": self.ETA_Second, "CoordinationSplit": self.coordinationSplit, "priorityRequestType": self.priorityRequestType, "requestUpdateTime": self.requestUpdateTime}
