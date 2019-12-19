'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  NTCIP1202v2Blob.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision. NTCIP1202v2Blob class does the following tasks:
    -> Read the NTCIP1202v2 blob received from a trafficController.
    -> Manually unpack the blob using byte-structure provided in Econolite SPAT Guide.
    -> API (as a minimum) for following information is provided:
        -> PhaseCurrentStatus: Required for MMITSS J2735SPAT and TrafficController Observer
        -> PhaseStatusStartTime: Required for MMITSS J2735SPAT and TrafficController Observer
        -> PhaseStatusMinEndTime: Required for MMITSS J2735SPAT and TrafficController Observer
        -> PhaseStatusMaxEndTime: Required for MMITSS J2735SPAT and TrafficController Observer
        -> PhaseStatusElapsedTime: Required for TrafficController Observer
        -> Intersection status: Required in MMITSS J2735SPAT and TrafficController Observer
        -> msOfMinute: Required for MMITSS J2735SPAT and TrafficController Observer
        -> minuteOfYear: Required for MMITSS J2735SPAT and TrafficController Observer
    -> minuteOfYear and msOfMinute is calculated from the MRP system time.
    -> Constructor requires no inputs. Just initializes the attributes.
    -> processNewData method needs to be called everytime a new blob is received. ReceivedBlob needs to be passed as a parameter for this method.
