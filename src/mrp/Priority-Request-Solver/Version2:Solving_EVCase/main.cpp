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
#include <jsoncpp/json/json.h>

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
    UdpSocket priorityRequestSolverSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PrioritySOlver"].asInt()));
    const int trafficControllerPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["TrafficController"].asInt());
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    char receiveBuffer[5120];
    int msgType{};
    string tciJsonString;

    priorityRequestSolver.readCurrentSignalTimingPlan();
    priorityRequestSolver.printSignalPlan();
    priorityRequestSolver.generateModFile();
    // priorityRequestSolver.readOptimalPlan();
    while (true)
    {
        priorityRequestSolverSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);
        cout << "Received Json String " << receivedJsonString << endl;
        msgType = priorityRequestSolver.getMessageType(receivedJsonString);

        if (msgType == static_cast<int>(msgType::priorityRequest))
        {
            priorityRequestSolver.createPriorityRequestList(receivedJsonString);
            priorityRequestSolver.getCurrentSignalStatus();

            if (priorityRequestSolver.findEVInList() == true)
            {
            }

            else
            {
                priorityRequestSolver.GLPKSolver(solverDataManager);
                tciJsonString = priorityRequestSolver.getScheduleforTCI(scheduleManager);
                priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
            }
        }

        else if (msgType == static_cast<int>(msgType::clearRequest))
        {
            priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
        }
    }
}