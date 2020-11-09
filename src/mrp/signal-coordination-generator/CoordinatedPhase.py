class CoordinatedPhase:
    def __init__(self):
        self.phaseNo = 0
        self.vehicleType = 0
        self.vehicleID = 0
        self.basicVehicleRole = 0
        self.ETA = 0.0
        self.coordinationSplit = 0.0
        self.priorityRequestType = 0
        self.requestUpdateTime = 0.0
    
    def asDict(self):
        return {"requestedPhase":self.phaseNo, "vehicleType":self.vehicleType, "vehicleID":self.vehicleID, "basicVehicleRole":self.basicVehicleRole, "ETA":self.ETA, "CoordinationSplit":self.coordinationSplit, "priorityRequestType":self.priorityRequestType, "requestUpdateTime":self.requestUpdateTime}