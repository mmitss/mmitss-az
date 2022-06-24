"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

signal-coordination-request-generator.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This is a wrapper module for generating signal coordination requests. It performs
following functions:
(1) Check whether there is active coordination plan or not
(2) Generate Virtual coordination plan for active coordination plan
(3) Update the coordination request and delete served coordination request
(4) Delete old coordination plan
***************************************************************************************
"""

import time
import json
import socket
import atexit
from Logger import Logger
from CoordinationPlanManager import CoordinationPlanManager
from CoordinationRequestManager import CoordinationRequestManager

def readConfigfile():
    # Read the Coordination config file into a json object:
    coordinationConfigFile = open("/nojournal/bin/mmitss-coordination-plan.json", 'r')
    coordinationConfigData = (json.load(coordinationConfigFile))
    
    # Close the config file:
    coordinationConfigFile.close()

    return coordinationConfigData

def destruct_logger(logger:Logger):
    logger.loggingAndConsoleDisplay("Signal Coordination Request Generator is shutting down now!")
    del logger

def main():
    # Read the config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
   
    # Close the config file:
    configFile.close()
    
    # Open a socket and bind it to the IP and port dedicated for this application:
    coordinationSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    mrpIp = config["HostIp"]
    port = config["PortNumber"]["SignalCoordination"]
    signalCoordination_commInfo = (mrpIp, port)
    coordinationSocket.bind(signalCoordination_commInfo)
    coordinationSocket.settimeout(0)
    
    # Get the PRS and PRSolver communication address
    prioritySolverAddress = (mrpIp, config["PortNumber"]["PrioritySolver"])
    priorityRequestServerAddress = (mrpIp, config["PortNumber"]["PriorityRequestServer"])
    
    # Get logging and console output variables
    consoleStatus = config["ConsoleOutput"]
    loggingStatus = config["Logging"]
    intersectionName = config["IntersectionName"]
    
    #Read Coordination config file
    coordinationConfigData = readConfigfile() 
    
    #Create instances for the class
    logger = Logger(consoleStatus, loggingStatus, intersectionName)
    atexit.register(lambda: destruct_logger(logger))
    coordinationPlanManager = CoordinationPlanManager(coordinationConfigData, config, logger)
    coordinationRequestManager = CoordinationRequestManager(config, logger)
    
    while True:
        # Receive data on the socket
        # Load the received data into a json object
        # Send the split data to the PRSolver
        try:
            data, address = coordinationSocket.recvfrom(10240)
            data = data.decode()
            receivedMessage = json.loads(data)
            if receivedMessage["MsgType"]=="CoordinationPlanRequest":
                logger.loggingAndConsoleDisplay("Received Coordination Plan Request from PRSolver")
                splitData = coordinationPlanManager.getSplitData()
                if bool(splitData):
                    coordinationSocket.sendto(splitData.encode(), prioritySolverAddress)
                    logger.loggingAndConsoleDisplay("Sent Split data to PRSolver")
                    
        # Check if it is required to obtain active coordination plan or not
        # Send the split data to the PRSolver
        except:
            if bool(coordinationPlanManager.checkActiveCoordinationPlan()):
                coordinationPlanManager.updateCoordinationConfigData(readConfigfile())
                coordinationParametersDictionary = coordinationPlanManager.getActiveCoordinationPlan()
                coordinationRequestManager.getCoordinationParametersDictionary(coordinationParametersDictionary)
                splitData = coordinationPlanManager.getSplitData()
                if bool(splitData):
                    coordinationSocket.sendto(splitData.encode(), prioritySolverAddress)
                    logger.loggingAndConsoleDisplay("Sent Split data to PRSolver")             
                    
            # Check if it is required to generate virtual coordination requests at the beginning of each cycle
            #  Formulate a json string for coordination requests and sends it to the PRS
            if bool(coordinationRequestManager.checkCoordinationRequestSendingRequirement()):
                coordinationPriorityRequestJsonString = coordinationRequestManager.generateVirtualCoordinationPriorityRequest()
                coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)
                logger.loggingAndConsoleDisplay("Virtual Coorination Request is Sent to PRS")
                print("Coordination request is following:\n", coordinationPriorityRequestJsonString)
            # Check if it is required to generate coordination requests to avoid PRS timed-out
            # Formulate a json string for coordination requests and send it to the PRS 
            elif bool(coordinationRequestManager.checkUpdateRequestSendingRequirement()):
                coordinationPriorityRequestJsonString = coordinationRequestManager.generateUpdatedCoordinationPriorityRequest()
                coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)
                logger.loggingAndConsoleDisplay("Sent updated Coordination Request to avoid PRS timed-out")
                print("Coordination request is following:\n", coordinationPriorityRequestJsonString)
            # The method updates ETA for each coordination request
            # The method deletes the old coordination requests.
            # The method sends the coordination requests list in a JSON formate to the PRS after deleting the old requests
            # The method deletes old coordination Plan
            else:
                coordinationRequestManager.updateETAInCoordinationRequestTable()
                if bool(coordinationRequestManager.deleteTimeOutRequestFromCoordinationRequestTable()):
                    coordinationPriorityRequestJsonString = coordinationRequestManager.getCoordinationPriorityRequestDictionary()
                    if bool(coordinationPriorityRequestJsonString):
                        coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)
                        logger.looging(coordinationPriorityRequestJsonString)
                        logger.loggingAndConsoleDisplay("Sent coordination request list to PRS after deletion process")
                        print("Coordination request is following:\n", coordinationPriorityRequestJsonString)
                if bool(coordinationPlanManager.checkTimedOutCoordinationPlanClearingRequirement()):
                    coordinationRequestManager.clearTimedOutCoordinationPlan()
                    coordinationClearRequestJsonString = coordinationRequestManager.getCoordinationClearRequestDictionary()
                    coordinationSocket.sendto(coordinationClearRequestJsonString.encode(), priorityRequestServerAddress)
                    logger.loggingAndConsoleDisplay("Sent coordination clear request list to PRS since active coordination plan is timed-out")
                    print("Coordination request is following:\n", coordinationPriorityRequestJsonString)                
            time.sleep(1)
    coordinationSocket.close()
    
    
if __name__ == "__main__":
    main()  