'''

import time
import datetime

class Ntcip1202v2Blob:


    def __init__(self):
        ################################################# CONSTANTS #################################################
        numVehPhases = 8
        numPedPhases = 8
        #############################################################################################################
        ########### Vehicle Phases ###########        
        self.numVehPhases = numVehPhases
        # Individual Colours
        self.vehPhaseStatusReds = [0]*numVehPhases
        self.vehPhaseStatusYellows = [0]*numVehPhases
        self.vehPhaseStatusGreens = [0]*numVehPhases
        # Phase status
        self.vehCurrState = [0]*numVehPhases
        self.vehPrevState = [0]*numVehPhases
        # Values required in J2735 SPaT Message
        self.vehStartTime = [0]*numVehPhases
        self.vehElapsedTime = [0]*numVehPhases
        self.vehMinEndTime = [0]*numVehPhases
        self.vehMaxEndTime = [0]*numVehPhases
        self.vehMinEndTimeByteMap = [[3,4],[16,17],[29,30],[42,43],[55,56],[68,69],[81,82],[94,95]]      
        self.vehMaxEndTimeByteMap = [[5,6],[18,19],[31,32],[44,45],[57,58],[70,71],[83,84],[96,97]]

        ########### Pedestrian Phases ###########
        self.numPedPhases = numPedPhases
        # Individual Colours
        self.pedPhaseStatusDontWalks = [0]*numPedPhases
        self.pedPhaseStatusPedClears = [0]*numPedPhases
        self.pedPhaseStatusWalks = [0]*numPedPhases
        # Phase status
        self.pedCurrState = [0]*numPedPhases
        self.pedPrevState = [0]*numPedPhases
        # Values required in J2735 SPaT Message
        self.pedStartTime = [0]*numPedPhases
        self.pedElapsedTime = [0]*numPedPhases
        self.pedMinEndTime = [0]*numPedPhases
        self.pedMaxEndTime = [0]*numPedPhases
        self.pedMinEndTimeByteMap = [[7,8],[20,21],[33,34],[46,47],[59,60],[72,73],[85,86],[98,99]]      
        self.pedMaxEndTimeByteMap = [[9,10],[22,23],[35,36],[48,49],[61,62],[74,75],[87,88],[100,101]]

        ########### Intersection Status ###########
        self.intersectionStatus = ''

        ########### Time parameters ###########
        self.minuteOfYear = 0
        self.msOfMinute = 0

    def processNewData(self, receivedBlob):
        # Derived from system time (Not controller's time)
        currentTimeMs = int(round(time.time() * 10))
        startOfTheYear = datetime.datetime((datetime.datetime.now().year), 1, 1)
        timeSinceStartOfTheYear = (datetime.datetime.now() - startOfTheYear)
        self.minuteOfYear = int(timeSinceStartOfTheYear.total_seconds()/60)
        self.msOfMinute = int((timeSinceStartOfTheYear.total_seconds() - (self.minuteOfYear * 60))*1000)
##################################### VEH INFORMATION ####################################################################
        # Phase status symbols: 
        RED = 3
        YELLOW = 8
        GREEN = 6

        # PhaseStatusRed:
        vehPhaseStatusRedStr = str(f'{receivedBlob[211]:08b}')[::-1]
        for i in range(0,self.numVehPhases):
            if vehPhaseStatusRedStr[i] == '1':
                self.vehPhaseStatusReds[i] = True
                self.vehCurrState[i] = RED
        
        # PhaseStatusYellow:
        vehPhaseStatusYellowStr = str(f'{receivedBlob[213]:08b}')[::-1]
        for i in range(0,self.numVehPhases):
            if vehPhaseStatusYellowStr[i] == '1':
                self.vehPhaseStatusYellows[i] = True
                self.vehCurrState[i] = YELLOW

        # PhaseStatusGreen:
        vehPhaseStatusGreenStr = str(f'{receivedBlob[215]:08b}')[::-1]
        for i in range(0,self.numVehPhases):
            if vehPhaseStatusGreenStr[i] == '1':
                self.vehPhaseStatusGreens[i] = True
                self.vehCurrState[i] = GREEN

        # Time since change to current state - check inactive phases first:
        for i in range(0,self.numVehPhases):
            if self.vehMinEndTime[i] == 0 and self.vehMaxEndTime[i] == 0:
                self.vehElapsedTime[i] = 0.0            
            else:
                if self.vehCurrState[i] == self.vehPrevState[i]:
                    self.vehElapsedTime[i] = currentTimeMs - self.vehStartTime[i]
                else: 
                    self.vehStartTime[i] = currentTimeMs
                    self.vehElapsedTime[i] = 0.0
                self.vehPrevState[i] = self.vehCurrState[i]
        
        # Minimum time to change from current state:
        for i in range(0,self.numVehPhases):
            self.vehMinEndTime[i] = int(receivedBlob[self.vehMinEndTimeByteMap[i][1]])

        # Maximum time to change from current state:
        for i in range(0,self.numVehPhases):
            self.vehMaxEndTime[i] = int(receivedBlob[self.vehMaxEndTimeByteMap[i][1]])
##################################### PED INFORMATION ####################################################################
        DONTWALK = 3
        PEDCLEAR = 8
        WALK = 6
        # PhaseStatusDontWalk:
        pedPhaseStatusDontWalkStr = str(f'{receivedBlob[217]:08b}')[::-1]
        for i in range(0,self.numPedPhases):
            if pedPhaseStatusDontWalkStr[i] == '1':
                self.pedPhaseStatusDontWalks[i] = True
                self.pedCurrState[i] = DONTWALK
        
        #PhaseStatusPedClear:
        pedPhaseStatusPedClearStr = str(f'{receivedBlob[219]:08b}')[::-1]
        for i in range(0,self.numPedPhases):
            if pedPhaseStatusPedClearStr[i] == '1':
                self.pedPhaseStatusPedClears[i] = True
                self.pedCurrState[i] = PEDCLEAR

        #PhaseStatusWalk:
        pedPhaseStatusWalkStr = str(f'{receivedBlob[221]:08b}')[::-1]
        for i in range(0,self.numPedPhases):
            if pedPhaseStatusWalkStr[i] == '1':
                self.pedPhaseStatusWalks[i] = True
                self.pedCurrState[i] = WALK
        
        # Time since change to current state - check inactive phases first!:
        for i in range(0,self.numPedPhases):
            if self.pedMinEndTime[i] == 0 and self.pedMaxEndTime[i] == 0:
                self.pedElapsedTime[i] = 0.0
            else:
                if self.pedCurrState[i] == self.pedPrevState[i]:
                    self.pedElapsedTime[i] = currentTimeMs - self.pedStartTime[i]
                else: 
                    self.pedStartTime[i] = currentTimeMs
                    self.pedElapsedTime[i] = 0.0
                self.pedPrevState[i] = self.pedCurrState[i]
        
        # Minimum time to change from current state:
        for i in range(0,self.numPedPhases):
            self.pedMinEndTime[i] = int(receivedBlob[self.pedMinEndTimeByteMap[i][1]])

        # Maximum time to change from current state:
        for i in range(0,self.numPedPhases):
            self.pedMaxEndTime[i] = int(receivedBlob[self.pedMaxEndTimeByteMap[i][1]])
##################################### INTERSECTION STATUS ####################################################################

        self.intersectionStatus = '00000000' + str(f'{receivedBlob[232]:08b}')        
        
    def getVehCurrState(self):
        return self.vehCurrState

    def getVehStartTime(self):
        return self.vehStartTime
    
    def getVehElapsedTime(self):
        return self.vehElapsedTime

    def getVehMinEndTime(self):
        return self.vehMinEndTime

    def getVehMaxEndTime(self):
        return self.vehMaxEndTime

    def getPedCurrState(self):
        return self.pedCurrState
    
    def getPedStartTime(self):
        return self.pedStartTime
    
    def getPedElapsedTime(self):
        return self.pedElapsedTime

    def getPedMinEndTime(self):
        return self.pedMinEndTime

    def getPedMaxEndTime(self):
        return self.pedMaxEndTime
    
    def getIntersectionStatus(self):
        return self.intersectionStatus

    def getMinuteOfYear(self):
        return self.minuteOfYear

    def getMsOfMinute(self):
        return self.msOfMinute
