import datetime
import time
import json
from CoordinatedPhase import CoordinatedPhase


class CoordinationRequestManager:
    def __init__(self,config):
        # # Read the config file and store the configuration in an JSON object:
        # configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
        # config = (json.load(configFile))

        # # Close the config file:
        # configFile.close()
        self.config = config
        
        self.SRM_GAPOUT_TIME = 2.0
        self.requestTimedOutValue = self.config['SRMTimedOutTime'] - self.SRM_GAPOUT_TIME       
        
        #Define Coordination Parameters
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

    def getCoordinationParametersDictionary(self, dictionary):
        self.coordinationParametersDictionary = dictionary
        
    def checkCoordinationRequestSendingRequirement(self):
        if not bool(self.coordinationParametersDictionary):
            sendRequest = False
        else:
            currentTime = self.getCurrentTime()
            coordinationStartTime = self.coordinationParametersDictionary['CoordinationStartTime_Hour'] * \
                3600.0 + \
                self.coordinationParametersDictionary['CoordinationStartTime_Minute'] * 60.0
            cycleLength = self.coordinationParametersDictionary['CycleLength']
            
            remainderTime = currentTime % cycleLength
            print("Remainder Time is: ", remainderTime)

            if coordinationStartTime - currentTime >= 0 and coordinationStartTime - currentTime <= cycleLength:
                sendRequest = True

            elif cycleLength - remainderTime >= 0 and cycleLength - remainderTime <= 1.0:
                sendRequest = True
            
            else:
                sendRequest = False

        return sendRequest

    def generateVirtualCoordinationPriorityRequest(self):
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
        coordinationRequestDict = [dict() for x in range(len(coordinatedPhases))]
        for i in range(len(coordinatedPhases)):
            coordinationRequestDict[i] = coordinationRequestList[i].asDict()
        self.coordinationPriorityRequestDictionary = {
            "MsgType": "CoordinationRequest",
            "noOfCoordinationRequest": len(coordinatedPhases),
            "CoordinationRequestList":
                {
                    "minuteOfYear": 0,
                    "msOfMinute": 0,
                    "requestorInfo": coordinationRequestDict
                }

        }
        print("\nVirtual Coordination Request", self.coordinationPriorityRequestDictionary)
        self.requestSentTime = time.time()
        coordinationPriorityRequestJsonString = json.dumps(self.coordinationPriorityRequestDictionary)
        
        return coordinationPriorityRequestJsonString
    
    def checkUpdateRequestSendingRequirement(self):
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
        self.coordinationPriorityRequestDictionary['minuteOfYear'] = 0
        self.coordinationPriorityRequestDictionary['msOfMinute'] = 0
        for i in range(noOfCoordinationRequest):
            self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['priorityRequestType'] = self.requestUpdate
            self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime'] = time.time()
        self.requestSentTime = time.time()
        print ("\nUpdated Coordination Request to avoid PRS timed-out is following: \n",self.coordinationPriorityRequestDictionary)
        coordinationPriorityRequestJsonString = json.dumps(self.coordinationPriorityRequestDictionary)
    
        return coordinationPriorityRequestJsonString
    
    def updateETAInCoordinationRequestTable(self):
        """
        The following method updates the active coordination priority request and sends the updated request to PriorityRequestServer.

        If the ETA of a request is greater than the minimum ETA (predefined), the following method only updates the ETA.
        If the ETA of a Request is less or equal to the minimum ETA (preddefined), the following method updates the Coordination Split time and set the ETA as minimum ETA.
        """
        
        if bool(self.coordinationPriorityRequestDictionary):
            currentTime_UTC = time.time()
            noOfCoordinationRequest = self.coordinationPriorityRequestDictionary['noOfCoordinationRequest']
            for i in range(noOfCoordinationRequest):
                temporaryRequestUpdateTime = self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime']
                if currentTime_UTC - temporaryRequestUpdateTime >= self.ETA_Update_Time:
                    self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] = self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] - (currentTime_UTC - temporaryRequestUpdateTime)
                    if self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] <= self.Minimum_ETA:
                        self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['ETA'] = self.Minimum_ETA
                        self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['CoordinationSplit'] = self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['CoordinationSplit'] - (currentTime_UTC - temporaryRequestUpdateTime)
                    self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime'] = time.time()
                    print("\nUpdate ETA in the Coordination Request List: \n", self.coordinationPriorityRequestDictionary)
    
    def deleteTimeOutRequestFromCoordinationRequestTable(self):
        deleteCoordinationRequest = False
        if bool(self.coordinationPriorityRequestDictionary):
            noOfCoordinationRequest = self.coordinationPriorityRequestDictionary['noOfCoordinationRequest']
            # for i in range(noOfCoordinationRequest):
            i = 0
            while i < noOfCoordinationRequest:
                if self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['CoordinationSplit'] <= self.Request_Delete_Time:
                    del[self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]]
                    self.coordinationPriorityRequestDictionary['noOfCoordinationRequest'] = noOfCoordinationRequest - 1
                    i -= 1
                    noOfCoordinationRequest -= 1 
                    deleteCoordinationRequest = True
                i += 1
        return deleteCoordinationRequest
    
    def getCoordinationPriorityRequestDictionary(self):
        noOfCoordinationRequest = self.coordinationPriorityRequestDictionary['noOfCoordinationRequest']
        
        self.coordinationPriorityRequestDictionary['minuteOfYear'] = 0
        self.coordinationPriorityRequestDictionary['msOfMinute'] = 0
        for i in range(noOfCoordinationRequest):
            self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['priorityRequestType'] = self.requestUpdate
            self.coordinationPriorityRequestDictionary['CoordinationRequestList']['requestorInfo'][i]['requestUpdateTime'] = time.time()
        self.requestSentTime = time.time()
       
        coordinationPriorityRequestJsonString = json.dumps(self.coordinationPriorityRequestDictionary)
        print("\nCoordination request List after deletion process is following: \n", self.coordinationPriorityRequestDictionary)
       
        return coordinationPriorityRequestJsonString
    
    def getCurrentTime(self):
        timeNow = datetime.datetime.now()
        currentTime = timeNow.hour * 3600.0 + timeNow.minute * 60.0 + timeNow.second
        
        return currentTime