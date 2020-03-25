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
This is a wrapper module for the traffic controller interface software. It performs
following functions:
(1) Listens for messages
(2) If the received message is a schedule:
    - If the schedule is a clear request, it clears the BackgroundScheduler and 
        clears all NTCIP commands from the signal controller.
    - If the schedule is a new s    timingPlanUpdateInterval_sec = config["SignalController"]["TimingPlanUpdateInterval_sec"]
    ntcipBackupTime_sec = config["SignalController"]["NtcipBackupTime_sec"]
chedule, it is processed and appropriate commands
        are added to the BackgroundScheduler.
(3) If the message is a request for current and next phases, it formulates a json 
    string containing the information about current and next phases, and sends it to 
    the requestor.
(4) If the message is a request for current timing plan, then send the current timing 
    plan stored in the SignalController object.
***************************************************************************************
"""
import json
import socket
import time
from SignalController import SignalController
from Snmp import Snmp
from Scheduler import Scheduler

def main():
    # Read a config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
    configFile.close()

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    mrpIp = config["HostIp"]
    port = config["PortNumber"]["TrafficControllerInterface"]
    tci_commInfo = (mrpIp, port)
    s.bind(tci_commInfo)
    
    observerPort = config["PortNumber"]["TrafficControllerObserver"]
    observer_commInfo = (mrpIp, observerPort)

    signalControllerIp = config["SignalController"]["IpAddress"]
    signalControllerNtcipPort = config["SignalController"]["NtcipPort"]
    signalController_commInfo = (signalControllerIp, signalControllerNtcipPort)
    timingPlanUpdateInterval_sec = config["SignalController"]["TimingPlanUpdateInterval_sec"]
    ntcipBackupTime_sec = config["SignalController"]["NtcipBackupTime_sec"]

    snmp = Snmp(signalController_commInfo)
    asc = SignalController(snmp,timingPlanUpdateInterval_sec,ntcipBackupTime_sec)
    scheduler = Scheduler(asc)

    while(True):
        data, address = s.recvfrom(2048)
        receivedMessage = json.loads(data.decode())
        if receivedMessage["MsgType"]=="Schedule":
            if receivedMessage["Schedule"] == "Clear":
                scheduler.clearBackgroundScheduler(True)
                scheduler.clearAllNtcipCommandsFromSignalController()
            else: 
                scheduler.processReceivedSchedule(receivedMessage)
        elif receivedMessage["MsgType"]=="CurrNextPhaseRequest":
            asc.sendCurrentAndNextPhasesDict(observer_commInfo, address)
        elif receivedMessage["MsgType"]=="TimingPlanRequest":
            currentTimingPlan = asc.currentTimingPlanJson
            s.sendto(currentTimingPlan.encode(),address)
        else: print("Invalid message received!")
    s.close()

if __name__ == "__main__":
    main()    