import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
import json
import socket
from OptimizationResultsManager import OptimizationResultsManager



def main():
    # Read the config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
   
    # Close the config file:
    configFile.close()
    
    # Open a socket and bind it to the IP and port dedicated for this application:
    timePhaseDiagramSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    hostIp = config["HostIp"]
    port = config["PortNumber"]["SignalCoordination"]
    timePhaseDiagram_commInfo = (hostIp, port)
    timePhaseDiagramSocket.bind(timePhaseDiagram_commInfo)
    
    optimizationResultsManager = OptimizationResultsManager()
    optimizationResultsManager.readOptimizationResultsFile('/nojournal/bin/OptimizationResults.txt')

    # while True:
    #     data, address = timePhaseDiagramSocket.recvfrom(10240)
    #     data = data.decode()
    #     receivedMessage = json.loads(data)
    #     if receivedMessage["MsgType"]=="Schedule":
    #         pass
            
if __name__ == '__main__':
    main()
