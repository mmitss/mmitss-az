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
  1. This script is the demonstration of Prioririty Request Server API.
*/

#include <iostream>
#include <fstream>
#include "PriorityRequestServer.h"
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

    PriorityRequestServer PRS;
    SignalRequest signalRequest;
    SignalStatus signalStatus;

    UdpSocket PRSSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PriorityRequestServer"].asInt()), 1, 0);
    const int ssmReceiverPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt());
    const int solverPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PrioritySolver"].asInt());
    const int messageDistributorPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageDistributor"].asInt());
    const int dataCollectorPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["DataCollector"].asInt());

   
    char receiveBuffer[15360];

    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    const string messageDistributorIP = jsonObject_config["MessageDistributorIP"].asString();

    int msgType{};
    bool timedOutOccur{};
    std::string ssmJsonString{};
    std::string solverJsonString{};
    std::string systemPerformanceDataCollectorJsonString{};



    while (true)
    {
        timedOutOccur = PRSSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));

        if (timedOutOccur == false)
        {
            std::string receivedJsonString(receiveBuffer);
            msgType = PRS.getMessageType(receivedJsonString);

            if (msgType == MsgEnum::DSRCmsgID_srm)
            {
                
                signalRequest.json2SignalRequest(receivedJsonString);
                PRS.managingSignalRequestTable(signalRequest); 
                ssmJsonString = PRS.createSSMJsonString(signalStatus);
                PRSSocket.sendData(LOCALHOST, static_cast<short unsigned int>(ssmReceiverPortNo), ssmJsonString);
                PRSSocket.sendData(messageDistributorIP, static_cast<short unsigned int>(messageDistributorPortNo), ssmJsonString);
                PRSSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), ssmJsonString);
                solverJsonString = PRS.createJsonStringForPrioritySolver();
                PRSSocket.sendData(LOCALHOST, static_cast<short unsigned int>(solverPortNo), solverJsonString);
                PRS.loggingData(receivedJsonString);
            }
        }

        else
        {
            /* 
	            - Delete vehicle info from Active Request Table if Infrustracture doesn't receive and SRM for predefined time
                - After the request, if Active request table is empty then send clear request message to PRSolver
            */
            if (PRS.shouldDeleteTimedOutRequestfromActiveRequestTable() == true)
            {
                PRS.deleteTimedOutRequestfromActiveRequestTable();

                PRS.printvector();
                if (PRS.sendClearRequest() == true)
                {
                    solverJsonString = PRS.createJsonStringForPrioritySolver();
                    PRSSocket.sendData(LOCALHOST, static_cast<short unsigned int>(solverPortNo), solverJsonString);
                }
            }
            
            if(PRS.sendSystemPerformanceDataLog()== true)
            {
                systemPerformanceDataCollectorJsonString = PRS.createJsonStringForSystemPerformanceDataLog();
                PRSSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), systemPerformanceDataCollectorJsonString);
                std::cout << "System Performance Data Log" << systemPerformanceDataCollectorJsonString << std::endl;
            }

            if (PRS.updateETA() == true)
            {
                PRS.updateETAInActiveRequestTable();
                ssmJsonString = PRS.createSSMJsonString(signalStatus);                
                PRSSocket.sendData(LOCALHOST, static_cast<short unsigned int>(ssmReceiverPortNo), ssmJsonString);
                PRSSocket.sendData(messageDistributorIP, static_cast<short unsigned int>(messageDistributorPortNo), ssmJsonString);
            }

        }
    }

    return 0;
}