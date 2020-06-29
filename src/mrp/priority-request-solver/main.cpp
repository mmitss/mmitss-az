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
  1. 
*/

#include <UdpSocket.h>
#include <fstream>
#include "PriorityRequestSolver.h"
#include "SolverDataManager.h"
#include <json/json.h>

int main()
{
    Json::Value jsonObject_config;
    Json::Reader reader;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject_config);

    PriorityRequestSolver priorityRequestSolver;
    ScheduleManager scheduleManager;
    SolverDataManager solverDataManager;
    UdpSocket priorityRequestSolverSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PrioritySolver"].asInt()));
    UdpSocket priorityRequestSolver_To_TCI_Interface_Socket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PrioritySolverToTCIInterface"].asInt()));
    const int trafficControllerPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["TrafficControllerInterface"].asInt());
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    char receiveBuffer[5120];
    char receivedSignalStatusBuffer[5120];
    int msgType{};
    string tciJsonString{};

    priorityRequestSolver.logging();
    priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), priorityRequestSolver.getSignalTimingPlanRequestString());

    while (true)
    {
        priorityRequestSolverSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);
        // cout << "Received Json String " << receivedJsonString << endl;
        msgType = priorityRequestSolver.getMessageType(receivedJsonString);

        if (msgType == static_cast<int>(msgType::signalPlan))
        {
            // cout << "Received Signal Timing Plan " << endl;
            priorityRequestSolver.getCurrentSignalTimingPlan(receivedJsonString);
        }

        else if (msgType == static_cast<int>(msgType::priorityRequest))
        {
            priorityRequestSolver.createPriorityRequestList(receivedJsonString);

            priorityRequestSolver_To_TCI_Interface_Socket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), priorityRequestSolver.getCurrentSignalStatusRequestString());
            priorityRequestSolver_To_TCI_Interface_Socket.receiveData(receivedSignalStatusBuffer, sizeof(receivedSignalStatusBuffer));

            std::string receivedSignalStatusString(receivedSignalStatusBuffer);
            // cout << "Received Signal Status" << endl;
            if (priorityRequestSolver.getMessageType(receivedSignalStatusString) == static_cast<int>(msgType::currentPhaseStatus))
                priorityRequestSolver.getCurrentSignalStatus(receivedSignalStatusString);

            tciJsonString = priorityRequestSolver.getScheduleforTCI();
            priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
            priorityRequestSolver.loggingOptimizationData(receivedJsonString, receivedSignalStatusString, tciJsonString);

        }

        else if (msgType == static_cast<int>(msgType::clearRequest))
        {
            // cout << "Received Clear Request " << endl;
            tciJsonString = priorityRequestSolver.getClearCommandScheduleforTCI();
            priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
            // cout << "Sent Clear Request " << endl;
            priorityRequestSolver.loggingClearRequestData(tciJsonString);
        }
    }
}