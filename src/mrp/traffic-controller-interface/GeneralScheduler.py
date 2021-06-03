import json
import time, datetime
from Scheduler import Scheduler
from apscheduler.triggers import interval
from apscheduler.triggers import date
from SignalController import SignalController
from Logger import Logger

class GeneralScheduler(Scheduler):
    def __init__(self, signalController:SignalController, logger:Logger):
        super().__init__(signalController, logger)
        
        self.scheduleTimingPlanUpdate(self.signalController.timingPlanUpdateInterval_sec)

    def scheduleTimingPlanUpdate(self, update_interval:int) -> int:
        """
        scheduleTimingPlanUpdate takes in the interval as an argument, and for that interval, 
        schedules the update of active timing plan.

        Arguments:
        ----------
            (1) interval:
                    Time interval (seconds) between successive function call.
        
        """
        trigger = interval.IntervalTrigger(seconds=update_interval)
        self.backgroundScheduler.add_job(self.signalController.updateAndSendActiveTimingPlan,
                    trigger = trigger,
                    max_instances=10)

    def activateAndScheduleSpecialFunctionMaintenance(self, functionId, startSecFromNow:float, endSecFromNow:float):
        if startSecFromNow == 0.0:
            startSecFromNow = 0.01 # Jobs that start at time NOW (0.0 sec from now) are incompatible with BackgroundScheduler

        self.signalController.updateSpecialFunctionLocalStatus(functionId, True)

        intervalTrigger = interval.IntervalTrigger(seconds=self.ntcipBackupTime_Sec-1,
                                                start_date=(datetime.datetime.now()+datetime.timedelta(seconds=startSecFromNow)), 
                                                end_date=(datetime.datetime.now()+datetime.timedelta(seconds=endSecFromNow)))

        self.backgroundScheduler.add_job(self.signalController.setSpecialFunctionControllerStatus, 
                                            args = [functionId], 
                                            trigger = intervalTrigger, 
                                            max_instances=3)

        dateTrigger = date.DateTrigger(run_date=(datetime.datetime.now()+datetime.timedelta(seconds=endSecFromNow-0.1)))
        self.backgroundScheduler.add_job(self.signalController.updateSpecialFunctionLocalStatus, 
                                            args = [functionId, False], 
                                            trigger = dateTrigger, 
                                            max_instances=3)
                                            
        dateTrigger = date.DateTrigger(run_date=(datetime.datetime.now()+datetime.timedelta(seconds=endSecFromNow-0.01)))
        self.backgroundScheduler.add_job(self.signalController.setSpecialFunctionControllerStatus, 
                                            args = [functionId], 
                                            trigger = dateTrigger, 
                                            max_instances=3)


