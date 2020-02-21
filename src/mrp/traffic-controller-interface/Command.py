class Command:
    def __init__(self, commandPhase, commandStartTime, commandEndTime, commandType):
        self.phase = commandPhase
        self.startTime = commandStartTime
        self.endTime = commandEndTime
        self.commandType = commandType

        # Formulate the binary integer representation of the commandPhase:
        # phaseStr = ""
        # for i in range(0,8):
        #     if i==self.phase-1:
        #         phaseStr = phaseStr + "1"
        #     else:
        #         phaseStr = phaseStr + "0"
        # phaseStr = phaseStr[::-1]

        # self.phaseInt = int(phaseStr,2)

# if __name__ == "__main__":
#     command = Command(8,8,8,8)
#     print(command.phaseInt)
#     print(str(f'{(command.phaseInt):08b}')[::-1])


