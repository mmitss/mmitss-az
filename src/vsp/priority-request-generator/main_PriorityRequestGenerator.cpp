/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  main.cpp
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
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "msgEnum.h"
#include "json/json.h"

int main()
{
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);        

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
        
    delete reader;
    char receiveBuffer[40960];
    std::string srmJsonString{};
    std::string prgStatusJsonString{};
    int msgType{};
    PRG.getLoggingStatus();
    PRG.setVehicleType();

    while (true)
    {
        priorityRequestGeneratorSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);
        msgType = PRG.getMessageType(receivedJsonString);

        if (msgType == static_cast<int>(msgType::lightSirenStatus))
            PRG.setLightSirenStatus(receivedJsonString);

        else if (msgType == MsgEnum::DSRCmsgID_bsm)
        {
            std::cout << "Received BSM: " << receivedJsonString << std::endl;
            basicVehicle.json2BasicVehicle(receivedJsonString);
            PRG.getVehicleInformationFromMAP(mapManager, basicVehicle);
            if (PRG.shouldSendOutRequest() == true)
            {
                srmJsonString = PRG.createSRMJsonObject(basicVehicle, signalRequest, mapManager);
                priorityRequestGeneratorSocket.sendData(HostIP, static_cast<short unsigned int>(srmReceiverPortNo), srmJsonString);
                priorityRequestGeneratorSocket.sendData(HostIP, static_cast<short unsigned int>(dataCollectorPort), srmJsonString);
            }
            // mapManager.updateMapAge();
            // mapManager.deleteMap();
            PRG.manageMapStatusInAvailableMapList(mapManager);
            prgStatusJsonString = prgStatus.priorityRequestGeneratorStatus2Json(PRG, basicVehicle);
            priorityRequestGeneratorSocket.sendData(HMIControllerIP, static_cast<short unsigned int>(prgStatusReceiverPortNo), prgStatusJsonString);
        }

        else if (msgType == MsgEnum::DSRCmsgID_map)
        {
            mapManager.json2MapPayload(receivedJsonString);
            mapManager.maintainAvailableMapList();
        }

        else if (msgType == MsgEnum::DSRCmsgID_ssm)
        {
            signalStatus.json2SignalStatus(receivedJsonString);
            PRG.creatingSignalRequestTable(signalStatus);
            std::cout << "SSM is received " << std::endl;
            signalStatus.reset();
        }
    }
}