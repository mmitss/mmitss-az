import json
import time, datetime
import atexit
from apscheduler.schedulers.background import BackgroundScheduler
from apscheduler.triggers import interval
from apscheduler.triggers import date
from SignalController import SignalController

class GeneralScheduler:
    def __init__(self, signalController:SignalController):
        self.signalController = signalController
        self.ntcipBackupTime_Sec = signalController.ntcipBackupTime_sec

        # Scheduler parameters
        self.backgroundScheduler = BackgroundScheduler() 
        self.backgroundScheduler.start()

        # Ensure that the scheduler shuts down when the app is exited
        atexit.register(lambda: self.stopBackgroundScheduler())
        
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

    def stopBackgroundScheduler(self):
        """
        stopBackgroundScheduler function first clears all jobs from the backgroundScheduler, 
        clears all NTCIP commands in the signal controller, and then shuts down the backgroundScheduler.
        This function is intended to run at the exit.
        """
        
        # Clear all jobs from the BackgroundScheduler
        self.clearBackgroundScheduler()
        
        # Shut down the background scheduler
        self.backgroundScheduler.shutdown(wait=False)

    def clearBackgroundScheduler(self):
        """
        clearBackgroundScheduler clears all phase control jobs from the BackgroundScheduler.
        """

        self.backgroundScheduler.remove_all_jobs()