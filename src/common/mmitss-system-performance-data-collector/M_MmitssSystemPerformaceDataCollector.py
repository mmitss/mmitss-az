import socket
import json

import datetime 
import csv
import pandas as pd
from SystemPerformanceDataCollector import SystemPerformanceDataCollector

def main():
    # Read the config file into a json object:
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))
    # Close the config file:
    configFile.close()
    #read the ip address and port number from the config file
    hostIP = config["HostIp"]
    port = config["PortNumber"]["SystemPerformanceDataCollector"]
    communicationInfo = (hostIP, port)    
    # Open a socket and bind it to the IP and port dedicated for this application:
    dataCollectorSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dataCollectorSocket.bind(communicationInfo)
    
    applicationPlatform = config["ApplicationPlatform"]
    #creating an instance of SystemPerformanceDataCollector class
    dataCollector = SystemPerformanceDataCollector(applicationPlatform)
    
    while True:
        # Receive data on the MsgTransceiver
        data, address = dataCollectorSocket.recvfrom(10240)
        data = data.decode()
        # Load the received data into a json object
        receivedMsg = json.loads(data)
        
        # dataLog = open(fileName,'a+')
        if receivedMsg["MsgType"] == "VehicleDataLog":
            dataCollector.loggingVehicleSideData(receivedMsg)
        
        elif receivedMsg["MsgType"] == "IntersectionDataLog":
            dataCollector.loggingRoadSideData(receivedMsg)
            
    dataCollectorSocket.close()

if __name__ == "__main__":
    main() 
