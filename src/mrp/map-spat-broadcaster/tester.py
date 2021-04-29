import json

with open("/nojournal/bin/mmitss-phase3-master-config.json", 'r') as fp:
    config = json.load(fp)

splitPhasesJson = config["SignalController"]["SplitPhases"]

splitPhasesList = [0 for phase in range(8)]

for phase in range(8):
    splitPhaseJsonKey = str(phase+1)
    if splitPhaseJsonKey in splitPhasesJson:
        splitPhasesList[phase] = splitPhasesJson[splitPhaseJsonKey]

