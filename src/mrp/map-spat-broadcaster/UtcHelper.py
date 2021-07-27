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


        currentUtcTime = datetime.utcnow()
        currentUtcMillisecond = ((currentUtcTime.minute*SEC_IN_MINUTE + currentUtcTime.second)*MILLISEC_IN_SECOND + 
                                    math.floor(currentUtcTime.microsecond/MICROSEC_IN_MILLISEC))
        
        utcTimemark = milliSecFromNow + currentUtcMillisecond

        if (utcTimemark) >= MILLISEC_IN_HOUR: # That is if the utcTimemark is in the next UTC hour
            return (utcTimemark-MILLISEC_IN_HOUR)
        else: return utcTimemark

    def get_milliSecFromNow_from_utcTimemark(self, utcTimemark:int):
        currentUtcTime = datetime.utcnow()
        currentUtcMillisecond = ((currentUtcTime.minute*SEC_IN_MINUTE + currentUtcTime.second)*MILLISEC_IN_SECOND + 
                                    math.floor(currentUtcTime.microsecond/MICROSEC_IN_MILLISEC))
        
        if currentUtcMillisecond <= utcTimemark:
            milliSecFromNow = utcTimemark - currentUtcMillisecond
        else:
            milliSecUntilNextHour = MILLISEC_IN_HOUR - currentUtcMillisecond
            milliSecFromNow = milliSecUntilNextHour + utcTimemark

        return milliSecFromNow

    def modify_spat_json_to_utc_timemark(self, spatJsonString:str):
        spatJson = json.loads(spatJsonString)
        modifiedSpatJson = spatJson
        phaseStates = spatJson["Spat"]["phaseState"]
        pedPhaseStates = spatJson["Spat"]["pedPhaseState"]

        for phaseState in phaseStates:
            phaseState["minEndTime"] = self.get_utcTimemark_from_milliSecFromNow(phaseState["minEndTime"])
            phaseState["maxEndTime"] = self.get_utcTimemark_from_milliSecFromNow(phaseState["maxEndTime"])
        
        for pedPhaseState in pedPhaseStates:
            pedPhaseState["minEndTime"] = self.get_utcTimemark_from_milliSecFromNow(pedPhaseState["minEndTime"])
            pedPhaseState["maxEndTime"] = self.get_utcTimemark_from_milliSecFromNow(pedPhaseState["maxEndTime"])

        modifiedSpatJson["Spat"]["phaseState"] = phaseStates
        modifiedSpatJson["Spat"]["pedPhaseState"] = pedPhaseStates

        modifiedSpatJsonString = json.dumps(modifiedSpatJson)

        return modifiedSpatJsonString


    def modify_spat_json_to_milliSecFromNow(self, spatJsonString:str):
        spatJson = json.loads(spatJsonString)
        modifiedSpatJson = spatJson
        phaseStates = spatJson["Spat"]["phaseState"]
        pedPhaseStates = spatJson["Spat"]["pedPhaseState"]

        for phaseState in phaseStates:
            phaseState["minEndTime"] = self.get_milliSecFromNow_from_utcTimemark(phaseState["minEndTime"])
            phaseState["maxEndTime"] = self.get_milliSecFromNow_from_utcTimemark(phaseState["maxEndTime"])
        
        for pedPhaseState in pedPhaseStates:
            pedPhaseState["minEndTime"] = self.get_milliSecFromNow_from_utcTimemark(pedPhaseState["minEndTime"])
            pedPhaseState["maxEndTime"] = self.get_milliSecFromNow_from_utcTimemark(pedPhaseState["maxEndTime"])

        modifiedSpatJson["Spat"]["phaseState"] = phaseStates
        modifiedSpatJson["Spat"]["pedPhaseState"] = pedPhaseStates

        modifiedSpatJsonString = json.dumps(modifiedSpatJson)

        return modifiedSpatJsonString


if __name__=="__main__":
    import time
    TEST_CONVERTER_FUNCTIONS = False
    TEST_SPAT_CONVERSION = False

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
            modifiedSpatJsonString = utcHelper.modify_spat_json_to_utc_timemark(spatJsonString)
            print(modifiedSpatJsonString)

            remodifiedSpatJsonString = utcHelper.modify_spat_json_to_milliSecFromNow(modifiedSpatJsonString)
            print(remodifiedSpatJsonString)
            
