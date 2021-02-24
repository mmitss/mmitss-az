"""
**********************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  Scheduler.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

**********************************************************************************
"""

import atexit
import time, datetime
from types import FunctionType
from apscheduler.schedulers.background import BackgroundScheduler
from apscheduler.triggers import interval

class Scheduler:
    """
    provides method to schedule a daily recurring jobs. 

    A single instance of `apscheduler.schedulers.background.BackgroundScheduler` is created and 
    it is used for all scheduling requests
    """
    def __init__(self):
        # Create an instance of the background scheduler and start it
        self.backgroundScheduler = BackgroundScheduler()
        self.backgroundScheduler.start()
        # Register the shutdown of background scheduler at exit
        atexit.register(lambda: self.backgroundScheduler.shutdown(wait=False))
        
    def schedule_daily_execution(self, func, startHour:int, startMinute:int):
        """
        schedules the `func()` as a daily recurring job starting at `startHour:startMinute`. 
        
        If the `startHour:startMinute` is already passed for the day, then the first 
        execution is scheduled for the next day. 

        NOTES: 
            (1) startHour must be in 24-hour format, hence: (0 <= startHour < 24) .
            (2) startMinute: (0 <= startMinute < 60).
            (3) If invalid startHour and/or startMinute is provided, defaults the execution time to midnight
        """
        # Validate startHour and startMinute. If invalid, default to midnight:
        if startHour < 0 or startHour >= 24 or startMinute < 0 or startMinute >= 60:
            startHour = 0
            startMinute = 0
        
        # Get current date and time
        now = datetime.datetime.now()
        
        #If it is in the past (for current day) create the start time (datetime object) for the data transfer from next day
        if ((int(now.hour) > startHour) or  ((int(now.hour) == startHour) and (int(now.minute) >= startMinute))):
            start = (datetime.datetime.now() + datetime.timedelta(days=1)).replace(hour=startHour, minute=startMinute, second=0)

        # Else create the start time (datetime object) for the data transfer starting from current day
        else: start = (datetime.datetime.now()).replace(hour=startHour, minute=startMinute, second=0)

        # Create an object of interval class. Use the datetime object created earlier for the start time.
        trigger = interval.IntervalTrigger(days=1, start_date=start)                                               

        # Add the recurring job in the background scheduler
        self.backgroundScheduler.add_job(func, trigger=trigger, max_instances=3)


if __name__ == "__main__":
    def dummy_function():
        """
        prints a string
        """
        print("hello")

    startHour = 7
    startMinute = 31

    func = dummy_function

    scheduler = Scheduler()
    scheduler.schedule_daily_execution(func, startHour, startMinute)
    while True: time.sleep(3600)


    