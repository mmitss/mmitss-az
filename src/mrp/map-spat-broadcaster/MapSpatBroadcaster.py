import socket
import time
import json
import Ntcip1202v2Blob
import MapSpatBroadcasterConfig
import Spat

def main():

    # Read a config file by creating an object of the time MapSpatBroadcasterConfig
    config = MapSpatBroadcasterConfig.MapSpatBroadcasterConfig("MapSpatBroadcasterConfig.json")


    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ip = config.getMrpIP()
    port = 6053
    MapSpatBroadcastAddress = (ip, port)
    s.bind(MapSpatBroadcastAddress)
    s.settimeout(3)

    msgSenderIP = '127.0.0.1'
    msgSenderPort = 10001
    msgSenderAddress = (msgSenderIP, msgSenderPort)

    msgEncoderIP = '127.0.0.1'
    msgEncoderPort = 10004
    msgEncoderAddress = (msgEncoderIP, msgEncoderPort)

    trafficControllerObserverIP = '127.0.0.1'
    trafficControllerObserverPort = 20006
    trafficControllerObserverAddress = (trafficControllerObserverIP,trafficControllerObserverPort)

    # Store map payload in a string
    mapPayload = config.getMapPayload()

    # Create an empty Ntcip1202v2Blob object to store the information to be received from the signal controller:
    currentBlob = Ntcip1202v2Blob.Ntcip1202v2Blob()

    # Create an object of Spat class filled with static information:
    spatObject = Spat.Spat()
    spatObject.setIntersectionID(config.getIntersectionID())
    spatObject.setRegionalID(config.getRegionalID())

    msgCnt = 0
    while True:
        try:
            spatBlob, addr = s.recvfrom(1024)            
            if addr[0] == config.getControllerIP():
                currentBlob.processNewData(spatBlob)
                if(msgCnt < 127):
                    msgCnt = msgCnt + 1
                else: msgCnt = 0
                spatObject.setmsgCnt(msgCnt)
                spatObject.fillSpatInformation(currentBlob)
                spatJsonString = spatObject.Spat2Json()
                
                s.sendto(spatJsonString.encode(), msgEncoderAddress)
                s.sendto(spatJsonString.encode(), trafficControllerObserverAddress)
                s.sendto(mapPayload.encode(), msgSenderAddress)

                print("Sent SPaT JSON to msgEncoder and trafficControllerObserver, and MAP payload to msgSender.")
        except socket.timeout:
            print("No packets received from the Traffic Signal Controller. Check:\n1. Physical connection between CVCP and Traffic Signal Controller.\n2. Server IP in MM-1-5-1 of the Signal Controller must match the IP address of CVCP.\n3. Address in MM-1-5-3 must be set to 6053.\n4. Controller must be power-cycled after changes in internal configuration.\n")

if __name__ == "__main__":
    main()
    

