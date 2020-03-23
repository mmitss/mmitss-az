"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

Scheduler.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This class is responsible for processing the schedule received from the Priority Solver 
or any other MMITSS component. For processing following steps are followed:
(1) All commands in the schedule are sorted based on the command start time.
(2) Formulate the groups of commands having same start time and same action.
(3) Create the bitstring based on the phases included in the group, and formulate 
corresponding integer representation. Formulate the corresponding group command.
(4) Run an instance of background scheduler and based on the start time and the end time 
of the group command and the NTCIP backup time, schedule a SignalController::phaseControl 
function in the background scheduler.
(5) If the schedule is new, make sure not to stop the currently active actions in the NTCIP
mode of the signal controller, before executing the new schedule.
(6) At exit, clear the NTCIP commands in the signal controller, and in the background scheduler.
***************************************************************************************
"""

import json
import datetime
import atexit
from apscheduler.schedulers.background import BackgroundScheduler
from Snmp import Snmp
from Command import Command
from SignalController import SignalController

class Scheduler:
    def __init__(self, snmp:Snmp, signalController:SignalController, ntcipBackupTime_Sec:int):
        # NTCIP Backup Time
        self.ntcipBackupTime_Sec = ntcipBackupTime_Sec
        
        # Create an object of SnmpApi
        self.snmp = snmp
        self.signalController = signalController

        # Scheduler parameters
        self.backgroundScheduler = BackgroundScheduler() 
        self.backgroundScheduler.start()
        
        self.commandId = 0
        self.currentCommandPool = []
        
        self.scheduleExecutionCompletion = True

        # Ensure that the scheduler shuts down when the app is exited
        atexit.register(lambda: self.stopbackgroundScheduler())
    
    def markExecutionCompletion(self):
        self.scheduleExecutionCompletion = True

    def processReceivedSchedule(self, receivedSchedule:json):
        
        self.scheduleExecutionCompletion = False

        
        
        receivedSchedule = receivedSchedule["Schedule"]
        maxEndTime = max([x['commandEndTime'] for x in receivedSchedule])
        print(maxEndTime)
        
        # Sort the schedule by three levels: 1. Command Start Time, 2. Command Type, and 3. Command End Time
        receivedSchedule = sorted(receivedSchedule, key = lambda i: (i["commandStartTime"]))

        # Read the json into a data structure
        scheduleDataStructure = self.createScheduleDataStructure(receivedSchedule)

        # Form action group
        index = 0
        groupIndex = 0
        while index < len(scheduleDataStructure):
            
            currentGroup = [scheduleDataStructure[index]]

            # Look for all commands in the list that has same start time and the type
            for command in scheduleDataStructure:
                if (command != currentGroup[0] and (command.startTime == currentGroup[0].startTime) and (command.action == currentGroup[0].action)):
                    currentGroup = currentGroup + [command]
                    index = index+1

            groupCommand =  self.formulateGroupCommand(currentGroup)

            # Check the end times of all items in the current group and add additional commands for the phases that end later.
            for command in currentGroup:
                if command.endTime > groupCommand.endTime:
                    scheduleDataStructure.append(Command(command.phase,command.action,groupCommand.endTime,command.endTime))

            groupMaxEndTime = max(command.endTime for command in currentGroup)
            

            groupEndCommand = Command(0,groupCommand.action,groupMaxEndTime,groupMaxEndTime)
            scheduleDataStructure.append(groupEndCommand)

            # Resort the schedule data structure based on command start time
            scheduleDataStructure.sort(key=lambda x: x.startTime, reverse=False)

            # If this is the first commandGroup of the schedule, check if it is already being executed inside the signal controller.

            if groupIndex == 0:
                # Check if the current groupCommand is already running in the signalController:
                    
                if False:
                    pass
                else:
                    self.clearBackgroundScheduler()
                    self.addCommandToSchedule(groupCommand)
            else: 
                self.addCommandToSchedule(groupCommand)
                
            currentGroup = []
            index = index + 1
            groupIndex = groupIndex + 1




    # Create Schedule data structure
    def createScheduleDataStructure(self, scheduleJson:json):
        scheduleDataStructure = []
        for command in scheduleJson:
            commandObject = Command(command["commandPhase"], command["commandType"],  command["commandStartTime"], command["commandEndTime"])
            scheduleDataStructure = scheduleDataStructure + [commandObject]

        return scheduleDataStructure


    # Formulate group command:
    def formulateGroupCommand(self, currentGroup:list):
        groupCommand = currentGroup[0].action
        groupStartTime = currentGroup[0].startTime
        groupEndTime = min(command.endTime for command in currentGroup )
        groupPhases = []
        for command in currentGroup:
            groupPhases = groupPhases + [command.phases]

        groupPhaseInt = self.formulateBinaryIntegerRepresentation(groupPhases)

        groupCommand = Command(groupPhaseInt, groupCommand, groupStartTime, groupEndTime)
        return groupCommand
    

    # Formulate the binary integer representation of the commandPhase:
    def formulateBinaryIntegerRepresentation(self, groupPhases):
        groupPhaseStr = list("00000000")
        for phase in groupPhases:
            groupPhaseStr[phase-1]="1"
        groupPhaseStr = groupPhaseStr[::-1]
        groupPhaseStr = "".join(groupPhaseStr)
        groupPhaseInt = int(groupPhaseStr,2)
        return groupPhaseInt    

    
    '''##############################################
                    Scheduler Methods
    ##############################################'''
    
    def addCommandToSchedule(self, commandObject:Command):

        # Define Command Types:
        # Assign Command ID:       
        if self.commandId > 65534:
            self.commandId = 0
        self.commandId = self.commandId + 1

        self.backgroundScheduler.add_job(self.signalController.phaseControl(), args = [commandObject.action, commandObject.phases], 
                trigger = 'interval',
                seconds = self.ntcipBackupTime_Sec-1,
                start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                id = str(self.commandId))
        return self.commandId

    def stopbackgroundScheduler(self):
        self.clearBackgroudScheduler()
        self.clearNtcipCommandsFromSignalController()
        self.backgroundScheduler.shutdown(wait=False)
        
    def clearBackgroundScheduler(self):
        self.backgroundScheduler.remove_all_jobs()
        
    def clearNtcipCommandsFromSignalController(self):    
        self.signalController.phaseControl(1,0)
        self.signalController.phaseControl(2,0)
        self.signalController.phaseControl(3,0)
        self.signalController.phaseControl(4,0)
        self.signalController.phaseControl(5,0)
        self.signalController.phaseControl(6,0)

'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    import time

    # Define controller's communication Info
    controllerIp = "10.12.6.17"
    controllerPort = 501
    controllerCommInfo = (controllerIp, controllerPort)

    # Create an object of Scheduler class
    scheduler = Scheduler(controllerCommInfo, 10,1)

    # Open a dummy schedule and load it into a json object
    scheduleFile = open("schedule.json", "r")
    scheduleJson = json.loads(scheduleFile.read())

    scheduler.processReceivedSchedule(scheduleJson)

    # Schedule a vehicle call on all phases after 10 seconds
    # scheduler.addCommandToSchedule(Command(255,10,10,6))
    time.sleep(100)