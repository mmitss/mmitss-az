import json
import datetime
import atexit
from apscheduler.schedulers.background import BackgroundScheduler
from SnmpApi import SnmpApi
from Command import Command

class Scheduler:
    def __init__(self, signalControllerCommInfo:tuple, ntcipBackupTime_Sec:int):
        # NTCIP Backup Time
        self.ntcipBackupTime_Sec = ntcipBackupTime_Sec
        
        # Create an object of SnmpApi
        self.snmp = SnmpApi(signalControllerCommInfo)

        # Scheduler parameters
        self.commandScheduler = BackgroundScheduler() 
        self.commandScheduler.start()
        
        self.commandId = 0
        self.currentCommandPool = []
        
        # Ensure that the scheduler shuts down when the app is exited
        atexit.register(lambda: self.commandScheduler.shutdown(wait=False))

    def processNewSchedule(self, scheduleJson:json):
        # Clear existing schedule _TODO_


        
        # Sort the schedule by three levels: 1. Command Start Time, 2. Command Type, and 3. Command End Time
        scheduleJson = scheduleJson["Schedule"]
        scheduleJson = sorted(scheduleJson, key = lambda i: (i["commandStartTime"]))

        # Read the json into a data structure
        scheduleDataStructure = self.createScheduleDataStructure(scheduleJson)

        # Form action group
        index = 0
        groupIndex = 0
        while index < len(scheduleDataStructure):
            
            currentGroup = [scheduleDataStructure[index]]

            # Look for all commands in the list that has same start time and the type
            for command in scheduleDataStructure:
                if (command != currentGroup[0] and (command.startTime == currentGroup[0].startTime) and (command.commandType == currentGroup[0].commandType)):
                    currentGroup = currentGroup + [command]
                    index = index+1

            groupCommand =  self.formulateGroupCommand(currentGroup)

            # Check the end times of all items in the current group and add additional commands for the phases that end later.
            for command in currentGroup:
                if command.endTime > groupCommand.endTime:
                    scheduleDataStructure.append(Command(command.phase,groupCommand.endTime,command.endTime,command.commandType))

            groupMaxEndTime = max(command.endTime for command in currentGroup)
            
            if groupCommand.commandType == 4:
                # Develop a command to end the HOLD or OMIT and add it in the schedule:
                groupEndCommand = Command(0,groupMaxEndTime,groupMaxEndTime,7)
                scheduleDataStructure.append(groupEndCommand)
            elif groupCommand.commandType == 5:
                # Develop a command to end the HOLD or OMIT and add it in the schedule:
                groupEndCommand = Command(0,groupMaxEndTime,groupMaxEndTime,8)
                scheduleDataStructure.append(groupEndCommand)
            elif groupCommand.commandType == 6:
                # Develop a command to end the HOLD or OMIT and add it in the schedule:
                groupEndCommand = Command(0,groupMaxEndTime,groupMaxEndTime,9)
                scheduleDataStructure.append(groupEndCommand)

            # Resort the schedule data structure based on command start time
            scheduleDataStructure.sort(key=lambda x: x.startTime, reverse=False)

            # If this is the first commandGroup of the schedule, check if it is already being executed inside the signal controller.

            if groupIndex == 0:
                # Check if the current groupCommand is already running in the signalController:
                    
                if False:
                    pass
                else:
                    self.clearScheduler()
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
            commandObject = Command(command["commandPhase"], command["commandStartTime"], command["commandEndTime"], command["commandType"])
            scheduleDataStructure = scheduleDataStructure + [commandObject]

        return scheduleDataStructure


    # Formulate group command:
    def formulateGroupCommand(self, currentGroup:list):
        groupCommand = currentGroup[0].commandType
        groupStartTime = currentGroup[0].startTime
        groupEndTime = min(command.endTime for command in currentGroup )
        groupPhases = []
        for command in currentGroup:
            groupPhases = groupPhases + [command.phase]

        groupPhaseInt = self.formulateBinaryIntegerRepresentation(groupPhases)

        groupCommand = Command(groupPhaseInt, groupStartTime, groupEndTime, groupCommand)
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
        CALL_VEH_PHASES = 1
        CALL_PED_PHASES = 2
        FORCEOFF_PHASES = 3
        HOLD_VEH_PHASES = 4
        OMIT_VEH_PHASES = 5
        OMIT_PED_PHASES = 6
        CLEAR_HOLD = 7
        CLEAR_VEH_OMIT = 8
        CLEAR_PED_OMIT = 9

        # Assign Command ID:       
        if self.commandId > 65534:
            self.commandId = 0
        self.commandId = self.commandId + 1
        
        # Check for validity of command type:
        if (commandObject.commandType < 1 or commandObject.commandType > 9):
            return False

        # Check if command is to clear an existing command from NTCIP backup:

        # Call vehicle phases
        elif commandObject.commandType == CALL_VEH_PHASES:
            self.commandScheduler.add_job(self.snmp.callVehPhases, args = [commandObject.phase], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Call pedestrian phases
        elif commandObject.commandType == CALL_PED_PHASES:
            self.commandScheduler.add_job(self.snmp.callPedPhases, args = [commandObject.phase], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId
        
        # Forceoff vehicle phases
        elif commandObject.commandType == FORCEOFF_PHASES:
            self.commandScheduler.add_job(self.snmp.forceOffPhases, args = [commandObject.phase], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Hold vehicle phases
        elif commandObject.commandType == HOLD_VEH_PHASES:
            self.commandScheduler.add_job(self.snmp.holdPhases, args = [commandObject.phase], 
                    trigger = 'interval', 
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Omit vehicle phases
        elif commandObject.commandType == OMIT_VEH_PHASES:
            self.commandScheduler.add_job(self.snmp.omitVehPhases, args = [commandObject.phase], 
                    trigger = 'interval', 
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                    id = str(self.commandId))
            return self.commandId
        
        # Omit pedestrian phases
        elif commandObject.commandType == OMIT_PED_PHASES:
            self.commandScheduler.add_job(self.snmp.omitPedPhases, args = [commandObject.phase], 
                    trigger = 'interval',
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                    id = str(self.commandId))
            return self.commandId
        
        # Clear Hold
        elif commandObject.commandType == CLEAR_HOLD:
            self.commandScheduler.add_job(self.snmp.holdPhases, args = [0], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime-0.001)), 
                    id = str(self.commandId))
            return self.commandId

        # Clear Veh-Omit
        elif commandObject.commandType == CLEAR_VEH_OMIT:
            self.commandScheduler.add_job(self.snmp.omitVehPhases, args = [0], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime-0.001)), 
                    id = str(self.commandId))
            return self.commandId

        # Clear Ped-Omit
        elif commandObject.commandType == CLEAR_PED_OMIT:
            self.commandScheduler.add_job(self.snmp.omitPedPhases, args = [0], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime-0.001)), 
                    id = str(self.commandId))
            return self.commandId





    def stopCommandScheduler(self):
        # _TODO_ Remove current holds and omits
        self.commandScheduler.remove_all_jobs()
        self.snmp.holdPhases(0)
        self.snmp.omitVehPhases(0)
        self.snmp.omitPedPhases(0)        
        self.commandScheduler.shutdown(wait=False)
        
    # _TODO_
    def clearScheduler(self):
        self.commandScheduler.remove_all_jobs()
        self.snmp.holdPhases(0)
        self.snmp.omitVehPhases(0)
        self.snmp.omitPedPhases(0)    
        pass

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
    scheduler = Scheduler(controllerCommInfo, 10)

    # Open a dummy schedule and load it into a json object
    scheduleFile = open("schedule.json", "r")
    scheduleJson = json.loads(scheduleFile.read())

    scheduler.processNewSchedule(scheduleJson)

    # Schedule a vehicle call on all phases after 10 seconds
    # scheduler.addCommandToSchedule(Command(255,10,10,6))
    time.sleep(100)
