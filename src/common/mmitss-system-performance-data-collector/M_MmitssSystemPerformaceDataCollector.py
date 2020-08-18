import socket
import json
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
    #check the platform (vehicle/intersection) on which applications are running
    applicationPlatform = config["ApplicationPlatform"]
    #creating an instance of SystemPerformanceDataCollector class
    dataCollector = SystemPerformanceDataCollector(applicationPlatform)
    
    while True:
        # Receive data over the socket
        data, address = dataCollectorSocket.recvfrom(10240)
        data = data.decode()
        # Load the received data into a json object
        receivedMsg = json.loads(data)
        print(receivedMsg)
        # Based on the message type, call the corresponding function 
        if receivedMsg["MsgType"] == "VehicleDataLog":
            dataCollector.loggingVehicleSideData(receivedMsg)
        
        elif receivedMsg["MsgType"] == "IntersectionDataLog":
            dataCollector.loggingRoadSideData(receivedMsg)
            
    dataCollectorSocket.close()

if __name__ == "__main__":
    main() 
