import json
from Command import Command

class Scheduler:
    def __init__(self):
        pass

    def processNewSchedule(self, scheduleJson:json):

        # Sort the schedule by three levels: 1. Command Start Time, 2. Command Type, and 3. Command End Time
        scheduleJson = scheduleJson["Schedule"]
        scheduleJson = sorted(scheduleJson, key = lambda i: (i["commandStartTime"], i["commandType"], i["commandEndTime"]))

        # Read the json into a data structure
        commandsInSchedule = []
        for command in scheduleJson:
            commandObject = Command(command["commandPhase"], command["commandStartTime"], command["commandEndTime"], command["commandType"])
            commandsInSchedule = commandsInSchedule + [commandObject]

        # Form action group
        i = 0
        while i < len(commandsInSchedule):
            currentGroup = [commandsInSchedule[i]]

            # Look for all commands in the list that has same start time and the type
            for command in commandsInSchedule:
                if (command != currentGroup[0] and (command.startTime == currentGroup[0].startTime) and (command.commandType == currentGroup[0].commandType)):
                    currentGroup = currentGroup + [command]
                    i = i+1

            groupCommand =  formulateGroupCommand(currentGroup)
            print(groupCommand)
            currentGroup = []
            i = i + 1
    
        # Formulate group command:
        def formulateGroupCommand(self, currentGroup:list):
            groupCommand = currentGroup[0].commandType
            groupStartTime = currentGroup[0].startTime
            groupEndTime = currentGroup[0].endTime
            groupPhases = []
            for command in currentGroup:
                groupPhases = groupPhases + [command.phase]

            groupPhaseInt = formulateBinaryInegerRepresentation(groupPhases)

            groupCommand = Command(groupPhaseInt, groupStartTime, groupEndTime, groupCommand)
            return groupCommand
        
        # Formulate the binary integer representation of the commandPhase:
        def formulateBinaryInegerRepresentation(self, groupPhases):
            groupPhaseStr = list("00000000")
            for phase in groupPhases:
                groupPhaseStr[phase-1]="1"
            groupPhaseStr = groupPhaseStr[::-1]
            groupPhaseStr = "".join(groupPhaseStr)
            groupPhaseInt = int(groupPhaseStr,2)
            return groupPhaseInt    

    if __name__ == "__main__":
        scheduleFile = open("schedule.json", "r")
        scheduleJson = json.loads(scheduleFile.read())