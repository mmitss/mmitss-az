import json
import time, datetime
from Scheduler import Scheduler
from apscheduler.triggers import interval
from apscheduler.triggers import date
from SignalController import SignalController

class GeneralScheduler(Scheduler):
    def __init__(self, signalController:SignalController):
        super().__init__(signalController)
        
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


