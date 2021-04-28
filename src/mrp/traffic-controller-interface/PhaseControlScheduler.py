"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

Scheduler.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This class is responsible for processing the schedule received from the PriorityRequestSolver 
or any other MMITSS component. For processing following steps are followed:
    (1) All commands in the schedule are sorted based on the command start time.
    (2) Formulate the groups of commands having same start time and same action.
    (3) Create the bitstring based on the phases included in the group, and formulate 
        corresponding integer representation. Formulate the corresponding group command.
    (4) Run an instance of background scheduler and based on the start time and the end time 
        of the group command and the NTCIP backup time, schedule a SignalController::phaseControl 
        function in the background scheduler.
    (5) If the schedule is new, make sure not to stop the currently active actions in the NTCIP
        mode of the signal controller, before executing the new schedule.
    (6) At exit, clear the NTCIP commands in the signal controller, and in the background scheduler.

This class uses the APScheduler library, which needs to be installed separately.
For installation and general help: https://apscheduler.readthedocs.io/en/v1.3.1/
***************************************************************************************
"""

import json
import time, datetime
import atexit
import socket
from apscheduler.schedulers.background import BackgroundScheduler
from apscheduler.triggers import interval
from apscheduler.triggers import date
import Command
from SignalController import SignalController
from Scheduler import Scheduler
from Logger import Logger
from ScheduleSpatTranslator import ScheduleSpatTranslator

class PhaseControlScheduler(Scheduler):

    def __init__(self, signalController:SignalController, logger:Logger):
        """
        extracts NTCIP_BackupTime, starts the background scheduler, and registers a call 
        to stopBackgroundScheduler() at exit.
        """
        super().__init__(signalController, logger)      
        self.scheduleReceiptTime = 0

        configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
        config = (json.load(configFile))

        # Close the config file:
        configFile.close()

        self.mapSpatBroadcasterAddress = ((config["HostIp"],config["PortNumber"]["MapSPaTBroadcaster"]))
        self.scheduleSpatTranslator = ScheduleSpatTranslator()
      

    def processReceivedSchedule(self, scheduleJson:json):
        """
        Scheduler::processReceivedSchedule function is responsible for processing the schedule (json) 
        passed in the arguments. After processing, command groups are created based on start times of 
        individual commands, phases in the command, and the action required in command. The group commands
        are scheduled in the BackgroundScheduler. In addition to group commands, additional commands are 
        created for:
            (1) Ending the scheudled commands after their EndTime is reached
            (2) If two (or more) commands have same startTime, same actions, but different endTimes.

        This function has has three subfunctions:
            (1) clearOldSchedule(scheduleDataStructure)
            (2) formulateGroupCommand(currentGroup)
            (3) clearScheduleDataStructure(scheduleJson)

        Arguments: 
        ----------
            (1) A JSON string containing the schedule that needs to be processed.
        """
        
        def clearOldSchedule(scheduleDataStructure:list, scheduleReceiptTime:float):   
            """
            clears the commands from old schedule if the new schedule needs those commands to continue running

            checks the received schedule for actions starting at time 0.0.
            If a hold, vehOmit, or pedOmit starts at time zero, this function checks if the corresponding action is already active in 
            the signal controller. Only if such action is NOT active in the signal controller, then it is cleared from the signal controller.
            On the other hand, vehCall, pedCall, and forceoffs are cleared from the signal controller unconditionally.

            Arguments:
            ----------
                (1) ScheduleDataStructure

            """     
            # Clear all jobs from the BackgroundScheduler
            self.backgroundScheduler.remove_all_jobs()
            
            # Initialize flags to clear Holds, PedOmits, and VehOmits
            clearHolds = True
            clearPedOmit = True
            clearVehOmit = True

            # Update the flags based on the commands in the schedule
            for command in scheduleDataStructure:
                if command.startTime == 0 or command.startTime == 0.0:
                    if command.control == Command.HOLD_VEH_PHASES:
                        if self.signalController.getPhaseControl(Command.HOLD_VEH_PHASES) > 0:
                            clearHolds = False
                    elif command.control == Command.OMIT_VEH_PHASES:    
                        if self.signalController.getPhaseControl(Command.OMIT_VEH_PHASES) > 0:
                            clearVehOmit = False
                    elif command.control == Command.OMIT_PED_PHASES:
                        if self.signalController.getPhaseControl(Command.OMIT_PED_PHASES) > 0:
                            clearPedOmit = False

            # Clear Holds if its flag is True    
            if clearHolds == True:
                self.signalController.setPhaseControl(Command.HOLD_VEH_PHASES,False,[], scheduleReceiptTime)    
            
            # Clear VehOmits if its flag is True    
            if clearVehOmit == True:
                self.signalController.setPhaseControl(Command.OMIT_VEH_PHASES,False,[], scheduleReceiptTime)    

            # Clear PedOmits if its flag is True            
            if clearPedOmit == True:
                self.signalController.setPhaseControl(Command.OMIT_PED_PHASES,False,[], scheduleReceiptTime)    

            # Clear VehCalls
            self.signalController.setPhaseControl(Command.CALL_VEH_PHASES,False, [], scheduleReceiptTime)
            # Clear PedCalls
            self.signalController.setPhaseControl(Command.CALL_PED_PHASES,False, [], scheduleReceiptTime)
            # Clear Forceoffs
            self.signalController.setPhaseControl(Command.FORCEOFF_PHASES,False, [], scheduleReceiptTime)  


        def formulateGroupCommand(currentGroup:list) -> Command:
                 
            # End the group at time which is minimum of all commands' end time. Additional commands will be added later in the code
            # for controls that end later.
            groupCommand = currentGroup[0].control
            groupStartTime = currentGroup[0].startTime
            groupEndTime = min(command.endTime for command in currentGroup )
            groupPhases = []
            for command in currentGroup:
                groupPhases = groupPhases + [command.phases]

            groupCommand = Command.Command(groupPhases, groupCommand, groupStartTime, groupEndTime)
            return groupCommand
        
        # Create Schedule data structure
        def createScheduleDataStructure(scheduleJson:json) -> list:
            """
            reads the JSON string of schedule into a data structure (list of dictionaries). 
            
            Arguments:
            ----------
                (1) JSON string of the schedule
            
            Returns:
            --------
                The list of dictionaries. Each dictionary contains an individiual command.
            """

            scheduleDataStructure = []
            for command in scheduleJson:
                if command["commandType"] == "call_veh": commandType = Command.CALL_VEH_PHASES
                elif command["commandType"] == "call_ped": commandType = Command.CALL_PED_PHASES
                elif command["commandType"] == "forceoff": commandType = Command.FORCEOFF_PHASES
                elif command["commandType"] == "hold": commandType = Command.HOLD_VEH_PHASES
                elif command["commandType"] == "omit_veh": commandType = Command.OMIT_VEH_PHASES
                elif command["commandType"] == "omit_ped": commandType = Command.OMIT_PED_PHASES

                
                commandObject = Command.Command(command["commandPhase"], commandType,  command["commandStartTime"], command["commandEndTime"])
                scheduleDataStructure = scheduleDataStructure + [commandObject]

            return scheduleDataStructure
    
    ######################################### SUB-FUNCTIONS DEFINITION END ######################################### 

        # Get the schedule-spat translation JSON string and send it to the map-spat-broadcaster
        
        currentTimingPlan = json.loads(self.signalController.currentTimingPlanJson)
        yellowChange = currentTimingPlan["TimingPlan"]["YellowChange"]
        redClear = currentTimingPlan["TimingPlan"]["RedClear"]

        clearanceTimes = [(phase[0]+phase[1]) for phase in zip(yellowChange, redClear)]                
        phaseRings = currentTimingPlan["TimingPlan"]["PhaseRing"]

        try:
            scheduleSpatTranslationJson = self.scheduleSpatTranslator.get_schedule_spat_translation_json(scheduleJson, clearanceTimes, phaseRings)
        
            with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
                s.sendto(scheduleSpatTranslationJson.encode(), self.mapSpatBroadcasterAddress)
                self.logger.write("Sent Schedule-Spat Translation to Map-Spat-Broadcaster: {}".format(scheduleSpatTranslationJson))
        except Exception as e:
            self.logger.write("*****ERROR IN TRANSLATING SCHEDULE TO SPAT*****:" + str(e))
        
        self.signalController.resetAllPhaseControls()
        scheduleJson = scheduleJson["Schedule"]
        self.scheduleReceiptTime = time.time()

        for command in scheduleJson:
            command["commandStartTime"] = round(command["commandStartTime"],1)
            command["commandEndTime"] = round(command["commandEndTime"],1)

        # Delete the commands with invalid combination of startTime and EndTime
        scheduleJson =  [command for command in scheduleJson if not 
                                                        (
                                                            (command["commandStartTime"] == command["commandEndTime"]) or 
                                                            (command["commandStartTime"] > command["commandEndTime"]) or
                                                            (command["commandStartTime"] < 0) or
                                                            (command["commandEndTime"] < 0)
                                                        )
                        ]
        # Sort the schedule by three levels: 1. Command Start Time, 2. Command Type, and 3. Command End Time
        scheduleJson = sorted(scheduleJson, key = lambda i: (i["commandType"], i["commandStartTime"], i["commandEndTime"], i["commandPhase"]))

        # Read the json into a data structure
        scheduleDataStructure = createScheduleDataStructure(scheduleJson)

        self.logger.write("Clearing old schedule - actions will be maintained if new schedule begins with them!")
        # Clear the old schedule from the Background Scheduler, and clear the require NTCIP commands.       
        clearOldSchedule(scheduleDataStructure, self.scheduleReceiptTime)

        # Form a list of startGroups for commands that start together
        
        startGroups = []
        index = 0
        while index < len(scheduleDataStructure):
            
            currentGroup = [scheduleDataStructure[index]]

            # Look for all commands in the list that has same start time and the type, and gather them together
            for command in scheduleDataStructure:
                if (command != currentGroup[0] and (command.startTime == currentGroup[0].startTime) and (command.control == currentGroup[0].control)):
                    currentGroup = currentGroup + [command]
                    index = index+1

            groupCommand =  formulateGroupCommand(currentGroup)
            currentGroup = []
            index = index + 1

            startGroups = startGroups + [groupCommand]

        # schedule activation of phase controls for all groups in the startGroup 
        for group in startGroups:
            self.schedulePhaseControlActivation(group.phases, group.control, group.startTime, group.endTime)

        # Form a list of endGroups for commands that end together
        endGroups = []
        index = 0
        while index < len(scheduleDataStructure):
            
            currentGroup = [scheduleDataStructure[index]]

            # Look for all commands in the list that has same start time and the type, and gather them together
            for command in scheduleDataStructure:
                if (command != currentGroup[0] and (command.endTime == currentGroup[0].endTime) and (command.control == currentGroup[0].control)):
                    currentGroup = currentGroup + [command]
                    index = index+1

            groupCommand =  formulateGroupCommand(currentGroup)
            currentGroup = []
            index = index + 1

            endGroups = endGroups + [groupCommand]


        # schedule deactivation of phase controls for all groups in the endGroup
        for group in endGroups:
            self.schedulePhaseControlDeactivation(group.phases, group.control, group.endTime)
        

    '''##############################################
                    Scheduler Methods
    ##############################################'''
    
    def schedulePhaseControlActivation(self, phases:list, control:int, startSecFromNow:float, endSecFromNow:float):
        """
        Activates the phase control at startSecFrmNow, and keeps it active till endSecFromNow.
        """
        if startSecFromNow == 0.0:
            startSecFromNow = 0.01 # Jobs that start at time NOW (0.0 sec from now) are incompatible with BackgroundScheduler

        intervalTrigger = interval.IntervalTrigger(seconds=self.ntcipBackupTime_Sec-1,
                                                start_date=(datetime.datetime.now()+datetime.timedelta(seconds=startSecFromNow)), 
                                                end_date=(datetime.datetime.now()+datetime.timedelta(seconds=endSecFromNow)))

        self.backgroundScheduler.add_job(self.signalController.setPhaseControl, 
                                            args = [control, True, phases ,self.scheduleReceiptTime], 
                                            trigger = intervalTrigger, 
                                            max_instances=3)

    def schedulePhaseControlDeactivation(self, phases:list, control:int, secFromNow:float):
        """
        Deactivates the phase control at secFromNow - Single Job
        """
        dateTrigger = date.DateTrigger(run_date=(datetime.datetime.now()+datetime.timedelta(seconds=secFromNow)))
        self.backgroundScheduler.add_job(self.signalController.setPhaseControl, 
                                            args = [control, False, phases ,self.scheduleReceiptTime], 
                                            trigger = dateTrigger, 
                                            max_instances=3)

    def stopBackgroundScheduler(self):
        """
        stopBackgroundScheduler function first clears all jobs from the backgroundScheduler, 
        clears all NTCIP commands in the signal controller, and then shuts down the backgroundScheduler.
        This function is intended to run at the exit.
        """
        
        # Clear all jobs from the BackgroundScheduler
        self.backgroundScheduler.remove_all_jobs()

        # Clear all phase controls from the traffic signal controller
        self.clearAllNtcipCommandsFromSignalController()
        
        # Shut down the background scheduler
        self.backgroundScheduler.shutdown(wait=False)

    def clearAllNtcipCommandsFromSignalController(self):
        """
        clears all existing NTCIP commands in the signal controller
        """
        # Clear VehCalls
        self.signalController.setPhaseControl(Command.CALL_VEH_PHASES,False, [],time.time())
        # Clear PedCalls
        self.signalController.setPhaseControl(Command.CALL_PED_PHASES,False, [],time.time())
        # Clear Forceoffs
        self.signalController.setPhaseControl(Command.FORCEOFF_PHASES,False, [],time.time())
        # Clear Holds
        self.signalController.setPhaseControl(Command.HOLD_VEH_PHASES,False, [],time.time())
        # Clear VehOmits
        self.signalController.setPhaseControl(Command.OMIT_VEH_PHASES,False, [],time.time())
        # Clear PedOmits
        self.signalController.setPhaseControl(Command.OMIT_PED_PHASES,False, [],time.time())


    
'''##############################################
                   Unit testing
##############################################'''
if __name__ == "__main__":
    import time

    logger = Logger(True, False, "speedway-mountain")

    asc = SignalController(logger)
    
    # Create an object of Scheduler class
    scheduler = PhaseControlScheduler(asc, logger)

    # Open a dummy schedule and load it into a json object
    scheduleFile = open("test/schedule1.json", "r")
    scheduleJson = json.loads(scheduleFile.read())
    scheduleFile.close()

    iteration = 1
    while True:
        scheduler.processReceivedSchedule(scheduleJson)
        print("Processed iteration#" + str(iteration))
        iteration = iteration+1
        if iteration > 10:
            break
        time.sleep(200)
