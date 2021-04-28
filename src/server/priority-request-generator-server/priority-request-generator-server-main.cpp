/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  priority-request-generator-server-main.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the demonstration of Prioririty Request Generator Server API.
*/
#include "PriorityRequestGeneratorServer.h"
#include <UdpSocket.h>

int main()
{
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);

    PriorityRequestGeneratorServer priorityRequestGeneratorServer;
    BasicVehicle basicVehicle;
    SignalStatus signalStatus;
    SignalRequest signalRequest;
    MapManager mapManager;
    UdpSocket priorityRequestGeneratorServerSocket(static_cast<short unsigned int>(jsonObject["PortNumber"]["PriorityRequestGeneratorServer"].asInt()), 1, 0);
    
    const string LOCALHOST = jsonObject["HostIp"].asString();
    const string HMIControllerIP = jsonObject["HMIControllerIP"].asString();
    const string messageDistributorIP = jsonObject["MessageDistributorIP"].asString();
    const int dataCollectorPort = static_cast<short unsigned int>(jsonObject["PortNumber"]["DataCollector"].asInt());
    const int srmReceiverPortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt());
    const int messageDistributorPortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["MessageDistributor"].asInt());
    delete reader;

    char receiveBuffer[40960];
    int msgType{};
    double timeStamp{};
    string srmJsonString{};
    string prgStatusJsonString{};
    bool timedOutOccur{};
   
    while (true)
    {
        timedOutOccur = priorityRequestGeneratorServerSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        if (timedOutOccur == false)
        {
            std::string receivedJsonString(receiveBuffer);
            msgType = priorityRequestGeneratorServer.getMessageType(receivedJsonString);
            /*
                - If received message is BSM, process the BSM (either add or update the vehicle informtaion in the list)
                - If vehicle is on an active map, SRM will be generated.
                - The SRM will be the sent to the respective user (msgDistributor, dataCollector, MsgEncoder etc.)
            */
            if (msgType == MsgEnum::DSRCmsgID_bsm)
            {
                basicVehicle.json2BasicVehicle(receivedJsonString);
                priorityRequestGeneratorServer.processBSM(basicVehicle);

                if (priorityRequestGeneratorServer.checkSrmSendingFlag() == true)
                {
                    srmJsonString = priorityRequestGeneratorServer.getSRMJsonString();
                    priorityRequestGeneratorServerSocket.sendData(LOCALHOST, static_cast<short unsigned int>(srmReceiverPortNo), srmJsonString);
                    priorityRequestGeneratorServerSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPort), srmJsonString);
                    priorityRequestGeneratorServerSocket.sendData(messageDistributorIP, static_cast<short unsigned int>(messageDistributorPortNo), srmJsonString);
                    timeStamp = getPosixTimestamp();
                    cout << "[" << fixed << showpoint << setprecision(4) << timeStamp << "] PRGServer sent SRM to MsgDistributor" << endl;
                    cout << "[" << fixed << showpoint << setprecision(4) << timeStamp << "]" << srmJsonString << endl;
                }
                // Delete the timed-out vehicle information from the list, if requires
                priorityRequestGeneratorServer.deleteTimedOutVehicleInformationFromPRGServerList();
            }

            /*
                - The received MAP will be either added or updated in the availableMapList for each connected vehicle.
            */
            else if (msgType == MsgEnum::DSRCmsgID_map)
                priorityRequestGeneratorServer.processMap(receivedJsonString, mapManager);

            /*
                - The active request table (ART) will be managed for each connected vehicle.
            */
            else if (msgType == MsgEnum::DSRCmsgID_ssm)                
                priorityRequestGeneratorServer.processSSM(receivedJsonString);
        }
        
        // Delete the timed-out vehicle information from the list, if requires
        else
            priorityRequestGeneratorServer.deleteTimedOutVehicleInformationFromPRGServerList();
    }
    
    priorityRequestGeneratorServerSocket.closeSocket();
    return 0;
}