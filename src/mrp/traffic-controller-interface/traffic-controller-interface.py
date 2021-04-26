"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

M_TrafficControllerInterface.py
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This is a wrapper module for the traffic controller commander software. It performs
following functions:
(1) Listens for messages from other MMITSS components
(2) If the received message is a schedule:
    - If the schedule is a clear request, it clears the BackgroundScheduler and 
        clears all NTCIP commands from the signal controller.
    - If the schedule is a new schedule, it is processed and appropriate commands
        are added to the BackgroundScheduler.
(3) If the message is a request for current and next phases, this module formulates a 
    json string containing the information about current and next phases, and sends it to 
    the requestor.
(4) If the message is a request for current timing plan, then send the current timing 
    plan stored in the SignalController object.
***************************************************************************************
"""
import json
import socket
import time, datetime
import atexit
from Logger import Logger
from SignalController import SignalController
from PhaseControlScheduler import PhaseControlScheduler
from GeneralScheduler import GeneralScheduler

def destruct_logger(logger:Logger):
    logger.write("TCI is shutting down now!")
    del logger

def main():
    # Read the config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))

    # Close the config file:
    configFile.close()

    # Open a socket and bind it to the IP and port dedicated for this application:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    mrpIp = config["HostIp"]
    port = config["PortNumber"]["TrafficControllerInterface"]
    mapSpatBroadcasterPort = config["PortNumber"]["MapSPaTBroadcaster"]

    tci_commInfo = (mrpIp, port)
    s.bind(tci_commInfo)
    
    # Create objects of the related modules:
    consoleStatus = config["ConsoleOutput"]
    loggingStatus = config["Logging"]
    intersectionName = config["IntersectionName"]

    logger = Logger(consoleStatus, loggingStatus, intersectionName)
    
    atexit.register(lambda: destruct_logger(logger))

    asc = SignalController(logger)
    phaseControlScheduler = PhaseControlScheduler(asc, logger)
    generalScheduler = GeneralScheduler(asc, logger)

    while(True):
        # Receive data on the TCI socket
        data, address = s.recvfrom(10240)
        data = data.decode()
        # Load the received data into a json object
        receivedMessage = json.loads(data)

        if receivedMessage["MsgType"]=="Schedule":
            if receivedMessage["Schedule"] == "Clear":
                logger.write("Received a clear request")
                clearSignal = json.dumps(dict({"MsgType": "ScheduleSpatClear"}))
                s.sendto(clearSignal.encode(), (mrpIp, mapSpatBroadcasterPort))
                logger.write("Forwarded clear signal to MapSPaTBroadcaster")
                phaseControlScheduler.backgroundScheduler.remove_all_jobs()
                # Clear all holds, forceoffs, calls, and omits from the ASC signal controller:
                phaseControlScheduler.clearAllNtcipCommandsFromSignalController()
                # Clear all phase controls from the SignalController class of the Schedule class.
                phaseControlScheduler.signalController.resetAllPhaseControls()
            else:
                logger.write("Received a new schedule") 
                logger.write(str(receivedMessage))
                phaseControlScheduler.signalController.resetAllPhaseControls()
                phaseControlScheduler.processReceivedSchedule(receivedMessage)

        elif receivedMessage["MsgType"]=="CurrNextPhaseRequest":
            # Let the object of SignalController class do the needful to send the information about current and next phase to the requestor.
            logger.write("Received CurrNextPhaseRequest")
            asc.sendCurrentAndNextPhasesDict(address)
            logger.write("Sent currNextPhaseStatus the requestor")

        elif receivedMessage["MsgType"]=="TimingPlanRequest":
            # Read the current timing plan from the object of SignalController class
            currentTimingPlan = asc.currentTimingPlanJson
            # Send the current timing plan to the requestor
            s.sendto(currentTimingPlan.encode(),address)
            logger.write(currentTimingPlan)

        elif receivedMessage["MsgType"]=="SpecialFunction":
            # Extract status
            requiredStatus = receivedMessage["Status"]
            if requiredStatus == True:
                # Then activate the special function from StartTime till EndTime
                startTime = receivedMessage["StartTime"]
                endTime = receivedMessage["EndTime"]
                functionId = receivedMessage["Id"]
                currentStatus = asc.specialFunctionLocalStatus[functionId]
                if currentStatus != requiredStatus:
                    generalScheduler.activateAndScheduleSpecialFunctionMaintenance(functionId, startTime, endTime)
                else: 
                    # This is a current limitations of TCI. If the special function is already ON, then further requests to set it ON will be discarded till it is ON.
                    logger.write("Special function {} is already in the required status. Discarding the new request!".format(functionId))
            elif requiredStatus == False:
                functionId = receivedMessage["Id"]
                asc.updateSpecialFunctionLocalStatus(functionId, requiredStatus)
                asc.setSpecialFunctionControllerStatus(functionId)
        else: logger.write("Received invalid message of type: " + receivedMessage["MsgType"])
        
    s.close()

if __name__ == "__main__":
    main()    