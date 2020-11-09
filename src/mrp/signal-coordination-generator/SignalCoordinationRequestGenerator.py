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
(1) 
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

    prioritySolverAddress = (mrpIp, config["PortNumber"]["PrioritySolver"])
    priorityRequestServerAddress = (mrpIp, config["PortNumber"]["PriorityRequestServer"])
    
    #Create instances for the class
    coordinationPlanManager = CoordinationPlanManager(coordinationConfigData)
    coordinationRequestManager = CoordinationRequestManager(config)
    
    while True:
        if bool(coordinationPlanManager.checkactiveCoordinationPlan()):
            coordinationParametersDictionary = coordinationPlanManager.getActiveCoordinationPlan()
            coordinationRequestManager.getCoordinationParametersDictionary(coordinationParametersDictionary)
            splitData = coordinationPlanManager.getSplitData()
            coordinationSocket.sendto(splitData.encode(), prioritySolverAddress)
        
        if bool(coordinationRequestManager.checkCoordinationRequestSendingRequirement()):
            coordinationPriorityRequestJsonString = coordinationRequestManager.generateVirtualCoordinationPriorityRequest()
            coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)
            
        elif bool(coordinationRequestManager.checkUpdateRequestSendingRequirement()):
            coordinationPriorityRequestJsonString = coordinationRequestManager.generateUpdatedCoordinationPriorityRequest()
            coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)

        else:
            coordinationRequestManager.updateETAInCoordinationRequestTable()
            if bool(coordinationRequestManager.deleteTimeOutRequestFromCoordinationRequestTable()):
                coordinationPriorityRequestJsonString = coordinationRequestManager.getCoordinationPriorityRequestDictionary()
                coordinationSocket.sendto(coordinationPriorityRequestJsonString.encode(), priorityRequestServerAddress)

                
        time.sleep(1)
    # coordinationPlanManager.getSplitData()
    
    
if __name__ == "__main__":
    main()  