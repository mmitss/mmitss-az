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
from SignalController import SignalController
from PhaseControlScheduler import PhaseControlScheduler
from GeneralScheduler import GeneralScheduler

def main():
    # Read the config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))

    # Close the config file:
    configFile.close()

    # Read the logging status:
    logging = config["Logging"]

    if (logging.lower() == "True" or logging.lower() == "true"): logging = True
    elif (logging.lower() == "False" or logging.lower() == "false"): logging = False

    # Open a socket and bind it to the IP and port dedicated for this application:
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    mrpIp = config["HostIp"]
    port = config["PortNumber"]["TrafficControllerInterface"]
    tci_commInfo = (mrpIp, port)
    s.bind(tci_commInfo)
    
    # Create objects of the related modules:
    asc = SignalController()
    phaseControlScheduler = PhaseControlScheduler(asc)
    generalScheduler = GeneralScheduler(asc)

    while(True):
        # Receive data on the TCI socket
        data, address = s.recvfrom(10240)
        data = data.decode()
        # Load the received data into a json object
        receivedMessage = json.loads(data)

        if receivedMessage["MsgType"]=="Schedule":
            if receivedMessage["Schedule"] == "Clear":
                print("[" + str(datetime.datetime.now()) + "] " + "Received a clear request at time:" + str(time.time()))
                phaseControlScheduler.clearBackgroundScheduler()
                # Clear all holds, forceoffs, calls, and omits from the ASC signal controller:
                phaseControlScheduler.clearAllNtcipCommandsFromSignalController()
                # Clear all phase controls from the SignalController class of the Schedule class.
                phaseControlScheduler.signalController.resetAllPhaseControls()
            else:
                print("[" + str(datetime.datetime.now()) + "] " + "Received a new schedule at time:" + str(time.time())) 
                if logging: print(receivedMessage)
                phaseControlScheduler.signalController.resetAllPhaseControls()
                phaseControlScheduler.processReceivedSchedule(receivedMessage)

        elif receivedMessage["MsgType"]=="CurrNextPhaseRequest":
            # Let the object of SignalController class do the needful to send the information about current and next phase to the requestor.
            print("[" + str(datetime.datetime.now()) + "] " + "Received CurrNextPhaseRequest at time " + str(time.time()))
            asc.sendCurrentAndNextPhasesDict(address)
            print("[" + str(datetime.datetime.now()) + "] " + "Sent currNextPhaseStatus")

        elif receivedMessage["MsgType"]=="TimingPlanRequest":
            # Read the current timing plan from the object of SignalController class
            currentTimingPlan = asc.currentTimingPlanJson
            # Send the current timing plan to the requestor
            s.sendto(currentTimingPlan.encode(),address)
            if logging: print(currentTimingPlan)

        elif receivedMessage["MsgType"]=="SpecialFunction":
            # Extract status
            requiredStatus = receivedMessage["Status"]
            if requiredStatus == True:
                # Then activate the special function from StartTime till EndTime
                startTime = receivedMessage["StartTime"]
                endTime = receivedMessage["EndTime"]
                functionId = receivedMessage["Id"]
                generalScheduler.activateSpecialFunction(functionId, startTime, endTime)
            elif requiredStatus == False:
                functionId = receivedMessage["Id"]
                asc.setSpecialFunction(functionId, False)
        else: print("[" + str(datetime.datetime.now()) + "] " + "Invalid message received!")
        
    s.close()

if __name__ == "__main__":
    main()    