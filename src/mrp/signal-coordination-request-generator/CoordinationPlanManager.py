"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

CoordinationPlanManager.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
The methods available from this class are the following:
- checkActiveCoordinationPlan(): A boolean function to check whether active coordination plan is available or not 
- getActiveCoordinationPlan(): Method to obtain the coordination parameters for the active coordination Plan
- getSplitData(): Method to obtain the split data for the active coordination Plan
- checkTimedOutCoordinationPlanClearingRequirement(): Method to check whether current coordination plan is old or not 
- getCurrentTime(): Method to obtain the current time of today
***************************************************************************************
"""

import json
import datetime
import time

SECONDSINAMINUTE = 60.0
HourToSecondConversion =  3600.0

class CoordinationPlanManager:
    def __init__(self, data):
        self.data = data
        self.activePlanCheckingTime = 0.0
        self.timeGapBetweenActivePlanChecking = 1800
        self.coordinationPlanName = ""
        self.coordinationParametersDictionary = {}
        self.coordinationSplitDataDictionary = {}

    def checkActiveCoordinationPlan(self):
        """
        If there is no active coordination plan, it is required to check for active coordination plan
        check the time gap between the current time and last time when active coordination plan is checked
        """
        activePlanCheckingRequirement = False
        currentTime_UTC = time.time()

        if not bool(self.coordinationParametersDictionary):
            activePlanCheckingRequirement = True
            self.activePlanCheckingTime = time.time()

        elif currentTime_UTC - self.activePlanCheckingTime >= self.timeGapBetweenActivePlanChecking:
            activePlanCheckingRequirement = True
            self.activePlanCheckingTime = time.time()

        return activePlanCheckingRequirement

    def getActiveCoordinationPlan(self):
        """
        For each coordination plan compute the start time and end time of the coordination plan
        If the current time is in between the start time and end time of a coordination plan or time difference between the coordination start time and current time is less than or equal to the cycle length, the plan is active coordination plan.
        Store active coordination plan name
        """
        currentTime = self.getCurrentTime()

        for parameters in self.data['CoordinationParameters']:
            coordinationStartTime = parameters['CoordinationStartTime_Hour'] * float(HourToSecondConversion) + parameters['CoordinationStartTime_Minute'] * float(SECONDSINAMINUTE)
            coordinationEndTime = parameters['CoordinationEndTime_Hour'] * float(HourToSecondConversion) + parameters['CoordinationEndTime_Minute'] * float(SECONDSINAMINUTE)

            cycleLength = parameters['CycleLength']
            if currentTime >= coordinationStartTime and currentTime <= coordinationEndTime or abs(coordinationStartTime - currentTime) <= cycleLength:
                self.coordinationParametersDictionary = {
                    "CoordinationPlanName": parameters['CoordinationPlanName'],
                    "CoordinationPatternNo": parameters['CoordinationPatternNo'],
                    "SplitPatternNo": parameters['SplitPatternNo'],
                    "CycleLength": parameters['CycleLength'],
                    "Offset": parameters['Offset'],
                    "CoordinationStartTime_Hour": parameters['CoordinationStartTime_Hour'],
                    "CoordinationStartTime_Minute": parameters['CoordinationStartTime_Minute'],
                    "CoordinationEndTime_Hour": parameters['CoordinationEndTime_Hour'],
                    "CoordinationEndTime_Minute": parameters['CoordinationEndTime_Minute'],
                    "CoordinationSplit": parameters['CoordinationSplit'],
                    "CoordinatedPhase1": parameters['CoordinatedPhase1'],
                    "CoordinatedPhase2": parameters['CoordinatedPhase2']
                }
                self.coordinationPlanName = parameters['CoordinationPlanName']

                print("\n[" + str(datetime.datetime.now()) + "] " + "Active Coordination Parameter at time " + str(time.time())+ " is following: \n", self.coordinationParametersDictionary)
                break

        return self.coordinationParametersDictionary

    def getSplitData(self):
        """
        Store Split data for the active coordination plan in a json string
        """
        phaseNumber = []
        splitTime = []
        for parameters in self.data['CoordinationParameters']:
            if self.coordinationPlanName == parameters['CoordinationPlanName']:
                for splitData in parameters['SplitPatternData']['PhaseNumber']:
                    phaseNumber.append(splitData)
                for splitData in parameters['SplitPatternData']['Split']:
                    splitTime.append(splitData)

        self.coordinationSplitDataDictionary = json.dumps({
            "MsgType": "ActiveCoordinationPlan",
            "TimingPlan":
                {
                    "NoOfPhase": 8,
                    "PhaseNumber": phaseNumber,
                    "SplitData": splitTime
                }
        })

        print("\n[" + str(datetime.datetime.now()) + "] " + "Coordination Split Data at time " + str(time.time())+ " is following: \n", self.coordinationSplitDataDictionary)
        return self.coordinationSplitDataDictionary
    
    def checkTimedOutCoordinationPlanClearingRequirement(self):
        """
        Get the current time and compute the End time of the current coordination plan.
        If current Time greater than the coordination End Time clear out the current coordination plan.
        """
        clearTimedOutCoordinationPlan = False
        currentTime = self.getCurrentTime()
        for parameters in self.data['CoordinationParameters']:
            coordinationEndTime = parameters['CoordinationEndTime_Hour'] * float(HourToSecondConversion) + parameters['CoordinationEndTime_Minute'] * float(SECONDSINAMINUTE)

            if currentTime > coordinationEndTime:
                self.coordinationPlanName = ""
                self.coordinationParametersDictionary.clear()
                clearTimedOutCoordinationPlan = True
                
        return clearTimedOutCoordinationPlan
                
    def getCurrentTime(self):
        """
        Compute the current time based on current hour, minute and second of a day in the unit of second
        """
        timeNow = datetime.datetime.now()
        currentTime = timeNow.hour * 3600.0 + timeNow.minute * 60.0 + timeNow.second

        return currentTime
