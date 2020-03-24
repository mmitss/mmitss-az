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
    def __init__(self, signalController:SignalController):
        # NTCIP Backup Time
        self.commandId = 0        
        # Create an object of SnmpApi
        self.snmp = snmp
        self.signalController = signalController
        self.ntcipBackupTime_Sec = signalController.ntcipBackupTime_sec

        # Scheduler parameters
        self.backgroundScheduler = BackgroundScheduler() 
        self.backgroundScheduler.start()
        self.scheduleTimingPlanUpdate(self.signalController.timingPlanUpdateInterval_sec)
        

        self.currentCommandPool = []
        
        self.scheduleExecutionCompletion = True

        # Ensure that the scheduler shuts down when the app is exited
        atexit.register(lambda: self.stopBackgroundScheduler())

    def clearOldSchedule(self, scheduleDataStructure):        

        self.clearBackgroundScheduler(True)
        
        clearHolds = True
        clearPedOmit = True
        clearVehOmit = True

        for command in scheduleDataStructure:
            if command.startTime == 0 or command.startTime == 0.0:
                if command.action == command.HOLD_VEH_PHASES:
                    if self.signalController.getPhaseControl(command.HOLD_VEH_PHASES) > 0:
                        clearHolds = False
                elif command.action == command.OMIT_VEH_PHASES:    
                    if self.signalController.getPhaseControl(command.OMIT_VEH_PHASES) > 0:
                        clearVehOmit = False
                elif command.action == command.OMIT_PED_PHASES:
                    if self.signalController.getPhaseControl(command.OMIT_PED_PHASES) > 0:
                        clearPedOmit = False

        if clearHolds == True:
            self.signalController.setPhaseControl(command.HOLD_VEH_PHASES,0)    
            
        if clearVehOmit == True:
            self.signalController.setPhaseControl(command.OMIT_VEH_PHASES,0)    
        
        if clearPedOmit == True:
            self.signalController.setPhaseControl(command.OMIT_PED_PHASES,0)    

        # Clear VehCalls
        self.signalController.setPhaseControl(command.CALL_VEH_PHASES,0)
        # Clear PedCalls
        self.signalController.setPhaseControl(command.CALL_PED_PHASES,0)
        # Clear Forceoffs
        self.signalController.setPhaseControl(command.FORCEOFF_PHASES,0)        

    def processReceivedSchedule(self, scheduleJson:json):
        # Formulate group command:
        def formulateGroupCommand(currentGroup:list):

            # Formulate the binary integer representation of the commandPhase:
            def formulateBinaryIntegerRepresentation(groupPhases):
                groupPhaseStr = list("00000000")
                
                if groupPhases[0] != 0:
                    for phase in groupPhases:
                        groupPhaseStr[phase-1]="1"
                groupPhaseStr = groupPhaseStr[::-1]
                groupPhaseStr = "".join(groupPhaseStr)
                groupPhaseInt = int(groupPhaseStr,2)
                
                return groupPhaseInt    


            groupCommand = currentGroup[0].action
            groupStartTime = currentGroup[0].startTime
            groupEndTime = min(command.endTime for command in currentGroup )
            groupPhases = []
            for command in currentGroup:
                groupPhases = groupPhases + [command.phases]

            groupPhaseInt = formulateBinaryIntegerRepresentation(groupPhases)

            groupCommand = Command(groupPhaseInt, groupCommand, groupStartTime, groupEndTime)
            return groupCommand
            
        # Create Schedule data structure
        def createScheduleDataStructure(scheduleJson:json):
            scheduleDataStructure = []
            for command in scheduleJson:
                commandObject = Command(command["commandPhase"], command["commandType"],  command["commandStartTime"], command["commandEndTime"])
                scheduleDataStructure = scheduleDataStructure + [commandObject]

            return scheduleDataStructure


        # Sort the schedule by three levels: 1. Command Start Time, 2. Command Type, and 3. Command End Time
        scheduleJson = scheduleJson["Schedule"]
        scheduleJson = sorted(scheduleJson, key = lambda i: (i["commandStartTime"]))

        # Read the json into a data structure
        scheduleDataStructure = createScheduleDataStructure(scheduleJson)
        
        self.clearOldSchedule(scheduleDataStructure)    

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

            groupCommand =  formulateGroupCommand(currentGroup)

            # Check the end times of all items in the current group and add additional commands for the phases that end later.
            for command in currentGroup:
                if command.endTime > groupCommand.endTime:
                    scheduleDataStructure.append(Command(command.phases,command.action,groupCommand.endTime,command.endTime))

            groupMaxEndTime = max(command.endTime for command in currentGroup)
            groupEndCommand = Command(0,command.action,groupMaxEndTime,groupMaxEndTime)

            # Add the group end command at the endTime of the group only if it does not already exist in the scheduleDataStructure
            addEndCommandFlag = any((elem.phases==groupEndCommand.phases and elem.action==groupEndCommand.action and elem.startTime==groupEndCommand.startTime) for elem in scheduleDataStructure)
            if addEndCommandFlag == False:
                scheduleDataStructure.append(groupEndCommand)

            # Resort the schedule data structure based on command start time
            scheduleDataStructure.sort(key=lambda x: x.startTime, reverse=False)

            self.addCommandToSchedule(groupCommand)
                
            currentGroup = []
            index = index + 1
            groupIndex = groupIndex + 1    


    
    '''##############################################
                    Scheduler Methods
    ##############################################'''
    
    def addCommandToSchedule(self, commandObject:Command):

        # Define Command Types:
        # Assign Command ID:       
        if self.commandId > 65534:
            self.commandId = 0
        self.commandId = self.commandId + 1

        if commandObject.phases == 0:
            self.backgroundScheduler.add_job(self.signalController.setPhaseControl, args = [commandObject.action, commandObject.phases], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId
        
        else: 
            self.backgroundScheduler.add_job(self.signalController.setPhaseControl, args = [commandObject.action, commandObject.phases], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            self.commandId = self.commandId + 1            
            
            self.backgroundScheduler.add_job(self.signalController.setPhaseControl, args = [commandObject.action, commandObject.phases], 
                    trigger = 'interval',
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                    id = str(self.commandId))
            return self.commandId

    def scheduleTimingPlanUpdate(self, interval:int):
        self.backgroundScheduler.add_job(self.signalController.updateActiveTimingPlan,
                    trigger = 'interval',
                    seconds = interval,
                    id = str(self.commandId))
        return self.commandId

    def stopBackgroundScheduler(self):
        command = Command(0,0,0,0)
        self.clearBackgroundScheduler(False)

        # Clear VehCalls
        self.signalController.setPhaseControl(command.CALL_VEH_PHASES,0)
        # Clear PedCalls
        self.signalController.setPhaseControl(command.CALL_PED_PHASES,0)
        # Clear Forceoffs
        self.signalController.setPhaseControl(command.FORCEOFF_PHASES,0)
        # Clear Holds
        self.signalController.setPhaseControl(command.HOLD_VEH_PHASES,0)
        # Clear VehOmits
        self.signalController.setPhaseControl(command.OMIT_VEH_PHASES,0)
        # Clear PedOmits
        self.signalController.setPhaseControl(command.OMIT_PED_PHASES,0)
        # Clear background scheudler
        self.backgroundScheduler.shutdown(wait=False)
        
    def clearBackgroundScheduler(self, rescheduleTimingPlanUpdate:bool):
        self.backgroundScheduler.remove_all_jobs()
        if rescheduleTimingPlanUpdate==True:
            self.scheduleTimingPlanUpdate(self.signalController.timingPlanUpdateInterval_sec)

'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    import time

    # Define controller's communication Info
    controllerIp = "10.12.6.17"
    controllerPort = 501
    controllerCommInfo = (controllerIp, controllerPort)

    snmp = Snmp(controllerCommInfo)
    asc = SignalController(snmp,10,100)

    # Create an object of Scheduler class
    scheduler = Scheduler(asc)

    # Open a dummy schedule and load it into a json object
    scheduleFile = open("schedule.json", "r")
    scheduleJson = json.loads(scheduleFile.read())

    scheduler.processReceivedSchedule(scheduleJson)

    # Schedule a vehicle call on all phases after 10 seconds
    # scheduler.addCommandToSchedule(Command(255,10,10,6))
    time.sleep(100)
