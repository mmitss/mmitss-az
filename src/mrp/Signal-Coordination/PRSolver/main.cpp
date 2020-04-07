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
    UdpSocket priorityRequestSolverSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PrioritySolver"].asInt()));
    const int trafficControllerPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["TrafficControllerInterface"].asInt());
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    char receiveBuffer[5120];
    int msgType{};
    string tciJsonString{};
 
    priorityRequestSolver.logging();

    priorityRequestSolver.readCurrentSignalTimingPlan();
    // priorityRequestSolver.printSignalPlan();
    priorityRequestSolver.generateModFile();
    
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
                priorityRequestSolver.setOptimizationInput();
                priorityRequestSolver.GLPKSolver();
                tciJsonString = priorityRequestSolver.getScheduleforTCI();
                priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
                priorityRequestSolver.loggingData(tciJsonString);
            }
            else
            {
                priorityRequestSolver.setOptimizationInput();
                priorityRequestSolver.GLPKSolver();
                tciJsonString = priorityRequestSolver.getScheduleforTCI();
                priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
                priorityRequestSolver.loggingData(tciJsonString);
            }
        }

        else if (msgType == static_cast<int>(msgType::clearRequest))
        {
            tciJsonString = priorityRequestSolver.getClearCommandScheduleforTCI();
            priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
            priorityRequestSolver.loggingData(tciJsonString);
        }
    }
}