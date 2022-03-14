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
- checkTimedOutCoordinationPlanClearingRequirement(): Method to check whether current coordination plan is timed-out or not
- fillUpCoordinationParametersDictionary(): Method to fill up the coordinationParametersDictionary with active coordination plan parameters
- resetParameters(self): Method to reset the parameters of CoordinationPlanManager class 
- updateCoordinationConfigData(self, coordinationConfigData):  Method to update coordination config data
- getOffset(coordinationPatternNo): Method to compute the cycle length for different coordination plan
- getDayOfWeek(): Method to compute the today's day of the week
- getCurrentTime(): Method to obtain the current time of today
***************************************************************************************
"""

import json
import datetime
import time
import calendar
from datetime import date
from Logger import Logger

SECOND_MINUTE_CONVERSION = 60.0
HOUR_SECOND_CONVERSION = 3600.0


class CoordinationPlanManager:
    def __init__(self, coordinationConfigData, config, logger:Logger):
        self.logger = logger
        self.coordinationConfigData = coordinationConfigData
        self.config = config
        self.activePlanCheckingTime = 0.0
        self.timeGapBetweenActivePlanChecking = self.config['CoordinationPlanCheckingTimeInterval']
        self.coordinationPlanName = ""
        self.coordinationParametersDictionary = {}
        self.coordinationSplitDataDictionary = {}
        self.coordinatedPhase1 = 0
        self.coordinatedPhase2 = 0
        self.cycleLength = 0.0
        self.offset = 0.0
        self.coordinationStartTime_Hour = 0
        self.coordinationStartTime_Minute = 0
        self.coordinationEndTime_Hour = 0
        self.coordinationEndTime_Minute = 0
        self.activeCoordinationPatternNo = 0

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
        For each coordination day plan, find out today's day plan
        For the matched coordination day plan, compute the start time, end time and cycle length of the coordination action plan
        If the current time is in between the start time and end time of a coordination plan or time difference between the coordination start time and current time is less than or equal to the cycle length, the plan is active coordination plan.
        Store active coordination plan name
        """
        self.resetParameters()
        currentTime = self.getCurrentTime()
        dayOfWeek = self.getDayOfWeek()

        for coordinationDayPlanDictionary in self.coordinationConfigData['CoordinationDayPlan']:
            if dayOfWeek in coordinationDayPlanDictionary['Days']:
                for parameters in coordinationDayPlanDictionary['ActionPlan']:
                    coordinationStartTime = parameters['CoordinationStartTime_Hour'] * float(
                        HOUR_SECOND_CONVERSION) + parameters['CoordinationStartTime_Minute'] * float(SECOND_MINUTE_CONVERSION)

                    coordinationEndTime = parameters['CoordinationEndTime_Hour'] * float(
                        HOUR_SECOND_CONVERSION) + parameters['CoordinationEndTime_Minute'] * float(SECOND_MINUTE_CONVERSION)

                    cycleLength = self.getOffset(
                        parameters['CoordinationPatternNo'])

                    if currentTime >= coordinationStartTime and currentTime <= coordinationEndTime or abs(coordinationStartTime - currentTime) <= cycleLength:
                        self.activeCoordinationPatternNo = parameters['CoordinationPatternNo']
                        self.coordinationStartTime_Hour = parameters['CoordinationStartTime_Hour']
                        self.coordinationStartTime_Minute = parameters['CoordinationStartTime_Minute']
                        self.coordinationEndTime_Hour = parameters['CoordinationEndTime_Hour']
                        self.coordinationEndTime_Minute = parameters['CoordinationEndTime_Minute']
                        self.fillUpCoordinationParametersDictionary()
                        self.logger.loggingAndConsoleDisplay("Active Coordination Parameter is following:")
                        self.logger.loggingAndConsoleDisplay(str(self.coordinationParametersDictionary))
                        break

        return self.coordinationParametersDictionary

    def getSplitData(self):
        """
        Store Split data for the active coordination plan in a json string
        """
        phaseNumber = [1, 2, 3, 4, 5, 6, 7, 8]
        splitTime = []
        if bool(self.coordinationSplitDataDictionary):
            self.coordinationSplitDataDictionary = {}

        for parameters in self.coordinationConfigData['CoordinationPattern']:
            if (parameters['CoordinationPatternNo'] == self.activeCoordinationPatternNo) and (self.coordinationPlanName == parameters['CoordinationPlanName']):
                self.coordinatedPhase1 = parameters['CoordinatedPhase1']
                self.coordinatedPhase2 = parameters['CoordinatedPhase2']
                self.cycleLength = parameters['CycleLength']
                self.offset = parameters['Offset']
                for splitData in parameters['Split']:
                    splitTime.append(splitData)

        if len(splitTime):
            self.coordinationSplitDataDictionary = json.dumps({
                "MsgType": "ActiveCoordinationPlan",
                "CycleLength": self.cycleLength,
                "Offset": self.offset,
                "CoordinationStartTime_Hour": self.coordinationStartTime_Hour,
                "CoordinationStartTime_Minute": self.coordinationStartTime_Minute,
                "CoordinatedPhase1": self.coordinatedPhase1,
                "CoordinatedPhase2": self.coordinatedPhase2,
                "TimingPlan":
                    {
                        "NoOfPhase": 8,
                        "PhaseNumber": phaseNumber,
                        "SplitData": splitTime
                    }
            })
        if bool(self.coordinationSplitDataDictionary):
            self.logger.loggingAndConsoleDisplay(str(self.coordinationSplitDataDictionary))                       

        return self.coordinationSplitDataDictionary

    def checkTimedOutCoordinationPlanClearingRequirement(self):
        """
        Get the current time and compute the End time of the current coordination plan.
        If current Time greater than the coordination End Time clear out the current coordination plan.
        """
        clearTimedOutCoordinationPlan = False
        currentTime = self.getCurrentTime()
        if not bool(self.coordinationParametersDictionary):
            clearTimedOutCoordinationPlan = False

        else:
            for parameters in self.coordinationConfigData['CoordinationPattern']:
                coordinationEndTime = self.coordinationEndTime_Hour * float(
                    HOUR_SECOND_CONVERSION) + self.coordinationEndTime_Minute * float(SECOND_MINUTE_CONVERSION)

                if currentTime > coordinationEndTime and parameters['CoordinationPlanName'] == self.coordinationPlanName:
                    self.logger.loggingAndConsoleDisplay("Cleared timed-out coordination plan")
                    self.coordinationPlanName = ""
                    self.coordinationParametersDictionary.clear()
                    clearTimedOutCoordinationPlan = True
                    break

        return clearTimedOutCoordinationPlan

    def fillUpCoordinationParametersDictionary(self):
        """
        Filled up the coordinationParametersDictionary with active coordination plan parameters
        """
        for parameters in self.coordinationConfigData['CoordinationPattern']:
            if parameters['CoordinationPatternNo'] == self.activeCoordinationPatternNo:
                self.coordinationParametersDictionary = {
                    "CoordinationPlanName": parameters['CoordinationPlanName'],
                    "CoordinationPatternNo": parameters['CoordinationPatternNo'],
                    "CycleLength": parameters['CycleLength'],
                    "Offset": parameters['Offset'],
                    "CoordinationStartTime_Hour": self.coordinationStartTime_Hour,
                    "CoordinationStartTime_Minute": self.coordinationStartTime_Minute,
                    "CoordinationEndTime_Hour": self.coordinationEndTime_Hour,
                    "CoordinationEndTime_Minute": self.coordinationEndTime_Minute,
                    "CoordinationSplit": parameters['CoordinationSplit'],
                    "CoordinatedPhase1": parameters['CoordinatedPhase1'],
                    "CoordinatedPhase2": parameters['CoordinatedPhase2']
                }
                self.coordinationPlanName = parameters['CoordinationPlanName']

    def resetParameters(self):
        """
        Reset the parameters of CoordinationPlanManager class
        """
        self.coordinationParametersDictionary = {}
        self.coordinationSplitDataDictionary = {}
        self.coordinatedPhase1 = 0
        self.coordinatedPhase2 = 0
        self.cycleLength = 0.0
        self.offset = 0.0
        self.coordinationStartTime_Hour = 0
        self.coordinationStartTime_Minute = 0
        self.coordinationEndTime_Hour = 0
        self.coordinationEndTime_Minute = 0
        self.activeCoordinationPatternNo = 0

    def updateCoordinationConfigData(self, coordinationConfigData):
        """
        Update coordination config data
        """
        self.coordinationConfigData = coordinationConfigData

    def getOffset(self, coordinationPatternNo):
        """
        Compute the cycle length for different coordination plan
        """
        for parameters in self.coordinationConfigData['CoordinationPattern']:

            if parameters['CoordinationPatternNo'] == coordinationPatternNo:
                cycleLength = parameters['CycleLength']
                break

        return cycleLength

    def getDayOfWeek(self):
        """
        Compute the today's day of the week
        """
        date_Today = date.today()
        dayOfWeek = calendar.day_name[date_Today.weekday()]

        return dayOfWeek

    def getCurrentTime(self):
        """
        Compute the current time based on current hour, minute and second of a day in the unit of second
        """
        timeNow = datetime.datetime.now()
        currentTime = timeNow.hour * 3600.0 + timeNow.minute * 60.0 + timeNow.second

        return currentTime
