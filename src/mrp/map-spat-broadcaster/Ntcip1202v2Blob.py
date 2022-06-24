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

  Description:
  1. This is the initial revision. NTCIP1202v2Blob class does the following tasks:
    -> Read the NTCIP1202v2 blob received from a trafficController.
    -> Manually unpack the blob using byte-structure provided in Econolite SPAT Guide.
    -> API (as a minimum) for following information is provided:
        -> PhaseCurrentStatus: Required for MMITSS J2735SPAT and TrafficControllerInterface
        -> PhaseStatusStartTime: Required for MMITSS J2735SPAT and TrafficController
        -> PhaseStatusMinEndTime: Required for MMITSS J2735SPAT and TrafficController
        -> PhaseStatusMaxEndTime: Required for MMITSS J2735SPAT and TrafficController
        -> PhaseStatusElapsedTime: Required for TrafficControllerInterface
        -> Intersection status: Required in MMITSS J2735SPAT
        -> msOfMinute: Required for MMITSS J2735SPAT
        -> minuteOfYear: Required for MMITSS J2735SPAT
    -> minuteOfYear and msOfMinute is calculated from the MRP system time.
    -> Constructor requires no inputs. Just initializes the attributes.
    -> processNewData method needs to be called everytime a new blob is received. ReceivedBlob needs to be passed as a parameter for this method.
