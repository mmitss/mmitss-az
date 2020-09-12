'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  Spat.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision. Spat class does the following tasks:
    -> Provides an API for setting regionaID, IntersectionID, msgCount, and for filling the SPAT information from the controller.
    -> Constructor needs no parameters. Only initializes the attributes.
    -> fillSpatInformation needs an object of NTCIP1202Blob class as an input parameter.
    -> Based on set information, spat2Json method returns a JSON string contaiing the filled information. This method needs to be called after setting all attributes.
    -> If NTCIP1202 standard changes, a similar class needs to be developed with similar API, for other components to run smooth.
'''

from Ntcip1202v2Blob import Ntcip1202v2Blob
import json
from Phase import Phase
import datetime

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
        self.vehMinEndTimeList = 0
        self.vehMaxEndTimeList = 0

    def setRegionalID(self, regionalID:int):
        self.regionalID = regionalID
    
    def setIntersectionID(self, intersectionID:int):
        self.intersectionID = intersectionID

    def setmsgCnt(self, msgCnt:int):
        self.msgCnt = msgCnt

    def fillSpatInformation(self, spatBlob:Ntcip1202v2Blob):
        self.minuteOfYear = spatBlob.getMinuteOfYear()
        self.msOfMinute = spatBlob.getMsOfMinute()
        self.intersectionStatus = spatBlob.getIntersectionStatus()
        
        vehCurrStateList = self.getVehCurrStateList(spatBlob)
        vehStartTimeList = self.getVehStartTimeList(spatBlob)
        vehMinEndTimeList = self.getVehMinEndTimeList(spatBlob)
        vehMaxEndTimeList = self.getVehMaxEndTimeList(spatBlob)
        vehElapsedTimeList = self.getVehElapsedTimeList(spatBlob)

        pedCurrStateList = self.getPedCurrStateList(spatBlob)
        pedStartTimeList = self.getPedStartTimeList(spatBlob)
        pedMinEndTimeList = self.getPedMinEndTimeList(spatBlob)
        pedMaxEndTimeList = self.getPedMaxEndTimeList(spatBlob)
        pedElapsedTimeList = self.getPedElapsedTimeList(spatBlob)
        
        for i in range(8):
            tempPhase = Phase()
            tempPhase.phaseNo = i+1
            tempPhase.currState = vehCurrStateList[i]
            tempPhase.startTime = vehStartTimeList[i]
            tempPhase.minEndTime = vehMinEndTimeList[i]
            tempPhase.maxEndTime = vehMaxEndTimeList[i]
            tempPhase.elapsedTime = vehElapsedTimeList[i]
            self.vehPhases[i] = tempPhase
            
            tempPhase = Phase()
            tempPhase.phaseNo = i+1
            tempPhase.currState = pedCurrStateList[i]
            tempPhase.startTime = pedStartTimeList[i]
            tempPhase.minEndTime = pedMinEndTimeList[i]
            tempPhase.maxEndTime = pedMaxEndTimeList[i]
            tempPhase.elapsedTime = pedElapsedTimeList[i]
            self.pedPhases[i] = tempPhase

    def getVehCurrStateList(self, spatBlob:Ntcip1202v2Blob):
        vehCurrStateList = spatBlob.getVehCurrState()
        return vehCurrStateList

    def getVehStartTimeList(self, spatBlob:Ntcip1202v2Blob):
        vehStartTimeList = spatBlob.getVehStartTime()
        return vehStartTimeList

    def getVehMinEndTimeList(self, spatBlob:Ntcip1202v2Blob):
        vehMinEndTimeList = spatBlob.getVehMinEndTime()
        self.vehMinEndTimeList = vehMinEndTimeList
        return vehMinEndTimeList

    def getVehMaxEndTimeList(self, spatBlob:Ntcip1202v2Blob):
        vehMaxEndTimeList = spatBlob.getVehMaxEndTime()
        self.vehMaxEndTimeList = vehMaxEndTimeList
        return vehMaxEndTimeList

    def getVehElapsedTimeList(self, spatBlob:Ntcip1202v2Blob):
        vehElapsedTimeList = spatBlob.getVehElapsedTime()
        return vehElapsedTimeList

    def getPedCurrStateList(self, spatBlob:Ntcip1202v2Blob):
        pedCurrStateList = spatBlob.getPedCurrState()
        return pedCurrStateList

    def getPedStartTimeList(self, spatBlob:Ntcip1202v2Blob):
        pedStartTimeList = spatBlob.getPedStartTime()
        return pedStartTimeList

    def getPedMinEndTimeList(self, spatBlob:Ntcip1202v2Blob):
        pedMinEndTimeList = spatBlob.getPedMinEndTime()
        return pedMinEndTimeList

    def getPedMaxEndTimeList(self, spatBlob:Ntcip1202v2Blob):
        pedMaxEndTimeList = spatBlob.getPedMaxEndTime()
        return pedMaxEndTimeList

    def getPedElapsedTimeList(self, spatBlob:Ntcip1202v2Blob):
        pedElapsedTimeList = spatBlob.getPedElapsedTime()
        return pedElapsedTimeList


    def Spat2Json(self):
        for i in range(8):
            self.vehPhasesDict[i] = self.vehPhases[i].asDict()
            self.pedPhasesDict[i] = self.pedPhases[i].asDict()
        spatDict = dict({"MsgType": "SPaT",
                    "Timestamp_verbose": str(datetime.datetime.now()),
                    "Timestamp_posix": datetime.datetime.now().timestamp(),
                    "Spat" :
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

