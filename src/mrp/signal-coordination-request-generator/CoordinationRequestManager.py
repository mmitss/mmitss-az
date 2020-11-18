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
- getCoordinationPriorityRequestDictionary(): Method to obtain json string for the coordination priority requests after deleting the old requests.
- getCoordinationParametersDictionary(dictionary): Method to load the coordination prarameters dictionary
- getCurrentTime(): Method to obtain the current time of today
***************************************************************************************
"""
import datetime
import time
import json
from CoordinatedPhase import CoordinatedPhase

HOURSINADAY = 24.0
MINUTESINAHOUR = 60.0
SECONDSINAMINUTE = 60.0
HourToSecondConversion =  3600.0
SECONDTOMILISECOND = 1000.0

class CoordinationRequestManager:
    def __init__(self, config):
        self.config = config
        self.SRM_GAPOUT_TIME = 2.0
        self.requestTimedOutValue = self.config['SRMTimedOutTime'] - self.SRM_GAPOUT_TIME

        # Define Coordination Parameters
        self.coordinationParametersDictionary = {}
        self.coordinationVehicleType = 20
        self.basicVehicleRole = 11
        self.priorityRequest = 1
        self.requestUpdate = 2
        self.coordinationPriorityRequestDictionary = {}
        self.ETA_Update_Time = 1.0
        self.Minimum_ETA = 0.0
        self.Request_Delete_Time = 0.0
        self.requestSentTime = time.time()

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
            coordinationStartTime = self.coordinationParametersDictionary['CoordinationStartTime_Hour'] * float(HourToSecondConversion) + self.coordinationParametersDictionary['CoordinationStartTime_Minute'] * float(SECONDSINAMINUTE)
            cycleLength = self.coordinationParametersDictionary['CycleLength']

            remainderTime = currentTime % cycleLength
            # print("\n[" + str(datetime.datetime.now()) + "] " + "Elapsed Time in the Cycle at time " + str(time.time())+ " is ", remainderTime)

            if coordinationStartTime - currentTime >= 0 and coordinationStartTime - currentTime <= cycleLength:
                sendRequest = True

            elif cycleLength - remainderTime >= 0 and cycleLength - remainderTime <= 1.0:
                sendRequest = True

            else:
                sendRequest = False

        return sendRequest

    def generateVirtualCoordinationPriorityRequest(self):
        """
        Following method will generated virtual coordination priority requests at the beginning of each cycle
        """
        coordinationRequestList = []
        coordinationVehicleID = [1, 2, 3, 4]
        coordinatedPhase1 = self.coordinationParametersDictionary['CoordinatedPhase1']
        coordinatedPhase2 = self.coordinationParametersDictionary['CoordinatedPhase2']
        coordinatedPhases = [coordinatedPhase1, coordinatedPhase2, coordinatedPhase1, coordinatedPhase2]
        ETAOfFirstCycleCoordinatedPhase = self.coordinationParametersDictionary['Offset']
        cycleLength = self.coordinationParametersDictionary['CycleLength']
        ETAOfSecondCycleCoordinatedPhase = ETAOfFirstCycleCoordinatedPhase + cycleLength
        coordinatedPhaseETA = [ETAOfFirstCycleCoordinatedPhase, ETAOfFirstCycleCoordinatedPhase,
                               ETAOfSecondCycleCoordinatedPhase, ETAOfSecondCycleCoordinatedPhase]
        coordinationSplit = self.coordinationParametersDictionary['CoordinationSplit']
                
        for i in range(len(coordinatedPhases)):
            temporaryCoordinationRequest = CoordinatedPhase()
            temporaryCoordinationRequest.phaseNo = coordinatedPhases[i]
            temporaryCoordinationRequest.vehicleType = self.coordinationVehicleType
            temporaryCoordinationRequest.vehicleID = coordinationVehicleID[i]
            temporaryCoordinationRequest.basicVehicleRole = self.basicVehicleRole
            temporaryCoordinationRequest.ETA = coordinatedPhaseETA[i]
            temporaryCoordinationRequest.coordinationSplit = coordinationSplit
            temporaryCoordinationRequest.priorityRequestType = self.priorityRequest
            temporaryCoordinationRequest.requestUpdateTime = time.time()
            coordinationRequestList.append(temporaryCoordinationRequest)
            
        coordinationRequestDict = [dict()for x in range(len(coordinatedPhases))]
        
        for i in range(len(coordinatedPhases)):
            coordinationRequestDict[i] = coordinationRequestList[i].asDict()
            
        self.coordinationPriorityRequestDictionary = {
            "MsgType": "CoordinationRequest",
            "noOfCoordinationRequest": len(coordinatedPhases),
            "minuteOfYear": self.getMinuteOfYear(),
            "msOfMinute": self.getMsOfMinute(),
            "CoordinationRequestList":{"requestorInfo": coordinationRequestDict}
        }
        print("\n[" + str(datetime.datetime.now()) + "] " + "Virtual Coordination Request at time " + str(time.time())+ " is following: \n", self.coordinationPriorityRequestDictionary)
        self.requestSentTime = time.time()
        coordinationPriorityRequestJsonString = json.dumps(self.coordinationPriorityRequestDictionary)

        return coordinationPriorityRequestJsonString

    def checkUpdateRequestSendingRequirement(self):
        """
        A boolean function to check whether coordination priority requests is required to send to avoid PRS timed-out
        """
        sendRequest = False
        currentTime_UTC = time.time()

        if bool(self.coordinationPriorityRequestDictionary) and currentTime_UTC - self.requestSentTime >= self.requestTimedOutValue:
            sendRequest = True

        return sendRequest

    def generateUpdatedCoordinationPriorityRequest(self):
        """
        The following method generates updated cooridnation priority request to avoid PRS timed-out.
        The method will set the request type as requestUpdate.
        """
        noOfCoordinationRequest = self.coordinationPriorityRequestDictionary['noOfCoordinationRequest']
        
        self.coordinationPriorityRequestDictionary['minuteOfYear'] = self.getMinuteOfYear()
        self.coordinationPriorityRequestDictionary['msOfMinute'] = self.getMsOfMinute()
        
        for i in range(noOfCoordinationRequest):
            self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['priorityRequestType'] = self.requestUpdate
            self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime'] = time.time()
        
        self.requestSentTime = time.time()
        print("\n[" + str(datetime.datetime.now()) + "] " + "Updated Coordination Request to avoid PRS timed-out at time " + str(time.time())+ " is following: \n", self.coordinationPriorityRequestDictionary)
        coordinationPriorityRequestJsonString = json.dumps(self.coordinationPriorityRequestDictionary)

        return coordinationPriorityRequestJsonString

    def updateETAInCoordinationRequestTable(self):
        """
        The following method updates the active coordination priority request and sends the updated request to PriorityRequestServer
        If the ETA of a request is greater than the minimum ETA (predefined), the following method only updates the ETA.
        If the ETA of a Request is less or equal to the minimum ETA (preddefined), the following method updates the Coordination Split time and set the ETA as minimum ETA.
        """

        if bool(self.coordinationPriorityRequestDictionary):
            currentTime_UTC = time.time()
            noOfCoordinationRequest = self.coordinationPriorityRequestDictionary['noOfCoordinationRequest']
            for i in range(noOfCoordinationRequest):
                temporaryRequestUpdateTime = self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime']
                
                if currentTime_UTC - temporaryRequestUpdateTime >= self.ETA_Update_Time:
                    self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] = self.coordinationPriorityRequestDictionary[
                        'CoordinationRequestList']['requestorInfo'][i]['ETA'] - (currentTime_UTC - temporaryRequestUpdateTime)
                    
                    if self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] <= self.Minimum_ETA:
                        self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] = self.Minimum_ETA
                        self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['CoordinationSplit'] = self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['CoordinationSplit'] - (currentTime_UTC - temporaryRequestUpdateTime)
                    
                    self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime'] = time.time()
                    # print("\n[" + str(datetime.datetime.now()) + "] " + "Update ETA in the Coordination Request List at time " + str(time.time()))

    def deleteTimeOutRequestFromCoordinationRequestTable(self):
        """
        Method to delete the old coordination requests, if coordination split is zero
        """
        deleteCoordinationRequest = False
        if bool(self.coordinationPriorityRequestDictionary):
            noOfCoordinationRequest = self.coordinationPriorityRequestDictionary['noOfCoordinationRequest']
            requestorIndex = 0
            while requestorIndex < noOfCoordinationRequest:
                if self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][requestorIndex]['CoordinationSplit'] <= self.Request_Delete_Time:
                    del[self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][requestorIndex]]
                    self.coordinationPriorityRequestDictionary['noOfCoordinationRequest'] = noOfCoordinationRequest - 1
                    requestorIndex -= 1
                    noOfCoordinationRequest -= 1
                    deleteCoordinationRequest = True
                requestorIndex += 1
                
        return deleteCoordinationRequest

    def getCoordinationPriorityRequestDictionary(self):
        """
        Method to get the Coordination requests List after the deletion process 
        """
        noOfCoordinationRequest = self.coordinationPriorityRequestDictionary['noOfCoordinationRequest']

        self.coordinationPriorityRequestDictionary['minuteOfYear'] = self.getMinuteOfYear()
        self.coordinationPriorityRequestDictionary['msOfMinute'] = self.getMsOfMinute()
        for i in range(noOfCoordinationRequest):
            self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['priorityRequestType'] = self.requestUpdate
            self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime'] = time.time()
        self.requestSentTime = time.time()

        coordinationPriorityRequestJsonString = json.dumps(self.coordinationPriorityRequestDictionary)
        print("\n[" + str(datetime.datetime.now()) + "] " + "Coordination request List after deletion process at time " + str(time.time())+ " is following: \n", self.coordinationPriorityRequestDictionary)

        return coordinationPriorityRequestJsonString

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
        currentTime = timeNow.hour * float(HourToSecondConversion) + timeNow.minute * 60.0 + timeNow.second

        return currentTime

    def getMinuteOfYear(self):
        """
        Method for obtaining minute of a year based on current time
        """
        timeNow = datetime.datetime.now()
        dayOfYear = int(timeNow.strftime("%j"))
        currentHour = timeNow.hour
        currentMinute = timeNow.minute
        minuteOfYear = (dayOfYear - 1) * float(HOURSINADAY) * float(MINUTESINAHOUR) + currentHour * float(MINUTESINAHOUR) + currentMinute

        return minuteOfYear

    def getMsOfMinute(self):
        """
        Method for obtaining millisecond of a minute based on current time
        """
        timeNow = datetime.datetime.now()
        currentSecond = timeNow.second
        msOfMinute = currentSecond * float(SECONDTOMILISECOND)

        return msOfMinute