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
    print("Current group has " + str(len(currentGroup)) + " elements")
    currentGroup = []
    i = i + 1



# i = 0

# while len(commandsInSchedule)!=0:
#     currentGroup = [commandsInSchedule[i]]
#     if ((commandsInSchedule[i].startTime == commandsInSchedule[i+1].startTime) and (commandsInSchedule[i].commandType == commandsInSchedule[i+1].commandType)):
#         currentGroup = currentGroup + [commandsInSchedule[i+1]]
#         print("Current group has " + str(len(currentGroup)) + " elements")
#         currentGroup = []
#         commandsInSchedule.pop(i)
#         commandsInSchedule.pop(i)
#         i = 0
#     else:
#             print("Current group has " + str(len(currentGroup)) + " elements")
#             currentGroup = []
#             commandsInSchedule.pop(i)
#             i = 0