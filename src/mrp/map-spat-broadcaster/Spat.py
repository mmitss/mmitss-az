from Ntcip1202v2Blob import Ntcip1202v2Blob
import json
from Phase import Phase

class Spat:
    def __init__(self):
        self.regionalID = 0
        self.intersectionID = 0
        self.msgCnt = 0
        self.vehPhases = [0 for x in range(8)]
        self.pedPhases = [0 for x in range(8)]
        self.vehPhasesDict = [dict() for x in range(8)]
        self.pedPhasesDict = [dict() for x in range(8)]
        self.spatJson = ''
        self.minuteOfYear = 0
        self.msOfMinute = 0
        self.intersectionStatus = ''

    def setRegionalID(self, regionalID:int):
        self.regionalID = regionalID
    
    def setIntersectionID(self, intersectionID:int):
        self.intersectionID = intersectionID

    def setmsgCnt(self, msgCnt:int):
        self.msgCnt = msgCnt

    def fillSpatInformation(self, ntcip1202v2Blob:Ntcip1202v2Blob):
        self.minuteOfYear = ntcip1202v2Blob.getMinuteOfYear()
        self.msOfMinute = ntcip1202v2Blob.getMsOfMinute()
        self.intersectionStatus = ntcip1202v2Blob.getIntersectionStatus()
        for i in range(8):
            tempPhase = Phase()
            tempPhase.phaseNo = i+1
            tempPhase.currState = ntcip1202v2Blob.getVehCurrState()[i]
            tempPhase.startTime = ntcip1202v2Blob.getVehStartTime()[i]
            tempPhase.minEndTime = ntcip1202v2Blob.getVehMinEndTime()[i]
            tempPhase.maxEndTime = ntcip1202v2Blob.getVehMaxEndTime()[i]
            tempPhase.elapsedTime = ntcip1202v2Blob.getVehElapsedTime()[i]
            self.vehPhases[i] = tempPhase
            
            tempPhase = Phase()
            tempPhase.phaseNo = i+1
            tempPhase.currState = ntcip1202v2Blob.getPedCurrState()[i]
            tempPhase.startTime = ntcip1202v2Blob.getPedStartTime()[i]
            tempPhase.minEndTime = ntcip1202v2Blob.getPedMinEndTime()[i]
            tempPhase.maxEndTime = ntcip1202v2Blob.getPedMaxEndTime()[i]
            tempPhase.elapsedTime = ntcip1202v2Blob.getPedElapsedTime()[i]
            self.pedPhases[i] = tempPhase

    def Spat2Json(self):
        for i in range(8):
            self.vehPhasesDict[i] = self.vehPhases[i].asDict()
            self.pedPhasesDict[i] = self.pedPhases[i].asDict()
        spatDict = dict({"Spat" :
                    {
                        "IntersectionState" :
                        {
                            "regionalID" : self.regionalID,
                            "intersectionID" : self.intersectionID
                        },
                        "msgCnt" : self.msgCnt,
                        "minuteOfYear" : self.minuteOfYear,
                        "msOfMinute" : self.msOfMinute,
                        "status" : self.intersectionStatus,
                        "phaseState": self.vehPhasesDict,
                        "pedPhaseState": self.pedPhasesDict
                    }
                })
        return json.dumps(spatDict)

    def Json2Spat(self, spatJson:json):
        self.spatJson = spatJson

