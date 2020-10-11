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
from Scheduler import Scheduler

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
    tci_commInfo = (mrpIp, port)
    s.bind(tci_commInfo)
    
    # Create objects of the related modules:
    asc = SignalController()
    scheduler = Scheduler(asc)

    while(True):
        # Receive data on the TCI socket
        data, address = s.recvfrom(10240)
        data = data.decode()
        # Load the received data into a json object
        receivedMessage = json.loads(data)

        if receivedMessage["MsgType"]=="Schedule":
            if receivedMessage["Schedule"] == "Clear":
                print("[" + str(datetime.datetime.now()) + "] " + "Received a clear request at time:" + str(time.time()))
                scheduler.clearBackgroundScheduler(True)
                # Clear all holds, forceoffs, calls, and omits from the signal controller:
                scheduler.clearAllNtcipCommandsFromSignalController()
            else:
                print("[" + str(datetime.datetime.now()) + "] " + "Received a new schedule at time:" + str(time.time())) 
                print(receivedMessage)
                scheduler.processReceivedSchedule(receivedMessage)

        elif receivedMessage["MsgType"]=="CurrNextPhaseRequest":
            # Let the object of SignalController class do the needful to send the information about current and next phase to the requestor.
            print("[" + str(datetime.datetime.now()) + "] " + "Received CurrNextPhaseRequest at time " + str(time.time()) + str(receivedMessage))
            asc.sendCurrentAndNextPhasesDict(address)
            print("[" + str(datetime.datetime.now()) + "] " + "Sent currNextPhaseStatus")

        elif receivedMessage["MsgType"]=="TimingPlanRequest":
            # Read the current timing plan from the object of SignalController class
            currentTimingPlan = asc.currentTimingPlanJson
            # Send the current timing plan to the requestor
            s.sendto(currentTimingPlan.encode(),address)
            print(currentTimingPlan)

        else: print("[" + str(datetime.datetime.now()) + "] " + "Invalid message received!")
        
    s.close()

if __name__ == "__main__":
    main()    