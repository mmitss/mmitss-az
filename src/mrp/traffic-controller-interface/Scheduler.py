import atexit
from apscheduler.schedulers.background import BackgroundScheduler
from SignalController import SignalController
from abc import ABC
from Logger import Logger

class Scheduler(ABC):
    def __init__(self, signalController, logger:Logger):
        self.logger = logger
        self.signalController = signalController
        self.ntcipBackupTime_Sec = signalController.ntcipBackupTime_sec

        # Scheduler parameters
        self.backgroundScheduler = BackgroundScheduler() 
        self.backgroundScheduler.start()

        # Ensure that the scheduler shuts down when the app is exited
        atexit.register(lambda: self.stopBackgroundScheduler())
    
    def stopBackgroundScheduler(self):
        """
        stopBackgroundScheduler function first clears all jobs from the backgroundScheduler, 
        and then shuts down the backgroundScheduler. This function is intended to run at the exit.
        """
        
        # Clear all jobs from the BackgroundScheduler
        self.backgroundScheduler.remove_all_jobs()
        
        # Shut down the background scheduler
        self.backgroundScheduler.shutdown(wait=False)