'''

import time
import datetime

class Ntcip1202v2Blob:


    def __init__(self, permissiveEnabled:dict, splitPhases:dict, inactiveVehPhases:list, inactivePedPhases:list):
        ################################################# CONSTANTS #################################################
        numVehPhases = 8
        numPedPhases = 8
        #############################################################################################################

        ########## Permissive Lefts ##########

        self.permissiveEnabled = permissiveEnabled
        self.splitPhases = splitPhases

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
        self.vehElapsedTimeInGMaxFlag = [False]*numVehPhases
        self.vehElapsedTimeInGMax = [0]*numVehPhases
        self.vehMinEndTime = [0]*numVehPhases

        self.vehMaxEndTime = [0]*numVehPhases
        self.vehPrevMaxEndTime = [0]*numVehPhases
        self.vehPrev2MaxEndTime = [0]*numVehPhases
        
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

        ########### Inactive Phases ###########
        self.inactiveVehPhases = inactiveVehPhases
        self.inactivePedPhases = inactivePedPhases

        ########### Intersection Status ###########
        self.intersectionStatus = ''

        ########### Time parameters ###########
        self.minuteOfYear = 0
        self.msOfMinute = 0

        ########### Current Phase Info ###########
        self.currentPhases = [0,0]

    def processNewData(self, receivedBlob):
        # Derived from system time (Not controller's time)
        currentTimeMs = int(round(time.time() * 10))
        startOfTheYear = datetime.datetime((datetime.datetime.now().year), 1, 1)
        timeSinceStartOfTheYear = (datetime.datetime.utcnow() - startOfTheYear)
        self.minuteOfYear = int(timeSinceStartOfTheYear.total_seconds()/60)
        self.msOfMinute = int((timeSinceStartOfTheYear.total_seconds() - (self.minuteOfYear * 60))*1000)
##################################### VEH INFORMATION ####################################################################
        # Phase status symbols: 
        RED = 'red'
        YELLOW = 'yellow'
        GREEN = 'green'
        PERMISSIVE = 'permissive_yellow'

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
        
        # Identify FIRST current phase (from ring 1)
        for i in range(0,4): 
            if self.vehCurrState[i] == GREEN:
                self.currentPhases[0] = (i+1)
        
        # Identify SECOND current phase (from ring 2)
        for i in range(4,8):
            if self.vehCurrState[i] == GREEN:
                self.currentPhases[1] = (i+1)
        
        # PhaseStatusPermissive:
        leftTurns = [1,3,5,7]
        for leftTurn in leftTurns:
            if self.permissiveEnabled[str(leftTurn)] == True:
                if ((self.vehCurrState[leftTurn-1] == RED) and (self.vehCurrState[self.splitPhases[str(leftTurn)]-1] == GREEN)):
                    self.vehCurrState[leftTurn-1] = PERMISSIVE
                
                elif ((self.vehCurrState[leftTurn-1] == RED) and (self.vehCurrState[self.splitPhases[str(leftTurn)]-1] == YELLOW)):
                    self.vehCurrState[leftTurn-1] = YELLOW    
   
        # Time since change to current state - check inactive phases first:
        for i in range(0,self.numVehPhases):
            if i+1 in self.inactiveVehPhases:
                self.vehElapsedTime[i] = 0.0            
            else:
                if self.vehCurrState[i] == self.vehPrevState[i]:
                    self.vehElapsedTime[i] = int(round(time.time() * 10)) - self.vehStartTime[i]
                else: 
                    self.vehStartTime[i] = int(round(time.time() * 10))
                    self.vehElapsedTime[i] = 0.0
                self.vehPrevState[i] = self.vehCurrState[i]

        # Minimum time to change from current state:
        for i in range(0,self.numVehPhases):
            firstByte = str(f'{receivedBlob[self.vehMinEndTimeByteMap[i][0]]:08b}')
            secondByte = str(f'{receivedBlob[self.vehMinEndTimeByteMap[i][1]]:08b}')
            completeByte = firstByte+secondByte            
            self.vehMinEndTime[i] = int(completeByte, 2)

        # Maximum time to change from current state:
        for i in range(0,self.numVehPhases):
            self.vehPrevMaxEndTime[i] = self.vehMaxEndTime[i]
            firstByte = str(f'{receivedBlob[self.vehMaxEndTimeByteMap[i][0]]:08b}')
            secondByte = str(f'{receivedBlob[self.vehMaxEndTimeByteMap[i][1]]:08b}')
            completeByte = firstByte+secondByte
            self.vehMaxEndTime[i] = int(completeByte, 2)

        # MinEndTime and MaxEndTime for Permissive:
        leftTurns = [1,3,5,7]
        for leftTurn in leftTurns:
            if self.permissiveEnabled[str(leftTurn)] == True:
                if ((self.vehCurrState[leftTurn-1] == PERMISSIVE) or 
                    (self.vehCurrState[leftTurn-1] == YELLOW and 
                     self.vehCurrState[self.splitPhases[str(leftTurn)]-1] == YELLOW)):
                          
                    self.vehMinEndTime[leftTurn-1] = self.vehMinEndTime[self.splitPhases[str(leftTurn)]-1]
                    self.vehMaxEndTime[leftTurn-1] = self.vehMaxEndTime[self.splitPhases[str(leftTurn)]-1]

        # Time elapsed since Gmax counter had began:
        for i in range(0,self.numVehPhases):
            if (self.vehCurrState[i] == GREEN):
                if (self.vehPrevState[i] != GREEN):
                    self.vehElapsedTimeInGMaxFlag[i] = False
                elif ((self.vehElapsedTimeInGMaxFlag[i] == False) and (self.vehMaxEndTime[i]!=self.vehPrevMaxEndTime[i]) and self.vehElapsedTime[i] > 0):                            
                    self.vehElapsedTimeInGMaxFlag[i] = True          
            else: 
                self.vehElapsedTimeInGMaxFlag[i] = False

        for i in range(0,self.numVehPhases):
            if (self.vehElapsedTimeInGMaxFlag[i] == True):
                self.vehElapsedTimeInGMax[i] += 1
            elif self.vehCurrState[i] != GREEN:
                self.vehElapsedTimeInGMax[i] = None
            else:
                self.vehElapsedTimeInGMax[i] = 0

        
        
##################################### PED INFORMATION ####################################################################
        DONTWALK = 'do_not_walk'
        PEDCLEAR = 'ped_clear'
        WALK = 'walk'
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
            if i+1 in self.inactivePedPhases:
                self.pedElapsedTime[i] = 0.0
            else:
                if self.pedCurrState[i] == self.pedPrevState[i]:
                    self.pedElapsedTime[i] = int(round(time.time() * 10)) - self.pedStartTime[i]
                else: 
                    self.pedStartTime[i] = int(round(time.time() * 10))
                    self.pedElapsedTime[i] = 0.0
                self.pedPrevState[i] = self.pedCurrState[i]
        
        # Minimum time to change from current state:
        for i in range(0,self.numPedPhases):
            firstByte = str(f'{receivedBlob[self.pedMinEndTimeByteMap[i][0]]:08b}')
            secondByte = str(f'{receivedBlob[self.pedMinEndTimeByteMap[i][1]]:08b}')
            completeByte = firstByte+secondByte
            self.pedMinEndTime[i] = int(completeByte, 2)

        # Maximum time to change from current state:
        for i in range(0,self.numPedPhases):
            firstByte = str(f'{receivedBlob[self.pedMinEndTimeByteMap[i][0]]:08b}')
            secondByte = str(f'{receivedBlob[self.pedMinEndTimeByteMap[i][1]]:08b}')
            completeByte = firstByte+secondByte
            self.pedMaxEndTime[i] = int(completeByte, 2)
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

    def getCurrentPhasesDict(self):
        gMaxEndTime = [None, None]

        if (self.vehCurrState[self.currentPhases[0]-1] == "green"):
            gMaxEndTime[0] = self.vehMaxEndTime[self.currentPhases[0]-1]
        if (self.vehCurrState[self.currentPhases[1]-1] == "green"):
            gMaxEndTime[1] = self.vehMaxEndTime[self.currentPhases[1]-1]
        
        currentPhasesDict = {
                                "currentPhases":
                                    [ 
                                        {   
                                            "Phase": self.currentPhases[0],
                                            "State": self.vehCurrState[self.currentPhases[0]-1],
                                            "ElapsedTime": self.vehElapsedTime[self.currentPhases[0]-1],
                                            "ElapsedTimeInGMax": self.vehElapsedTimeInGMax[self.currentPhases[0]-1],
                                            "RemainingGMax" : gMaxEndTime[0],
                                            "PedState": self.pedCurrState[self.currentPhases[0]-1],
                                        },
                                        {   
                                            "Phase": self.currentPhases[1],
                                            "State": self.vehCurrState[self.currentPhases[1]-1],
                                            "ElapsedTime": self.vehElapsedTime[self.currentPhases[1]-1],
                                            "ElapsedTimeInGMax": self.vehElapsedTimeInGMax[self.currentPhases[1]-1],
                                            "RemainingGMax" : gMaxEndTime[1],
                                            "PedState": self.pedCurrState[self.currentPhases[1]-1],
                                        }
                                    ]
                            }
        return currentPhasesDict


if __name__=="__main__":
    import json
    import socket

    # Read a config file by creating an object of the time MapSpatBroadcasterConfig
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))

    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    mrpIp = config["HostIp"]
    MapSpatBroadcastAddress = (mrpIp, config["PortNumber"]["MapSPaTBroadcaster"])
    s.bind(MapSpatBroadcastAddress)

    permissiveEnabled = config["SignalController"]["PermissiveEnabled"]
    splitPhases = config["SignalController"]["SplitPhases"]

    # Get inactive vehicle and ped phases from the configuration file
    inactiveVehPhases = config["SignalController"]["InactiveVehPhases"]
    inactivePedPhases = config["SignalController"]["InactivePedPhases"]

    # Create an empty Ntcip1202v2Blob object to store the information to be received from the signal controller:
    currentBlob = Ntcip1202v2Blob(permissiveEnabled, splitPhases, inactiveVehPhases, inactivePedPhases)

    while True:
        data, addr = s.recvfrom(1024)
        blobReceiptTime = time.time()
        currentBlob.processNewData(data)
        processingTime = time.time()-blobReceiptTime
        print(processingTime)