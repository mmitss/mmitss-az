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

  priorityRequestSolver.readCurrentSignalTimingPlan();
  // priorityRequestSolver.printSignalPlan();
  priorityRequestSolver.GenerateModFile();
  // priorityRequestSolver.readOptimalPlan();
  while (true)
  {
    priorityRequestSolverSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
    std::string receivedJsonString(receiveBuffer);
    cout << "Received Json String " << receivedJsonString << endl;
    priorityRequestSolver.createPriorityRequestList(receivedJsonString);
    priorityRequestSolver.getCurrentSignalStatus();
    priorityRequestSolver.getRequestedSignalGroupFromPriorityRequestList();
    priorityRequestSolver.removeDuplicateSignalGroup();
    //priorityRequestSolver.printvector();
    priorityRequestSolver.addAssociatedSignalGroup();
    // priorityRequestSolver.printvector();
    priorityRequestSolver.modifyGreenMax();
    priorityRequestSolver.generateDatFile();

    priorityRequestSolver.GLPKSolver();
    priorityRequestSolver.readOptimalSignalPlan();
    priorityRequestSolver.obtainRequiredSignalGroup();
    priorityRequestSolver.createEventList();
    priorityRequestSolver.createScheduleJsonString();
    //cout << "current time " << priorityRequestSolver.GetSeconds() << endl;
  }
}