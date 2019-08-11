import json

class MapSpatBroadcasterConfig:
    def __init__(self, nameOfConfigFile:str):
        f = open(nameOfConfigFile, 'r')
        configJson = json.load(f)
        self.intersectionID = configJson["MapSpatBroadcasterConfig"]["intersectionID"]
        self.regionalID = configJson["MapSpatBroadcasterConfig"]["regionalID"]
        self.mapPayload = configJson["MapSpatBroadcasterConfig"]["mapPayload"]
        self.controllerIP = configJson["MapSpatBroadcasterConfig"]["controllerIP"]
        self.mrpIP = configJson["MapSpatBroadcasterConfig"]["mrpIP"]

    def getIntersectionID(self):
        return self.intersectionID
    
    def getRegionalID(self):
        return self.regionalID

    def getMapPayload(self):
        return self.mapPayload

    def getControllerIP(self):
        return self.controllerIP

    def getMrpIP(self):
        return self.mrpIP