'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  UtcHelper.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Description:
  UtcHelper is a helper class that provides an API to convert (time from now) to a 
  timemark in UTC system. 

'''

import math
import json
from datetime import datetime

MILLISEC_IN_HOUR = 36000
SEC_IN_MINUTE = 60
MILLISEC_IN_SECOND = 10
MICROSEC_IN_MILLISEC = 100000

class UtcHelper:
    def __init__(self):
        pass

    def get_utcTimemark_from_milliSecFromNow(self, milliSecFromNow:int):
        """
        takes millisecond from now as an input and returns a corresponding
        UTC timemark
        """

        # Get current UTC time
        currentUtcTime = datetime.utcnow()

        # Extract current millisecond in the current UTC hour
        currentUtcMillisecond = ((currentUtcTime.minute*SEC_IN_MINUTE + currentUtcTime.second)*MILLISEC_IN_SECOND + 
                                    math.floor(currentUtcTime.microsecond/MICROSEC_IN_MILLISEC))
        
        # Compute the initial UTC timemark
        utcTimemark = milliSecFromNow + currentUtcMillisecond

        # If the utcTimemark is in the next UTC hour
        if (utcTimemark) >= MILLISEC_IN_HOUR: 
            # Return return the UTC timemark in the next hour by 
            # subtracting total milliseconds in an hour (36000)
            return (utcTimemark-MILLISEC_IN_HOUR)

        else: 
            # Return the computed UTC timemark
            return utcTimemark

    def get_milliSecFromNow_from_utcTimemark(self, utcTimemark:int):
        """
        takes a UTC timemark as an input and returns milliseconds from now
        """
        # Get current UTC time
        currentUtcTime = datetime.utcnow()
        
        # Extract the current millisecond in the current UTC hour
        currentUtcMillisecond = ((currentUtcTime.minute*SEC_IN_MINUTE + currentUtcTime.second)*MILLISEC_IN_SECOND + 
                                    math.floor(currentUtcTime.microsecond/MICROSEC_IN_MILLISEC))
        
        # If current millisecond in the UTC hour is less than the input utcTimemark
        if currentUtcMillisecond <= utcTimemark:
            # Then the input UTC timemark is in the current UTC hour
            # Therefore, millisecond from now is just the difference between
            # current UTC millisecond and the input UTC timemark
            milliSecFromNow = utcTimemark - currentUtcMillisecond
        else:
            # Then the input UTC timemark is in the next UTC hour

            # Compute milliseconds left till the beginning of the next UTC hour
            milliSecUntilNextHour = MILLISEC_IN_HOUR - currentUtcMillisecond
            # milliseconds from now is the addition of the milliseconds 
            # left until the beginning of the next UTC hour and 
            # milliseconds in the next time hour (that is input utcTimemark)
            milliSecFromNow = milliSecUntilNextHour + utcTimemark

        return milliSecFromNow

    def modify_spat_json_to_utc_timemark(self, spatJsonString:str):
        """
        takes a SPaT JSON string as in input, modifies all min and max end times 
        to UTC time marks, and returns the modified SPaT JSON string
        """
        # Create a JSON object from the input string
        spatJson = json.loads(spatJsonString)
        
        # Get original parameters for all vehicle and pedestrian phases 
        phaseStates = spatJson["Spat"]["phaseState"]
        pedPhaseStates = spatJson["Spat"]["pedPhaseState"]
        
        # Create a copy of the SPaT JSON object to store the modified parameters
        modifiedSpatJson = spatJson        

        # For each vehicle state, modify the min and max end times to timemarks in current 
        # or the next UTC hour
        for phaseState in phaseStates:
            phaseState["minEndTime"] = self.get_utcTimemark_from_milliSecFromNow(phaseState["minEndTime"])
            phaseState["maxEndTime"] = self.get_utcTimemark_from_milliSecFromNow(phaseState["maxEndTime"])
        
        # For each pedestrian state, modify the min and max end times to timemarks in current 
        # or the next UTC hour
        for pedPhaseState in pedPhaseStates:
            pedPhaseState["minEndTime"] = self.get_utcTimemark_from_milliSecFromNow(pedPhaseState["minEndTime"])
            pedPhaseState["maxEndTime"] = self.get_utcTimemark_from_milliSecFromNow(pedPhaseState["maxEndTime"])

        # Plug in the modified parameters in the corresponding parameters of modified SPaT JSON object
        modifiedSpatJson["Spat"]["phaseState"] = phaseStates
        modifiedSpatJson["Spat"]["pedPhaseState"] = pedPhaseStates

        # Create a modified JSON string from the modified JSON object and return it
        return json.dumps(modifiedSpatJson)


    def modify_spat_json_to_milliSecFromNow(self, spatJsonString:str):
        """
        takes a SPaT JSON string as in input, modifies all min and max end times 
        to milliseconds from now, and returns the modified SPaT JSON string
        """
        # Create a JSON object from the input string
        spatJson = json.loads(spatJsonString)

        # Get original parameters for all vehicle and pedestrian phases 
        phaseStates = spatJson["Spat"]["phaseState"]
        pedPhaseStates = spatJson["Spat"]["pedPhaseState"]
        
        # Create a copy of the SPaT JSON object to store the modified parameters
        modifiedSpatJson = spatJson

        # For each vehicle phase, modify the min and max end times to milliseconds from now
        for phaseState in phaseStates:
            phaseState["minEndTime"] = self.get_milliSecFromNow_from_utcTimemark(phaseState["minEndTime"])
            phaseState["maxEndTime"] = self.get_milliSecFromNow_from_utcTimemark(phaseState["maxEndTime"])
        
        # For each vehicle phase, modify the min and max end times to milliseconds from now
        for pedPhaseState in pedPhaseStates:
            pedPhaseState["minEndTime"] = self.get_milliSecFromNow_from_utcTimemark(pedPhaseState["minEndTime"])
            pedPhaseState["maxEndTime"] = self.get_milliSecFromNow_from_utcTimemark(pedPhaseState["maxEndTime"])
        
        # Plug in the modified parameters in the corresponding parameters of modified SPaT JSON object
        modifiedSpatJson["Spat"]["phaseState"] = phaseStates
        modifiedSpatJson["Spat"]["pedPhaseState"] = pedPhaseStates

        # Create a modified JSON string from the modified JSON object and return it
        return json.dumps(modifiedSpatJson)


if __name__=="__main__":
    import time
    TEST_CONVERTER_FUNCTIONS = False
    TEST_SPAT_CONVERSION = True

    utcHelper = UtcHelper() 

    if TEST_CONVERTER_FUNCTIONS==False and TEST_SPAT_CONVERSION==False: print("Nothing to test!")
    
    if TEST_CONVERTER_FUNCTIONS:
        while True:
            milliSecFromNow = 35000            
            utcTimemark = utcHelper.get_utcTimemark_from_milliSecFromNow(milliSecFromNow)
            computedMilliSecFromNow = utcHelper.get_milliSecFromNow_from_utcTimemark(utcTimemark)
            print("UTC Time:", utcTimemark, "Millisec From Now:",computedMilliSecFromNow)
            time.sleep(0.1)
    
    if TEST_SPAT_CONVERSION:
        with open("Spat.json", 'r') as spatFile:
            spatJson = json.load(spatFile)
            spatJsonString = json.dumps(spatJson)
            
            startTime = time.time()
            modifiedSpatJsonString = utcHelper.modify_spat_json_to_utc_timemark(spatJsonString)
            endTime = time.time()
            
            print(modifiedSpatJsonString)
            print("\nTime taken to modify SPaT from milliseconds from now to UTC timemark = {} seconds\n".format(endTime-startTime))

            startTime = time.time()
            remodifiedSpatJsonString = utcHelper.modify_spat_json_to_milliSecFromNow(modifiedSpatJsonString)
            endTime = time.time()

            print(remodifiedSpatJsonString)
            print("\nTime taken to modify SPaT from UTC timemark to milliseconds from now = {} seconds\n".format(endTime-startTime))
