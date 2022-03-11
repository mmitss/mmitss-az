/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  priority-request-generator-main.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the demonstration of Prioririty Request Generator API.
*/

#include "PriorityRequestGenerator.h"
#include "PriorityRequestGeneratorStatus.h"
#include <UdpSocket.h>


int main()
{
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);        
    delete reader;

    PriorityRequestGenerator PRG;
    MapManager mapManager;
    PriorityRequestGeneratorStatus prgStatus;
    BasicVehicle basicVehicle;
    SignalStatus signalStatus;
    SignalRequest signalRequest;

    //Socket Communication
    UdpSocket priorityRequestGeneratorSocket(static_cast<short unsigned int>(jsonObject["PortNumber"]["PriorityRequestGenerator"].asInt()));
    const string HostIP = jsonObject["HostIp"].asString();
    const string HMIControllerIP = jsonObject["HMIControllerIP"].asString();
    const int dataCollectorPort = static_cast<short unsigned int>(jsonObject["PortNumber"]["DataCollector"].asInt());
    const int srmReceiverPortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt());
    const int prgStatusReceiverPortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["HMIController"].asInt());
    
    char receiveBuffer[40960];
    string srmJsonString{};
    string prgStatusJsonString{};
    int msgType{};
    
    while (true)
    {
        priorityRequestGeneratorSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        string receivedJsonString(receiveBuffer);
        msgType = PRG.getMessageType(receivedJsonString);

        if (msgType == static_cast<int>(msgType::lightSirenStatus))
            PRG.setLightSirenStatus(receivedJsonString);

        else if (msgType == MsgEnum::DSRCmsgID_bsm)
        {
            basicVehicle.json2BasicVehicle(receivedJsonString);
            PRG.getVehicleInformationFromMAP(mapManager, basicVehicle);
            
            // Formulate srm JSON string, if requires and send it over the socket.
            if (PRG.checkPriorityRequestSendingRequirementStatus())
            {
                srmJsonString = PRG.createSRMJsonString(basicVehicle, signalRequest, mapManager);
                priorityRequestGeneratorSocket.sendData(HostIP, static_cast<short unsigned int>(srmReceiverPortNo), srmJsonString);
                priorityRequestGeneratorSocket.sendData(HostIP, static_cast<short unsigned int>(dataCollectorPort), srmJsonString);
            }
            
            // Update the Map status (MapAge, or delete old Map)
            PRG.manageMapStatusInAvailableMapList(mapManager);
            
            // Formulate PRGStatus JSON string and send it to HMI-Controller
            prgStatusJsonString = prgStatus.priorityRequestGeneratorStatus2Json(PRG, basicVehicle);
            priorityRequestGeneratorSocket.sendData(HMIControllerIP, static_cast<short unsigned int>(prgStatusReceiverPortNo), prgStatusJsonString);
            PRG.loggingData(prgStatusJsonString);
        }

        //The received MAP will be either added or updated in the availableMapList.
        else if (msgType == MsgEnum::DSRCmsgID_map)
        {
            mapManager.json2MapPayload(receivedJsonString);
            mapManager.maintainAvailableMapList();
        }

        // The active request table (ART) will be managed for the connected vehicle.
        else if (msgType == MsgEnum::DSRCmsgID_ssm)
        {

            signalStatus.json2SignalStatus(receivedJsonString);
            PRG.creatingSignalRequestTable(signalStatus);
            signalStatus.reset();
        }
    }
    
    priorityRequestGeneratorSocket.closeSocket();
    return 0;
}