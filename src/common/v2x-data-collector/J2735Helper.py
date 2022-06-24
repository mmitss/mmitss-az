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

DECISEC_IN_HOUR = 36000
SEC_IN_MINUTE = 60
DECISEC_IN_SECOND = 10
MICROSEC_IN_DECISEC = 100000

class J2735Helper:
    def __init__(self):
        pass

    def get_utcTimemark_from_deciSecFromNow(self, deciSecFromNow:int):
        """
        takes decisecond from now as an input and returns a corresponding
        UTC timemark
        """

        # Get current UTC time
        currentUtcTime = datetime.utcnow()

        # Extract current decisecond in the current UTC hour
        currentUtcDecisecond = ((currentUtcTime.minute*SEC_IN_MINUTE + currentUtcTime.second)*DECISEC_IN_SECOND + 
                                    math.floor(currentUtcTime.microsecond/MICROSEC_IN_DECISEC))
        
        # Compute the initial UTC timemark
        utcTimemark = deciSecFromNow + currentUtcDecisecond

        # If the utcTimemark is in the next UTC hour
        if (utcTimemark) >= DECISEC_IN_HOUR: 
            # Return  the UTC timemark in the next hour by 
            # subtracting total deciseconds in an hour (36000)
            return (utcTimemark-DECISEC_IN_HOUR)

        else: 
            # Return the computed UTC timemark
            return utcTimemark

    def get_deciSecFromNow_from_utcTimemark(self, utcTimemark:int):
        """
        takes a UTC timemark as an input and returns deciseconds from now
        """
        # Get current UTC time
        currentUtcTime = datetime.utcnow()
        
        # Extract the current decisecond in the current UTC hour
        currentUtcDecisecond = ((currentUtcTime.minute*SEC_IN_MINUTE + currentUtcTime.second)*DECISEC_IN_SECOND + 
                                    math.floor(currentUtcTime.microsecond/MICROSEC_IN_DECISEC))
        
        # If current decisecond in the UTC hour is less than the input utcTimemark
        if currentUtcDecisecond <= utcTimemark:
            # Then the input UTC timemark is in the current UTC hour
            # Therefore, decisecond from now is just the difference between
            # current UTC decisecond and the input UTC timemark
            deciSecFromNow = utcTimemark - currentUtcDecisecond
        else:
            # Then the input UTC timemark is in the next UTC hour

            # Compute deciseconds left till the beginning of the next UTC hour
            deciSecUntilNextHour = DECISEC_IN_HOUR - currentUtcDecisecond
            # deciseconds from now is the addition of the deciseconds 
            # left until the beginning of the next UTC hour and 
            # deciseconds in the next time hour (that is input utcTimemark)
            deciSecFromNow = deciSecUntilNextHour + utcTimemark

        if deciSecFromNow >= DECISEC_IN_HOUR-1:
            deciSecFromNow = 0

        return deciSecFromNow

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
            phaseState["minEndTime"] = self.get_utcTimemark_from_deciSecFromNow(phaseState["minEndTime"])
            phaseState["maxEndTime"] = self.get_utcTimemark_from_deciSecFromNow(phaseState["maxEndTime"])
        
        # For each pedestrian state, modify the min and max end times to timemarks in current 
        # or the next UTC hour
        for pedPhaseState in pedPhaseStates:
            pedPhaseState["minEndTime"] = self.get_utcTimemark_from_deciSecFromNow(pedPhaseState["minEndTime"])
            pedPhaseState["maxEndTime"] = self.get_utcTimemark_from_deciSecFromNow(pedPhaseState["maxEndTime"])

        # Plug in the modified parameters in the corresponding parameters of modified SPaT JSON object
        modifiedSpatJson["Spat"]["phaseState"] = phaseStates
        modifiedSpatJson["Spat"]["pedPhaseState"] = pedPhaseStates

        # Create a modified JSON string from the modified JSON object and return it
        return json.dumps(modifiedSpatJson)


    def modify_spat_json_to_deciSecFromNow(self, spatJsonString:str):
        """
        takes a SPaT JSON string as in input, modifies all min and max end times 
        to deciseconds from now, and returns the modified SPaT JSON string
        """
        # Create a JSON object from the input string
        spatJson = json.loads(spatJsonString)

        # Get original parameters for all vehicle and pedestrian phases 
        phaseStates = spatJson["Spat"]["phaseState"]
        pedPhaseStates = spatJson["Spat"]["pedPhaseState"]
        
        # Create a copy of the SPaT JSON object to store the modified parameters
        modifiedSpatJson = spatJson

        # For each vehicle phase, modify the min and max end times to deciseconds from now
        for phaseState in phaseStates:
            phaseState["minEndTime"] = self.get_deciSecFromNow_from_utcTimemark(phaseState["minEndTime"])
            phaseState["maxEndTime"] = self.get_deciSecFromNow_from_utcTimemark(phaseState["maxEndTime"])
        
        # For each vehicle phase, modify the min and max end times to deciseconds from now
        for pedPhaseState in pedPhaseStates:
            pedPhaseState["minEndTime"] = self.get_deciSecFromNow_from_utcTimemark(pedPhaseState["minEndTime"])
            pedPhaseState["maxEndTime"] = self.get_deciSecFromNow_from_utcTimemark(pedPhaseState["maxEndTime"])
        
        # Plug in the modified parameters in the corresponding parameters of modified SPaT JSON object
        modifiedSpatJson["Spat"]["phaseState"] = phaseStates
        modifiedSpatJson["Spat"]["pedPhaseState"] = pedPhaseStates

        # Create a modified JSON string from the modified JSON object and return it
        return json.dumps(modifiedSpatJson)

    def drop_inactive_phases(self, spatJson:str, inactiveVehPhases:list, inactivePedPhases:list):
        modifiedSpat = json.loads(spatJson)
        modifiedSpat["Spat"]["phaseState"] = [phase for phase in modifiedSpat["Spat"]["phaseState"] if phase["phaseNo"] not in inactiveVehPhases]
        modifiedSpat["Spat"]["pedPhaseState"] = [phase for phase in modifiedSpat["Spat"]["pedPhaseState"] if phase["phaseNo"] not in inactivePedPhases]
        
        return json.dumps(modifiedSpat)

    def get_standard_string_for_broadcast(self, spatJson:str, inactiveVehPhases:list, inactivePedPhases:list):
        spatJson = self.drop_inactive_phases(spatJson, inactiveVehPhases, inactivePedPhases)
        spatJson = self.modify_spat_json_to_utc_timemark(spatJson)
        return spatJson


