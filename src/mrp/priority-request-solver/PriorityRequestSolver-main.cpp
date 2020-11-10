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
  1. An API for PriorityRequestSolver  
*/

#include <UdpSocket.h>
#include <fstream>
#include "PriorityRequestSolver.h"
#include "SolverDataManager.h"
#include <json/json.h>

int main()
{
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);        
    delete reader;

    PriorityRequestSolver priorityRequestSolver;
    ScheduleManager scheduleManager;
    SolverDataManager solverDataManager;
    UdpSocket priorityRequestSolverSocket(static_cast<short unsigned int>(jsonObject["PortNumber"]["PrioritySolver"].asInt()));
    UdpSocket priorityRequestSolver_To_TCI_Interface_Socket(static_cast<short unsigned int>(jsonObject["PortNumber"]["PrioritySolverToTCIInterface"].asInt()));
    const int trafficControllerPortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["TrafficControllerInterface"].asInt());
    const string LOCALHOST = jsonObject["HostIp"].asString();
    char receiveBuffer[40960];
    char receivedSignalStatusBuffer[40960];
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
            cout << "Received Signal Timing Plan " << endl;
            priorityRequestSolver.getCurrentSignalTimingPlan(receivedJsonString);
        }

        else if (msgType == static_cast<int>(msgType::splitData))
        {
            cout << "Received Split Data for Signal Coordination " << endl;
            priorityRequestSolver.getSignalCoordinationTimingPlan(receivedJsonString);
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
            if(priorityRequestSolver.getOptimalSolutionValidationStatus() == true)
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