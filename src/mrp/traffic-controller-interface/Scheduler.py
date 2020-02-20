class Scheduler:
    def __init__(self)

    def addCommandToSchedule(self, ring, commandObject):
        if self.commandId > 65534:
            self.commandId = 0
        self.commandId = self.commandId + 1

        if (commandType <= 1 or commandType > 7):
            return False
        
        # Omit vehicle phases
        elif commandType == 2:

            self.commandScheduler[ring].add_job(self.omitVehPhases, args = [commandObject.startTime], 
                    trigger = 'interval', 
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandStartTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandEndTime)),                     
                    id = str(self.commandId))
            return self.commandId
        
        # Omit pedestrian phases
        elif commandType == 3:
            self.commandScheduler[ring].add_job(self.omitPedPhases, args = [commandPhase], 
                    trigger = 'interval',
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandStartTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandEndTime)),                     
                    id = str(self.commandId))
            return self.commandId

        # Hold vehicle phases
        elif commandType == 4:
            self.commandScheduler[ring].add_job(self.holdPhases, args = [commandPhase], 
                    trigger = 'interval', 
                    seconds = self.ntcipBackupTime_Sec-1,
                    start_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandStartTime)), 
                    end_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandEndTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Forceoff vehicle phases
        elif commandType == 5:
            self.commandScheduler[ring].add_job(self.forceOffPhases, args = [commandPhase], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandStartTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Call vehicle phases
        elif commandType == 6:
            self.commandScheduler[ring].add_job(self.callVehPhases, args = [commandPhase], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandStartTime)), 
                    id = str(self.commandId))
            return self.commandId

        # Call pedestrian phases
        elif commandType == 7:
            self.commandScheduler[ring].add_job(self.callPedPhases, args = [commandPhase], 
                    trigger = 'date', 
                    run_date=(datetime.datetime.now()+datetime.timedelta(seconds=commandStartTime)), 
                    id = str(self.commandId))
            return self.commandId

    def stopCommandScheduler(self):
        for schedulerIndex in range (0,2):
            self.commandScheduler[schedulerIndex].remove_all_jobs()
            self.commandScheduler[schedulerIndex].shutdown()
        return True