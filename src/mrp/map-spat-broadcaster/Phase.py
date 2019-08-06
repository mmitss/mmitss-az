class Phase:
    def __init__(self):
        self.phaseNo = 0
        self.currState = 0
        self.startTime = 0
        self.minEndTime = 0
        self.maxEndTime = 0
        self.elapsedTime = 0
    
    def asDict(self):
        return {"phaseNo":self.phaseNo, "currState":self.currState, "startTime":self.startTime, "minEndTime":self.minEndTime, "maxEndTime":self.maxEndTime, "elapsedTime":self.elapsedTime}