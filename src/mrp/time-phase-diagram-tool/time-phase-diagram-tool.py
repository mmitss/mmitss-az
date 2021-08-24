"""
***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

time-phase-diagram-tool.py
Created by: Debashis Das
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This is a wrapper module for generating time-phase diagram. It performs
following functions:
(1) Creates "/nojournal/bin/performance-measurement-diagrams/time-phase-diagram" directory if requires
(2) Generates time-phase diagram
(3) Checks for updates of the parameter in the config file after a specified time interval
***************************************************************************************
"""

import json, time
import socket
import os
from OptimizationResultsManager import OptimizationResultsManager
from TimePhaseDiagramManager import TimePhaseDiagramManager

def createDirectory():
    path = "/nojournal/bin/performance-measurement-diagrams/time-phase-diagram"

    if not os.path.exists(path):
        os.makedirs(path)

def checkTimePhaseDigramGeneratingStatus(configFile):  
    diagramGenerationStatus = False
    
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = json.load(configFile)
    configFile.close()
    
    diagramGenerationStatus = config["TimePhaseDiagram"]
    
    return diagramGenerationStatus
    
def main():
    # Read the config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = json.load(configFile)

    # Close the config file:
    configFile.close()

    # Open a socket and bind it to the IP and port dedicated for this application:
    timePhaseDiagramSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = config["HostIp"]
    port = config["PortNumber"]["TimePhaseDiagramTool"]
    timePhaseDiagram_commInfo = (hostIp, port)
    timePhaseDiagramSocket.bind(timePhaseDiagram_commInfo)
    timePhaseDiagramSocket.settimeout(2)
    
    timeGapBetweenDiagramGenerationStatusChecking = config['SystemPerformanceTimeInterval']
    diagramGenerationStatus = checkTimePhaseDigramGeneratingStatus(configFile)
    generateDiagramStatusCheckingTime = time.time()
    
    createDirectory()
    optimizationResultsManager = OptimizationResultsManager()
    timePhaseDiagramManager = TimePhaseDiagramManager()
        
    timePhaseDiagramManager.removeOldestDiagram()
    
    try:
        while True:
            try:
                data, address = timePhaseDiagramSocket.recvfrom(1024)
                data = data.decode()
                receivedMessage = json.loads(data)
                if receivedMessage["MsgType"]=="TimePhaseDiagram" and receivedMessage["OptimalSolutionStatus"]== True and bool(diagramGenerationStatus):
                    optimizationResultsManager.readOptimizationResultsFile()
                    
                elif receivedMessage["MsgType"]=="TimePhaseDiagram" and receivedMessage["OptimalSolutionStatus"]== False and bool(diagramGenerationStatus):
                    timePhaseDiagramManager.timePhaseDiagramMethodForNonOptimalSolution()
                    
            except:
                if (time.time() - generateDiagramStatusCheckingTime) >= timeGapBetweenDiagramGenerationStatusChecking:
                    diagramGenerationStatus = checkTimePhaseDigramGeneratingStatus(configFile)
                    generateDiagramStatusCheckingTime = time.time()
                     
    except KeyboardInterrupt:
        print("Finished with ctrl+c")

    timePhaseDiagramSocket.close()

if __name__ == '__main__':
    main()