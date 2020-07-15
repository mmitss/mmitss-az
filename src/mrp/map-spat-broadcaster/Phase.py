'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  Phase.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision. Phase class does the following tasks:
    -> Stores information about phaseNo, currState, startTime, minEndTime, maxEndTime, and elapsedTime
    -> provides an API for creating a dictionary for each phase.
'''

class Phase:
    def __init__(self):
        self.phaseNo = 0
        self.currState = 0
        self.startTime = 0
        self.minEndTime = 0
        self.maxEndTime = 0
        self.elapsedTime = 0
    
    def asDict(self):
        return {"phaseNo":self.phaseNo, "currState":self.currState, "startTime":0, "minEndTime":self.minEndTime, "maxEndTime":self.maxEndTime, "elapsedTime":self.elapsedTime}