import json, time
import socket
import os
from OptimizationResultsManager import OptimizationResultsManager

def createDirectory():
    path = "/nojournal/bin/performance-measurement-diagrams/time-phase-diagram"

    if not os.path.exists(path + "/archive"):
        os.makedirs(path + "/archive")

def checkTimePhaseDigramGeneratingStatus(configFile):  
    diagramGenerationStatus = False
    
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = json.load(configFile)
    configFile.close()
    
    diagramGenerationStatus = config["PerformanceMeasurementDiagram"]
    
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
    
    try:
        while True:
            try:
                data, address = timePhaseDiagramSocket.recvfrom(1024)
                data = data.decode()
                receivedMessage = json.loads(data)
                if receivedMessage["MsgType"]=="TimePhaseDiagram" and receivedMessage["OptimalSolutionStatus"]== True and bool(diagramGenerationStatus):
                    optimizationResultsManager.readOptimizationResultsFile()
                    
                elif receivedMessage["MsgType"]=="TimePhaseDiagram" and receivedMessage["OptimalSolutionStatus"]== False and bool(diagramGenerationStatus):
                    optimizationResultsManager.generateTimePhaseDiagramForNonOptimalSolution()
                    
            except:
                if (time.time() - generateDiagramStatusCheckingTime) >= timeGapBetweenDiagramGenerationStatusChecking:
                    diagramGenerationStatus = checkTimePhaseDigramGeneratingStatus(configFile)
                    generateDiagramStatusCheckingTime = time.time() 
    except KeyboardInterrupt:
        print("Finished with ctrl+c")

    timePhaseDiagramSocket.close()

if __name__ == '__main__':
    main()