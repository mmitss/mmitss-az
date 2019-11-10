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
#include <iostream>
#include <UdpSocket.h>
#include "msgEnum.h"

int main()
{
    PriorityRequestGenerator PRG;
    MapManager mapManager;
    BasicVehicle basicVehicle;
    SignalStatus signalStatus;
    SignalRequest signalRequest;

    //Socket Communication
    UdpSocket priorityRequestGeneratorSocket(20004);
    const string LOCALHOST = "127.0.0.1";
    const int receiverPortNo = 10003;
    char receiveBuffer[5120];
    std::string sendingJsonString;

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
                sendingJsonString = PRG.createSRMJsonObject(basicVehicle, signalRequest, mapManager);
                priorityRequestGeneratorSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);
                std::cout << "SRM is sent" << std::endl;
            }
            mapManager.deleteMap();
        }

        else if (PRG.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_map)
        {
            mapManager.json2MapPayload(receivedJsonString);
            mapManager.maintainAvailableMapList();
            mapManager.printAvailableMapList();
        }

        else if (PRG.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_ssm)
        {
            signalStatus.json2SignalStatus(receivedJsonString);
            PRG.creatingSignalRequestTable(signalStatus);
            std::cout << "SSM is received" << std::endl;
        }
    }
}