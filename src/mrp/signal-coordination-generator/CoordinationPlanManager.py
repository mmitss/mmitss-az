import json
import datetime
import time
from CoordinationPlan import CoordinationPlan


class CoordinationPlanManager:
    def __init__(self, data):
        self.data = data
        self.activePlanCheckingTime = 0.0
        self.timeGapBetweenActivePlanChecking = 1800
        self.coordinationPlanName = ""
        
    def checkactiveCoordinationPlan(self):
        activePlanCheckingRequirement = False
        currentTime = self.getCurrentTime()

        if currentTime - self.activePlanCheckingTime >= self.timeGapBetweenActivePlanChecking:
            activePlanCheckingRequirement = True

        return activePlanCheckingRequirement

    def getCoordinationParameters(self):
        parameterList = []
        for parameters in self.data['CoordinationParameters']:
            parametersDictionary = {
                # "CoordinationPlanName": parameters['CoordinationPlanName'],
                parameters['CoordinationPlanName']:
                {
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
            }

            parameterList.append(parametersDictionary)

        coordinationParametersDictionary = {"CoordinationParameters": parameterList}

        return coordinationParametersDictionary

    def getSplitData(self):
        coordinationSplitDataDictionary = {}
        phaseNumber = []
        splitTime = []
        for parameters in self.data['CoordinationParameters']:
            if self.coordinationPlanName == parameters['CoordinationPlanName']:
                for splitData in parameters['SplitPatternData']['PhaseNumber']:
                    phaseNumber.append(splitData)
                for splitData in parameters['SplitPatternData']['Split']:
                    splitTime.append(splitData)

        # coordinationSplitDataDictionary = dict({
        #     "MsgType": "ActiveCoordinationPlan",
        #     "TimingPlan":
        #         {
        #             "NoOfPhase": 8,
        #             "PhaseNumber": phaseNumber,
        #             "SplitData": splitTime
        #         }
        # })
        coordinationSplitDataDictionary = json.dumps({
            "MsgType": "ActiveCoordinationPlan",
            "TimingPlan":
                {
                    "NoOfPhase": 8,
                    "PhaseNumber": phaseNumber,
                    "SplitData": splitTime
                }
        })
        
        print("Coordination Split Data is following: \n", coordinationSplitDataDictionary)
        return coordinationSplitDataDictionary

    # print(json.dumps(coordinationParametersDictionary))
    # for element in data["CoordinationParameters"]:
    #     for splitData in element["SplitPatternData"]:
    #         for phase in splitData["CoordinatedPhase"]:
    #             print(phase[0])
    # print(data['IntersectionName'])
    # for parameters in data['CoordinationParameters']:
    #     # print(parameters['CoordinationPlanName'])
    #     for splitData in parameters['SplitPatternData']:
    # for parameters in data['CoordinationParameters']:
    #     for splitData in parameters['SplitPatternData']['CoordinatedPhase']:
    #         print(splitData)

    def getActiveCoordinationPlan(self):
        coordinationParametersDictionary = {}
        currentTime = self.getCurrentTime()

        for parameters in self.data['CoordinationParameters']:
            coordinationStartTime = parameters['CoordinationStartTime_Hour'] * \
                3600.0 + parameters['CoordinationStartTime_Minute'] * 60.0
            coordinationEndTime = parameters['CoordinationEndTime_Hour'] * \
                3600.0 + parameters['CoordinationEndTime_Minute'] * 60.0

            # print ("Coordination Start Time is", coordinationStartTime)
            cycleLength = parameters['CycleLength']
            #print (cycleLength)
            if currentTime >= coordinationStartTime and currentTime <= coordinationEndTime or abs(coordinationStartTime - currentTime) <= cycleLength:
                coordinationParametersDictionary = {
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
                self.activePlanCheckingTime = time.time()
                print("Active Coordination Parameter is following: \n",
                      coordinationParametersDictionary)
                break

        return coordinationParametersDictionary

    def getCurrentTime(self):
        timeNow = datetime.datetime.now()
        currentTime = timeNow.hour * 3600.0 + timeNow.minute * 60.0 + timeNow.second

        return currentTime
