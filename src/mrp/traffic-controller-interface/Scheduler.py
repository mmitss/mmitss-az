import json
import datetime
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
        
    def processNewSchedule(self, scheduleJson:json):
        # Sort the schedule by three levels: 1. Command Start Time, 2. Command Type, and 3. Command End Time
        scheduleJson = scheduleJson["Schedule"]
        scheduleJson = sorted(scheduleJson, key = lambda i: (i["commandStartTime"], i["commandType"], i["commandEndTime"]))

        # Read the json into a data structure
        scheduleDataStructure = self.createScheduleDataStructure(scheduleJson)

        # Form action group
        index = 0
        while index < len(scheduleDataStructure):
            currentGroup = [scheduleDataStructure[index]]

            # Look for all commands in the list that has same start time and the type
            for command in scheduleDataStructure:
                if (command != currentGroup[0] and (command.startTime == currentGroup[0].startTime) and (command.commandType == currentGroup[0].commandType)):
                    currentGroup = currentGroup + [command]
                    index = index+1

            groupCommand =  self.formulateGroupCommand(currentGroup)
            currentGroup = []
            index = index + 1

        # _TODO_: Check the end times of all items in the current group and add additional commands for the phases that end later.
        
        # while
    
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
        print(groupEndTime)
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
        # print(groupPhases)
        # print(groupPhaseStr)
        # print(groupPhaseInt)
        # print("\n")
        return groupPhaseInt    

    '''##############################################
                    Scheduler Methods
    ##############################################'''

    def addCommandToSchedule(self, commandObject:Command):
        if self.commandId > 65534:
            self.commandId = 0
        self.commandId = self.commandId + 1

        if (commandObject.commandType <= 1 or commandObject.commandType > 7):
            return False
        
        # Omit vehicle phases
        elif commandObject.commandType == 2:

            self.commandScheduler.add_job(self.snmp.omitVehPhases, args = [commandObject.phaseInt], 
                    trigger = 'interval', 
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                    id = str(self.commandId))
            return self.commandId
        
        # Omit pedestrian phases
        elif commandObject.commandType == 3:
            self.commandScheduler.add_job(self.snmp.omitPedPhases, args = [commandObject.phaseInt], 
                    trigger = 'interval',
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)),                     
                    id = str(self.commandId))
            return self.commandId

        # Hold vehicle phases
        elif commandObject.commandType == 4:
            self.commandScheduler.add_job(self.snmp.holdPhases, args = [commandObject.phaseInt], 
                    trigger = 'interval', 
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.endTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Forceoff vehicle phases
        elif commandObject.commandType == 5:
            self.commandScheduler.add_job(self.snmp.forceOffPhases, args = [commandObject.phaseInt], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Call vehicle phases
        elif commandObject.commandType == 6:
            self.commandScheduler.add_job(self.snmp.callVehPhases, args = [commandObject.phaseInt], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Call pedestrian phases
        elif commandObject.commandType == 7:
            self.commandScheduler.add_job(self.snmp.callPedPhases, args = [commandObject.phaseInt], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandObject.startTime)), 
                    id = str(self.commandId))
            return self.commandId

    def stopCommandScheduler(self):
        self.commandScheduler.remove_all_jobs()
        self.commandScheduler.shutdown()


if __name__ == "__main__":
    
    # Define controller's communication Info
    controllerIp = "10.12.6.17"
    controllerPort = 501
    controllerCommInfo = (controllerIp, controllerPort)


    # Create an object of Scheduler class
    scheduler = Scheduler(controllerCommInfo, 2)

    # Open a dummy schedule and load it into a json object
    scheduleFile = open("schedule.json", "r")
    scheduleJson = json.loads(scheduleFile.read())

    scheduler.processNewSchedule(scheduleJson)

