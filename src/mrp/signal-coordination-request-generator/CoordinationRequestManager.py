"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

CoordinationRequestManager.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
The methods available from this class are the following:
- checkCoordinationRequestSendingRequirement(): A boolean function to check whether coordination request is required to send or not 
- generateVirtualCoordinationPriorityRequest(): Method to generate virtual the coordination requests at the beginning of each cycle
- checkUpdateRequestSendingRequirement(): A boolean function to check whether coordination priority requests are required to send or not to avoid PRS timed-out
- generateUpdatedCoordinationPriorityRequest():  Method to generate updated cooridnation priority request to avoid PRS timed-out
- updateETAInCoordinationRequestTable(): Method to update ETA and coordination split for each coordination priority request
- deleteTimeOutRequestFromCoordinationRequestTable(): Method to delete the coordination priority requestfor whom coordination split value is zero
- clearTimedOutCoordinationPlan(): Method to clear coordination parameters for old coordination plan
- getCoordinationClearRequestDictionary(): Method to get the Coordination clear requests if coordination request is empty or active coordination plan gets timed-out 
- getCoordinationPriorityRequestDictionary(): Method to obtain json string for the coordination priority requests after deleting the old requests.
- getCoordinationParametersDictionary(dictionary): Method to load the coordination prarameters dictionary
- getCurrentTime(): Method to obtain the current time of today
- getMsgCount(): Method to get the msgCount
***************************************************************************************
"""
import datetime
import time
import json
from CoordinatedPhase import CoordinatedPhase
from Logger import Logger

HOUR_DAY_CONVERSION = 24
MINUTE_HOUR_CONVERSION = 60
SECOND_MINUTE_CONVERSION = 60
HOUR_SECOND_CONVERSION = 3600
SECOND_MILISECOND_CONVERSION = 1000
MaxMsgCount = 127
MinMsgCount = 1


class CoordinationRequestManager:
    def __init__(self, config, logger: Logger):
        self.logger = logger
        self.config = config
        self.SRM_GAPOUT_TIME = 2.0
        self.requestTimedOutValue = self.config['SRMTimedOutTime'] - \
            self.SRM_GAPOUT_TIME

        # Define Coordination Parameters
        self.coordinationParametersDictionary = {}
        self.coordinationVehicleType = 20
        self.basicVehicleRole = 11
        self.priorityRequest = 1
        self.requestUpdate = 2
        self.coordinationPriorityRequestDictionary = {}
        self.coordinationClearRequestDictionary = {}
        self.ETA_Update_Time = 1.0
        self.Minimum_ETA = 0.0
        self.Request_Delete_Time = 0.0
        self.requestSentTime = time.time()
        self.msgCount = 0
        self.ETAOfFirstCycleCoordinatedPhase = 0.0
        self.ETAOfSecondCycleCoordinatedPhase = 0.0

    def checkCoordinationRequestSendingRequirement(self):
        """
        If there is active coordination plan, it is not required to check for coordination request sending requirement
        If time gap between the current time and coordination start time is less than equal to the cycle length, it is required to send virtual coordination priority requests
        If time gap between the current time and remainder time of a cycle is in between 0 to 1 second, it is required to send coordination priority requests
        """
        if not bool(self.coordinationParametersDictionary):
            sendRequest = False
        else:
            currentTime = self.getCurrentTime()
            coordinationStartTime = self.coordinationParametersDictionary['CoordinationStartTime_Hour'] * float(
                HOUR_SECOND_CONVERSION) + self.coordinationParametersDictionary['CoordinationStartTime_Minute'] * float(SECOND_MINUTE_CONVERSION)
            cycleLength = self.coordinationParametersDictionary['CycleLength']
            offset = self.coordinationParametersDictionary['Offset']

            elapsedTimeInCycle = (
                currentTime - coordinationStartTime) % cycleLength
            if not bool(self.coordinationPriorityRequestDictionary) and (coordinationStartTime - currentTime >= 0) and ((coordinationStartTime - currentTime) <= (cycleLength - offset)):
                sendRequest = True
                self.ETAOfFirstCycleCoordinatedPhase = coordinationStartTime - currentTime

            elif (currentTime > coordinationStartTime) and (offset - elapsedTimeInCycle) >= 0 and (offset - elapsedTimeInCycle) <= 1.0:
                sendRequest = True
                self.ETAOfFirstCycleCoordinatedPhase = offset - elapsedTimeInCycle

            else:
                sendRequest = False

        return sendRequest

    def generateVirtualCoordinationPriorityRequest(self):
        """
        Following method will generated virtual coordination priority requests at the beginning of each cycle
        """
        relativeETAInMiliSecond = 0
        coordinationRequestList = []
        coordinationVehicleID = [1, 2, 3, 4]
        coordinatedPhase1 = self.coordinationParametersDictionary['CoordinatedPhase1']
        coordinatedPhase2 = self.coordinationParametersDictionary['CoordinatedPhase2']
        coordinatedPhases = [
            coordinatedPhase1, coordinatedPhase2, coordinatedPhase1, coordinatedPhase2]
        cycleLength = self.coordinationParametersDictionary['CycleLength']
        self.ETAOfSecondCycleCoordinatedPhase = self.ETAOfFirstCycleCoordinatedPhase + cycleLength
        coordinatedPhaseETA = [self.ETAOfFirstCycleCoordinatedPhase, self.ETAOfFirstCycleCoordinatedPhase,
                               self.ETAOfSecondCycleCoordinatedPhase, self.ETAOfSecondCycleCoordinatedPhase]
        coordinationSplit = self.coordinationParametersDictionary['CoordinationSplit']

        for i in range(len(coordinatedPhases)):
            relativeETAInMiliSecond = coordinatedPhaseETA[i] * \
                SECOND_MILISECOND_CONVERSION + self.getMsOfMinute()
            temporaryCoordinationRequest = CoordinatedPhase()
            temporaryCoordinationRequest.phaseNo = coordinatedPhases[i]
            temporaryCoordinationRequest.vehicleType = self.coordinationVehicleType
            temporaryCoordinationRequest.vehicleID = coordinationVehicleID[i]
            temporaryCoordinationRequest.basicVehicleRole = self.basicVehicleRole
            temporaryCoordinationRequest.ETA = coordinatedPhaseETA[i]
            temporaryCoordinationRequest.ETA_Minute = self.getMinuteOfYear(
            ) + (relativeETAInMiliSecond // (SECOND_MINUTE_CONVERSION * SECOND_MILISECOND_CONVERSION))
            temporaryCoordinationRequest.ETA_Second = relativeETAInMiliSecond % (
                SECOND_MINUTE_CONVERSION * SECOND_MILISECOND_CONVERSION)
            temporaryCoordinationRequest.coordinationSplit = int(coordinationSplit * SECOND_MILISECOND_CONVERSION)
            temporaryCoordinationRequest.priorityRequestType = self.priorityRequest
            temporaryCoordinationRequest.requestUpdateTime = time.time()
            coordinationRequestList.append(temporaryCoordinationRequest)

        coordinationRequestDict = [dict()
                                   for x in range(len(coordinatedPhases))]

        for i in range(len(coordinatedPhases)):
            coordinationRequestDict[i] = coordinationRequestList[i].asDict()

        self.coordinationPriorityRequestDictionary = {
            "MsgType": "CoordinationRequest",
            "msgCount": self.getMsgCount(),
            "noOfCoordinationRequest": len(coordinatedPhases),
            "minuteOfYear": self.getMinuteOfYear(),
            "msOfMinute": self.getMsOfMinute(),
            "CoordinationRequestList": {"requestorInfo": coordinationRequestDict}
        }

        self.requestSentTime = time.time()
        coordinationPriorityRequestJsonString = json.dumps(
            self.coordinationPriorityRequestDictionary)
        self.logger.looging(coordinationPriorityRequestJsonString)

        return coordinationPriorityRequestJsonString

    def checkUpdateRequestSendingRequirement(self):
        """
        A boolean function to check whether coordination priority requests is required to send to avoid PRS timed-out
        """
        sendRequest = False
        currentTime_UTC = time.time()
        if bool(self.coordinationPriorityRequestDictionary):
            noOfCoordinationRequest = self.coordinationPriorityRequestDictionary[
                'noOfCoordinationRequest']

            if (noOfCoordinationRequest > 0) and ((currentTime_UTC - self.requestSentTime) >= self.requestTimedOutValue):
                sendRequest = True

        return sendRequest

    def generateUpdatedCoordinationPriorityRequest(self):
        """
        The following method generates updated cooridnation priority request to avoid PRS timed-out.
        The method calls updateETAInCoordinationRequestTable() to update the ETA.
        """
        self.coordinationPriorityRequestDictionary['minuteOfYear'] = self.getMinuteOfYear(
        )
        self.coordinationPriorityRequestDictionary['msOfMinute'] = self.getMsOfMinute(
        )
        self.coordinationPriorityRequestDictionary['msgCount'] = self.getMsgCount(
        )
        self.updateETAInCoordinationRequestTable()
        self.deleteTimeOutRequestFromCoordinationRequestTable
        self.requestSentTime = time.time()
        coordinationPriorityRequestJsonString = json.dumps(
            self.coordinationPriorityRequestDictionary)
        self.logger.looging(coordinationPriorityRequestJsonString)

        return coordinationPriorityRequestJsonString

    def updateETAInCoordinationRequestTable(self):
        """
        The following method updates the active coordination priority request and sends the updated request to PriorityRequestServer
        If the ETA of a request is greater than the minimum ETA (predefined), the following method only updates the ETA.
        If the ETA of a Request is less or equal to the minimum ETA (preddefined), the following method updates (reduces) the Coordination Split time and set the ETA as minimum ETA.
        The method will set the request type as requestUpdate
        """
        relativeETAInMiliSecond = 0
        if bool(self.coordinationPriorityRequestDictionary):
            currentTime_UTC = time.time()
            noOfCoordinationRequest = self.coordinationPriorityRequestDictionary[
                'noOfCoordinationRequest']
            for i in range(noOfCoordinationRequest):
                temporaryRequestUpdateTime = self.coordinationPriorityRequestDictionary[
                    'CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime']
                self.coordinationPriorityRequestDictionary['CoordinationRequestList'][
                    'requestorInfo'][i]['priorityRequestType'] = self.requestUpdate

                if (currentTime_UTC - temporaryRequestUpdateTime) >= self.ETA_Update_Time:
                    self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] = self.coordinationPriorityRequestDictionary[
                        'CoordinationRequestList']['requestorInfo'][i]['ETA'] - (currentTime_UTC - temporaryRequestUpdateTime)

                    if self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] <= self.Minimum_ETA:
                        self.coordinationPriorityRequestDictionary['CoordinationRequestList'][
                            'requestorInfo'][i]['ETA'] = self.Minimum_ETA
                        self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['CoordinationSplit'] = int(self.coordinationPriorityRequestDictionary[
                            'CoordinationRequestList']['requestorInfo'][i]['CoordinationSplit'] - (currentTime_UTC - temporaryRequestUpdateTime) * SECOND_MILISECOND_CONVERSION)

                    relativeETAInMiliSecond = self.coordinationPriorityRequestDictionary['CoordinationRequestList'][
                        'requestorInfo'][i]['ETA'] * SECOND_MILISECOND_CONVERSION + self.getMsOfMinute()

                    self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA_Minute'] = self.getMinuteOfYear() + (
                        int(relativeETAInMiliSecond) // (SECOND_MINUTE_CONVERSION * SECOND_MILISECOND_CONVERSION))
                    self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA_Second'] = int(relativeETAInMiliSecond) % (
                        SECOND_MINUTE_CONVERSION * SECOND_MILISECOND_CONVERSION)

                    self.coordinationPriorityRequestDictionary['CoordinationRequestList'][
                        'requestorInfo'][i]['requestUpdateTime'] = time.time()

    def deleteTimeOutRequestFromCoordinationRequestTable(self):
        """
        Method to delete the old coordination requests, if coordination split is zero
        """
        deleteCoordinationRequest = False
        if bool(self.coordinationPriorityRequestDictionary):
            noOfCoordinationRequest = self.coordinationPriorityRequestDictionary[
                'noOfCoordinationRequest']
            requestorIndex = 0
            while requestorIndex < noOfCoordinationRequest:
                if self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][requestorIndex]['CoordinationSplit'] <= self.Request_Delete_Time:
                    del[self.coordinationPriorityRequestDictionary['CoordinationRequestList']
                        ['requestorInfo'][requestorIndex]]
                    self.coordinationPriorityRequestDictionary[
                        'noOfCoordinationRequest'] = noOfCoordinationRequest - 1
                    requestorIndex -= 1
                    noOfCoordinationRequest -= 1
                    deleteCoordinationRequest = True
                requestorIndex += 1

        return deleteCoordinationRequest

    def getCoordinationPriorityRequestDictionary(self):
        """
        Method to get the Coordination requests List after the deletion process 
        """
        noOfCoordinationRequest = self.coordinationPriorityRequestDictionary[
            'noOfCoordinationRequest']

        if noOfCoordinationRequest > 0:
            self.coordinationPriorityRequestDictionary['minuteOfYear'] = self.getMinuteOfYear(
            )
            self.coordinationPriorityRequestDictionary['msOfMinute'] = self.getMsOfMinute(
            )
            self.coordinationPriorityRequestDictionary['msgCount'] = self.getMsgCount(
            )
            for i in range(noOfCoordinationRequest):
                self.coordinationPriorityRequestDictionary['CoordinationRequestList'][
                    'requestorInfo'][i]['priorityRequestType'] = self.requestUpdate
                self.coordinationPriorityRequestDictionary['CoordinationRequestList'][
                    'requestorInfo'][i]['requestUpdateTime'] = time.time()
            self.requestSentTime = time.time()

        else:
            self.coordinationPriorityRequestDictionary = {}

        coordinationPriorityRequestJsonString = json.dumps(
            self.coordinationPriorityRequestDictionary)

        return coordinationPriorityRequestJsonString

    def clearTimedOutCoordinationPlan(self):
        """
        Method to clear timed-out the coordination prarameters dictionary
        """
        self.coordinationParametersDictionary.clear()
        self.coordinationPriorityRequestDictionary.clear()

    def getCoordinationClearRequestDictionary(self):
        """
        Method to get the Coordination clear requests if coordination request is empty or coordination plan gets timed-out 
        """
        self.coordinationClearRequestDictionary = {
            "MsgType": "CoordinationRequest",
            "msgCount": self.getMsgCount(),
            "noOfCoordinationRequest": 0
        }
        coordinationClearRequestJsonString = json.dumps(
            self.coordinationClearRequestDictionary)
        self.logger.looging(coordinationClearRequestJsonString)

        return coordinationClearRequestJsonString

    def getCoordinationParametersDictionary(self, dictionary):
        """
        Method to load the coordination prarameters dictionary
        """
        self.coordinationParametersDictionary = dictionary

        return self.coordinationParametersDictionary

    def getCurrentTime(self):
        """
        Compute the current time based on current hour, minute and second of a day in the unit of second
        """
        timeNow = datetime.datetime.now()
        currentTime = timeNow.hour * \
            float(HOUR_SECOND_CONVERSION) + \
            timeNow.minute * 60.0 + timeNow.second

        return currentTime

    def getMinuteOfYear(self):
        """
        Method for obtaining minute of a year based on current time
        """
        timeNow = datetime.datetime.now()
        dayOfYear = int(timeNow.strftime("%j"))
        currentHour = timeNow.hour
        currentMinute = timeNow.minute
        minuteOfYear = (dayOfYear - 1) * HOUR_DAY_CONVERSION * MINUTE_HOUR_CONVERSION + currentHour * MINUTE_HOUR_CONVERSION + currentMinute

        return minuteOfYear

    def getMsOfMinute(self):
        """
        Method for obtaining millisecond of a minute based on current time
        """
        timeNow = datetime.datetime.now()
        currentSecond = timeNow.second
        msOfMinute = currentSecond * SECOND_MILISECOND_CONVERSION

        return msOfMinute

    def getMsgCount(self):
        """
        Method to get the msgCount
        """
        if self.msgCount < MaxMsgCount:
            self.msgCount += 1

        else:
            self.msgCount = MinMsgCount

        return self.msgCount