if __name__=="__main__":
    import time
    TEST_CONVERTER_FUNCTIONS = False
    TEST_SPAT_CONVERSION = True

    utcHelper = UtcHelper() 

    if TEST_CONVERTER_FUNCTIONS==False and TEST_SPAT_CONVERSION==False: print("Nothing to test!")
    
    if TEST_CONVERTER_FUNCTIONS:
        while True:
            deciSecFromNow = 35000            
            utcTimemark = utcHelper.get_utcTimemark_from_deciSecFromNow(deciSecFromNow)
            computedDeciSecFromNow = utcHelper.get_deciSecFromNow_from_utcTimemark(utcTimemark)
            print("UTC Time:", utcTimemark, "Decisec From Now:",computedDeciSecFromNow)
            time.sleep(0.1)
    
    if TEST_SPAT_CONVERSION:
        with open("Spat.json", 'r') as spatFile:
            spatJson = json.load(spatFile)
            spatJsonString = json.dumps(spatJson)
            
        startTime = time.time()
        modifiedSpatJsonString = utcHelper.modify_spat_json_to_utc_timemark(spatJsonString)
        endTime = time.time()
        
        print(modifiedSpatJsonString)
        print("\nTime taken to modify SPaT from deciseconds from now to UTC timemark = {} seconds\n".format(endTime-startTime))

        startTime = time.time()
        remodifiedSpatJsonString = utcHelper.modify_spat_json_to_deciSecFromNow(modifiedSpatJsonString)
        endTime = time.time()

        print(remodifiedSpatJsonString)
        print("\nTime taken to modify SPaT from UTC timemark to deciseconds from now = {} seconds\n".format(endTime-startTime))

        # A test to verify if we are back to the original string after modifying and then remodifying
        remodifiedSpatJson = json.loads(remodifiedSpatJsonString)

        # Verify vehicle phases
        originalVehPhases = spatJson["Spat"]["phaseState"]
        remodifiedVehPhases = remodifiedSpatJson["Spat"]["phaseState"]

        vehMinEndTimeResults = []
        vehMaxEndTimeResults = []

        for vehPhase in originalVehPhases:
            phaseId = vehPhase["phaseNo"]
            originalMinEndTime = vehPhase["minEndTime"]
            originalMaxEndTime = vehPhase["maxEndTime"]

            remodifiedPhase = remodifiedVehPhases[phaseId-1]
            remodifiedMinEndTime = remodifiedPhase["minEndTime"]
            remodifiedMaxEndTime = remodifiedPhase["maxEndTime"]

            if originalMinEndTime!=remodifiedMinEndTime:
                vehMinEndTimeResults += ["FAIL"]
            else:
                vehMinEndTimeResults += ["PASS"]
            
            if originalMaxEndTime!=remodifiedMaxEndTime:
                vehMaxEndTimeResults += ["FAIL"]
            else:
                vehMaxEndTimeResults += ["PASS"]

        print("VehMinEndTime Test Results:{}".format(str(vehMinEndTimeResults)))
        print("VehMaxEndTime Test Results:{}".format(str(vehMaxEndTimeResults)))

        # Verify pedestrian phases
        originalPedPhases = spatJson["Spat"]["pedPhaseState"]
        remodifiedPedPhases = remodifiedSpatJson["Spat"]["pedPhaseState"]

        pedMinEndTimeResults = []
        pedMaxEndTimeResults = []

        for pedPhase in originalPedPhases:
            phaseId = pedPhase["phaseNo"]
            originalMinEndTime = pedPhase["minEndTime"]
            originalMaxEndTime = pedPhase["maxEndTime"]

            remodifiedPhase = remodifiedPedPhases[phaseId-1]
            remodifiedMinEndTime = remodifiedPhase["minEndTime"]
            remodifiedMaxEndTime = remodifiedPhase["maxEndTime"]

            if originalMinEndTime!=remodifiedMinEndTime:
                pedMinEndTimeResults += ["FAIL"]
            else:
                pedMinEndTimeResults += ["PASS"]
            
            if originalMaxEndTime!=remodifiedMaxEndTime:
                pedMaxEndTimeResults += ["FAIL"]
            else:
                pedMaxEndTimeResults += ["PASS"]

        print("PedMinEndTime Test Results:{}".format(str(vehMinEndTimeResults)))
        print("PedMaxEndTime Test Results:{}".format(str(vehMaxEndTimeResults)))
            
                
