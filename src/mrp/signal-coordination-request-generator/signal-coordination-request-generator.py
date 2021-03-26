"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

SignalCoordinationRequestGenerator.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This is a wrapper module for generating signal coordination request software. It performs
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
from CoordinationPlanManager import CoordinationPlanManager
from CoordinationRequestManager import CoordinationRequestManager

def main():
    # Read the config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))

    # Read the Coordination config file into a json object:
    coordinationConfigFile = open("/nojournal/bin/mmitss-coordination-plan.json", 'r')
    coordinationConfigData = (json.load(coordinationConfigFile))
    
    # Close the config file:
    configFile.close()
    coordinationConfigFile.close()
    
    
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
    
    #Create instances for the class
    coordinationPlanManager = CoordinationPlanManager(coordinationConfigData, config)
    coordinationRequestManager = CoordinationRequestManager(config)
    
    while True:
        # Receive data on the socket
        # Load the received data into a json object
        # Send the split data to the PRSolver
        try:
            data, address = coordinationSocket.recvfrom(10240)
            data = data.decode()
            receivedMessage = json.loads(data)
            if receivedMessage["MsgType"]=="CoordinationPlanRequest":
                splitData = coordinationPlanManager.getSplitData()
                if bool(splitData):
                    coordinationSocket.sendto(splitData.encode(), prioritySolverAddress)
                
        # Check if it is required to obtain active coordination plan or not
        # Send the split data to the PRSolver
        except:
            if bool(coordinationPlanManager.checkActiveCoordinationPlan()):
                coordinationParametersDictionary = coordinationPlanManager.getActiveCoordinationPlan()
                coordinationRequestManager.getCoordinationParametersDictionary(coordinationParametersDictionary)
                splitData = coordinationPlanManager.getSplitData()
                if bool(splitData):
                    coordinationSocket.sendto(splitData.encode(), prioritySolverAddress)
                    
            # Check if it is required to generate virtual coordination requests at the beginning of each cycle
            #  Formulate a json string for coordination requests and sends it to the PRS
            if bool(coordinationRequestManager.checkCoordinationRequestSendingRequirement()):
                coordinationPriorityRequestJsonString = coordinationRequestManager.generateVirtualCoordinationPriorityRequest()
                coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)
                
            # Check if it is required to generate coordination requests to avoid PRS timed-out
            # Formulate a json string for coordination requests and send it to the PRS 
            elif bool(coordinationRequestManager.checkUpdateRequestSendingRequirement()):
                coordinationPriorityRequestJsonString = coordinationRequestManager.generateUpdatedCoordinationPriorityRequest()
                coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)

            # The method updates ETA for each coordination request
            # The method deletes the old coordination requests.
            # The method sends the coordination requests list in a JSON formate to the PRS after deleting the old requests
            # The method deletes old coordination Plan
            else:
                coordinationRequestManager.updateETAInCoordinationRequestTable()
                if bool(coordinationRequestManager.deleteTimeOutRequestFromCoordinationRequestTable()):
                    coordinationPriorityRequestJsonString = coordinationRequestManager.getCoordinationPriorityRequestDictionary()
                    coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)
                
                if bool(coordinationPlanManager.checkTimedOutCoordinationPlanClearingRequirement()):
                    coordinationRequestManager.clearTimedOutCoordinationPlan()
                                    
            time.sleep(1)
    coordinationSocket.close()
    
    
if __name__ == "__main__":
    main()  