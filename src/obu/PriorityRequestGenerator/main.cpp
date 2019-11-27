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
    Json::Value jsonObject_config;
	Json::Reader reader;
	std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject_config);
    
    PriorityRequestGenerator PRG;
    MapManager mapManager;
    PriorityRequestGeneratorStatus prgStatus;
    BasicVehicle basicVehicle;
    SignalStatus signalStatus;
    SignalRequest signalRequest;

    //Socket Communication
    UdpSocket priorityRequestGeneratorSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PriorityRequestGenerator"].asInt()));
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    const int srmReceiverPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt());
    const int prgStatusReceiverPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["HMIController"].asInt());
    char receiveBuffer[5120];
    std::string srmJsonString;
    std::string prgStatusJsonString;

    while (true)
    {
        priorityRequestGeneratorSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);

        if (PRG.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_bsm)
        {
            basicVehicle.json2BasicVehicle(receivedJsonString);
            PRG.getVehicleInformationFromMAP(mapManager, basicVehicle);
            if (PRG.shouldSendOutRequest(basicVehicle) == true)
            {
                srmJsonString = PRG.createSRMJsonObject(basicVehicle, signalRequest, mapManager);
                priorityRequestGeneratorSocket.sendData(LOCALHOST, static_cast<short unsigned int>(srmReceiverPortNo), srmJsonString);
                std::cout << "SRM is sent" << std::endl;
            }
            mapManager.updateMapAge();
            mapManager.deleteMap();
            prgStatusJsonString = prgStatus.priorityRequestGeneratorStatus2Json(PRG, basicVehicle, mapManager);
            std::cout << prgStatusJsonString << std::endl;
            priorityRequestGeneratorSocket.sendData(LOCALHOST, static_cast<short unsigned int>(prgStatusReceiverPortNo), prgStatusJsonString);
            std::cout << "Message sent to HMI Conrtoller" << std::endl;
            // PRG.printART();
        }

        else if (PRG.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_map)
        {
            mapManager.json2MapPayload(receivedJsonString);
            mapManager.maintainAvailableMapList();
            // mapManager.printAvailableMapList();
            std::cout << "Map is received" << std::endl;
        }

        else if (PRG.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_ssm)
        {
            signalStatus.json2SignalStatus(receivedJsonString);
            PRG.creatingSignalRequestTable(signalStatus);
            std::cout << "SSM is received" << std::endl;
        }
    }
}