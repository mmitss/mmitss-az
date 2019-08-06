import socket
import time
import json
from Ntcip1202v2Blob import Ntcip1202v2Blob
from MapSpatBroadcasterConfig import MapSpatBroadcasterConfig
from Spat import Spat

def main():

    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ip = '10.254.56.49'
    port = 6053
    s.bind((ip, port))

    # Read a config file by creating an object of the time MapSpatBroadcasterConfig
    config = MapSpatBroadcasterConfig("MapSpatBroadcasterConfig.json")

    # Create an empty Ntcip1202v2Blob object to store the information to be received from the signal controller:
    currentBlob = Ntcip1202v2Blob()

    # Create an object of Spat class filled with static information:
    spatObject = Spat()
    spatObject.setIntersectionID(config.getIntersectionID())
    spatObject.setRegionalID(config.getRegionalID())

    msgCnt = 0
    while True:
        spatBlob, addr = s.recvfrom(1024)
        if addr[0] == config.getControllerIP():
            currentBlob.processNewData(spatBlob)
            if(msgCnt < 127):
                msgCnt = msgCnt + 1
            else: msgCnt = 0
            spatObject.setmsgCnt(msgCnt)
            spatObject.fillSpatInformation(currentBlob)
            #spatJsonString = spatObject.Spat2Json()
            print(currentBlob.getVehCurrState())
            print(currentBlob.getVehStartTime())
            print(currentBlob.getVehElapsedTime())
            print(currentBlob.getVehMinEndTime())
            print(currentBlob.getVehMaxEndTime())
            print ("")

if __name__ == "__main__":
    main()
    

