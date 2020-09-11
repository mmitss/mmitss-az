'''
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  MapSpatBroadcaster.py  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. This is the initial revision. MapSpatBroadcaster does the follwoing tasks: 
    -> Read configuration data: controllerIP, selfIP, mapPayload, regionalID, and IntersectionID.
    -> Receive SPAT data from the traffic controller (currently NTCIP1202v2 Blob: defined in separate class)
    -> Calculate the data required for other mmitss components and J2735 SPAT message.
    -> Formulate a json string and send it the data to trafficControllerInterface, MsgEncoder.
    -> Send the MapPayload as it is to MsgEncoder. and send the Map Json string to the Message Distributor.
    -> ### IMPORTANT ### If the format of NTCIP1202 blob changes in the future (for example, NTCIP1202v3), a new class will be 
       required to be created that could be used in similar manner like NTCIP1202v2Blob class.
'''

import socket
import time
import json
import Ntcip1202v2Blob
import Spat
import MmitssSpat

def main():

    # Read a config file by creating an object of the time MapSpatBroadcasterConfig
    configFile = open("/nojournal/bin/mmitss-phase3-master-config.json", 'r')
    config = (json.load(configFile))

    # Establish a socket and bind it to IP and port
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    mrpIp = config["HostIp"]
    MapSpatBroadcastAddress = (mrpIp, config["PortNumber"]["MapSPaTBroadcaster"])
    s.bind(MapSpatBroadcastAddress)
    
    snmpEngineAddress = (mrpIp, config["PortNumber"]["SnmpEngine"])
    msgEncoderAddress = (mrpIp, config["PortNumber"]["MessageTransceiver"]["MessageEncoder"])
    dataCollectorAddress = (config["DataCollectorIP"], config["PortNumber"]["DataCollector"])
    msgDistributorAddress = (config["MessageDistributorIP"], config["PortNumber"]["MessageDistributor"])
    tci_currPhaseAddress = (mrpIp, config["PortNumber"]["TrafficControllerCurrPhaseListener"])
    
    # Read controllerIp from the config file and store it.
    controllerIp = config["SignalController"]["IpAddress"]

    # If the controller is Econolite, the broadcasting of SPAT needs to be enabled manually through SnmpSet request.
    # In such cases, the M_SnmpEngine component needs to be running before running the MapSpatBroadcaster
    if ((config["SignalController"]["Vendor"]).lower() == "econolite"):
        enableSpatJsonRequest = json.dumps({"MsgType": "SnmpSetRequest","OID": "1.3.6.1.4.1.1206.3.5.2.9.44.1.1","Value": 6})
        s.sendto(enableSpatJsonRequest.encode(),snmpEngineAddress)
        
    clientsJson = json.load(open('/nojournal/bin/mmitss-data-external-clients.json','r'))
    clients_spatBlob = clientsJson["spat"]["blob"]
    clients_spatJson = clientsJson["spat"]["json"]

    # Store map payload in a string
    mapPayload = config["MapPayload"]
    intersectionID = config["IntersectionID"]
    intersectionName = config["IntersectionName"]
    regionalID = config["RegionalID"]
    
    # Store Map Message Json:
    mapJson = json.dumps({"MsgType": "MAP","IntersectionName": intersectionName,
                            "IntersectionID": intersectionID,"MapPayload": mapPayload,"RegionalID": regionalID})

    permissiveEnabled = config["SignalController"]["PermissiveEnabled"]
    splitPhases = config["SignalController"]["SplitPhases"]

    # Get inactive vehicle and ped phases from the configuration file
    inactiveVehPhases = config["SignalController"]["InactiveVehPhases"]
    inactivePedPhases = config["SignalController"]["InactivePedPhases"]

    # Create an empty Ntcip1202v2Blob object to store the information to be received from the signal controller:
    currentBlob = Ntcip1202v2Blob.Ntcip1202v2Blob(permissiveEnabled, splitPhases, inactiveVehPhases, inactivePedPhases)
    # NOTE: Think about V2 and V3. Encapsulation.

    # Create an object of Spat class filled with static information:
    spatObject = Spat.Spat()
    spatObject.setIntersectionID(intersectionID)
    spatObject.setRegionalID(regionalID)

    mmitssSpatObject = MmitssSpat.MmitssSpat()
    mmitssSpatObject.setIntersectionID(intersectionID)
    mmitssSpatObject.setRegionalID(regionalID)

    msgCnt = 0
    spatMapMsgCount = 0

    print("Waiting for packets received from the Traffic Signal Controller. Check:\n1. Physical connection between the Host and the Traffic Signal Controller.\n2. Server IP in MM-1-5-1 of the Signal Controller must match the IP address of the Host.\n3. Address in MM-1-5-3 must be set to 6053.\n4. Controller must be power-cycled after changes in internal configuration.\n")

    spatBroadcastSuccessFlag = False

    while True:
        data, addr = s.recvfrom(1024)


        try:
            data = data.decode()
            print(data)
            data = json.loads(data)
            if(data["MsgType"]=="ActivePhaseControlSchedule"):
                phaseControlSchedule = data
                mmitssSpatObject.extract_local_phase_control_schedule(phaseControlSchedule)
        except (UnicodeDecodeError, AttributeError):
            spatBlob = data
            if spatBroadcastSuccessFlag == False:
                print("\nStarted receiving packets from the Signal Controller. SPAT Broadcast Set Successfully!")
                spatBroadcastSuccessFlag = True
            # Send spat blob to external clients:       
            if addr[0] == controllerIp:
                for client in clients_spatBlob:
                    address = (client["IP"], client["Port"])
                    s.sendto(spatBlob, address)
                        
                currentBlob.processNewData(spatBlob)
                if(msgCnt < 127):
                    msgCnt = msgCnt + 1
                else: msgCnt = 0

                # Check if TCI is running:
                tciIsRunning = checkIfProcessRunning("M_TrafficControllerInterface")
                snmpEngineIsRunning = checkIfProcessRunning("M_SnmpEngine")
                scheduleIsActive = mmitssSpatObject.isScheduleActive

                if (tciIsRunning and snmpEngineIsRunning and scheduleIsActive):
                    currentSpatObject = mmitssSpatObject
                    mmitssSpatObject.update_current_phases(currentBlob)
                    currentSpatObject.update_local_phase_control_schedule()
                else: currentSpatObject = spatObject

                currentSpatObject.setmsgCnt(msgCnt)
                currentSpatObject.fillSpatInformation(currentBlob)
                spatJsonString = currentSpatObject.Spat2Json()
                currentPhasesJson = json.dumps(currentBlob.getCurrentPhasesDict())
                s.sendto(spatJsonString.encode(), msgEncoderAddress)
                s.sendto(spatJsonString.encode(), dataCollectorAddress)
                s.sendto(currentPhasesJson.encode(), tci_currPhaseAddress)
                            
                # Send spat json to external clients:
                for client in clients_spatJson:
                    address = (client["IP"], client["Port"])
                    s.sendto(spatJsonString.encode(), address)

                spatMapMsgCount = spatMapMsgCount + 1
                if spatMapMsgCount > 9:
                    s.sendto(mapPayload.encode(), msgEncoderAddress)
                    s.sendto(mapJson.encode(), msgDistributorAddress)
                    print("MapSpatBroadcaster Sent Map")
                    spatMapMsgCount = 0
        
def checkIfProcessRunning(processName):
    '''
    Check if there is any running process that contains the given name processName.
    '''
    #Iterate over the all the running process
    for proc in psutil.process_iter():
        try:
            # Check if process name contains the given name string.
            if processName.lower() in proc.name().lower():
                return True
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass
    return False
                

if __name__ == "__main__":
    main()
    

