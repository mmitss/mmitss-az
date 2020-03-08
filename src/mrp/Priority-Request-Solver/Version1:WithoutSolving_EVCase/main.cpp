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
#include <jsoncpp/json/json.h>

int main()
{
    Json::Value jsonObject_config;
    Json::Reader reader;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject_config);

    PriorityRequestSolver priorityRequestSolver;
    UdpSocket priorityRequestSolverSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PrioritySOlver"].asInt()));
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    char receiveBuffer[5120];
    int msgType{};

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

        if (msgType == PRIORITYREQUEST)
        {
            priorityRequestSolver.createPriorityRequestList(receivedJsonString);
            priorityRequestSolver.getCurrentSignalStatus();
            //For EV
            if (priorityRequestSolver.findEVInList() == true)
            {
                priorityRequestSolver.modifyPriorityRequestList();
                priorityRequestSolver.getRequestedSignalGroupFromPriorityRequestList();
                priorityRequestSolver.removeDuplicateSignalGroup();
                priorityRequestSolver.createEventList();
                // priorityRequestSolver.createScheduleJsonString();
                std::cout << "Schedule: " << priorityRequestSolver.createScheduleJsonString() << std::endl;

                // //Single or Multiple EV coming from same direction
                // if (priorityRequestSolver.getRequestedSignalGroupSize() <= 2)
                // {
                //     // priorityRequestSolver.getEVPhases();
                //     // priorityRequestSolver.getEVTrafficSignalPlan();
                //     priorityRequestSolver.createEventList();
                //     priorityRequestSolver.createScheduleJsonString();
                // }
                // //Multiple EV coming from different direction
                // else if (priorityRequestSolver.getNoOfEVInList() > 2 && priorityRequestSolver.getRequestedSignalGroupSize() > 2)
                // {

                //     // priorityRequestSolver.getEVPhases();
                //     // priorityRequestSolver.getEVTrafficSignalPlan();
                //     priorityRequestSolver.generateDatFile();
                //     priorityRequestSolver.generateEVModFile();
                //     priorityRequestSolver.GLPKSolver();
                //     priorityRequestSolver.obtainRequiredSignalGroup();
                //     priorityRequestSolver.readOptimalSignalPlan();
                //     priorityRequestSolver.createEventList();
                //     priorityRequestSolver.createScheduleJsonString();
                // }
            }
            //For Transit or Truck
            else
            {
                priorityRequestSolver.getRequestedSignalGroupFromPriorityRequestList();
                priorityRequestSolver.removeDuplicateSignalGroup();
                priorityRequestSolver.addAssociatedSignalGroup();
                priorityRequestSolver.modifyGreenMax();
                priorityRequestSolver.generateDatFile();
                priorityRequestSolver.GLPKSolver();
                priorityRequestSolver.readOptimalSignalPlan();
                priorityRequestSolver.obtainRequiredSignalGroup();
                priorityRequestSolver.createEventList();
                priorityRequestSolver.createScheduleJsonString();
            }
        }

        else if(msgType == CLEARREQUEST)
        {
            priorityRequestSolver.createClearScheduleJsonString();
        }
    }
}