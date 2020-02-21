import json
from Command import Command

scheduleFile = open("schedule.json", "r")
scheduleJson = json.loads(scheduleFile.read())

schedule = scheduleJson["Schedule"]

schedule = sorted(schedule, key = lambda i: (i["commandStartTime"], i["commandType"], i["commandEndTime"]))

# Read the json into a data structure
commandsInSchedule = []
for command in schedule:
    commandObject = Command(command["commandPhase"], command["commandStartTime"], command["commandEndTime"], command["commandType"])
    commandsInSchedule = commandsInSchedule + [commandObject]

# Form action group
flag = True
i = 0
while i < len(commandsInSchedule):
    currentGroup = [commandsInSchedule[i]]

    # Look for all commands in the list that has same start time and the type
    for command in commandsInSchedule:
        if (command != currentGroup[0] and (command.startTime == currentGroup[0].startTime) and (command.commandType == currentGroup[0].commandType)):
            currentGroup = currentGroup + [command]
            i = i+1
    
    groupCommand = currentGroup[0].commandType
    groupStartTime = currentGroup[0].startTime
    groupEndTime = currentGroup[0].endTime
    groupPhases = []
    for command in currentGroup:
        groupPhases = groupPhases + [command.phase]

    phaseStr = list("00000000")
    for phase in groupPhases:
        phaseStr[phase-1]="1"
    phaseStr = phaseStr[::-1]
    phaseStr = "".join(phaseStr)
    phaseInt = int(phaseStr,2)

    # Formulate the binary integer representation of the commandPhase:
    



    currentGroup = []
    i = i + 1