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
    string tciJsonString{};
    ofstream outputfile;
    ifstream infile;
    bool bLog = priorityRequestSolver.logging();

    if (bLog == true)
    {
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        outputfile.open("/nojournal/bin/log/PRSolver_Log" + std::to_string(timenow) + ".txt");
    }

    priorityRequestSolver.readCurrentSignalTimingPlan();
    priorityRequestSolver.printSignalPlan();
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
                if (bLog == true)
                {
                    auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    
                    outputfile << "\n Current Mod File at time " << timenow << endl;
                    infile.open("NewModel_EV.mod");
                    for (string line; getline(infile, line);)
                    {
                        outputfile << line << endl;
                    }
                    infile.close();

                    outputfile << "\n Current Dat File at time : " << timenow << endl;
                    infile.open("NewModelData.dat");
                    for (string line; getline(infile, line);)
                    {
                        outputfile << line << endl;
                    }
                    infile.close();

                    outputfile << "\n Current Results File at time : " << timenow << endl;
					infile.open("Results.txt");
					for (std::string line; getline(infile, line);)
					{
						outputfile << line << endl;
					}
					infile.close();

                    outputfile << "\n Following Schedule will send to TCI for EV case at time " << timenow << endl;
                    outputfile << tciJsonString << endl;
                }
            }
            else
            {
                priorityRequestSolver.setOptimizationInput();
                priorityRequestSolver.GLPKSolver();
                tciJsonString = priorityRequestSolver.getScheduleforTCI();
                priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
                if (bLog == true)
                {
                    auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                    outputfile << "Following Schedule will send to TCI at time " << timenow << endl;
                    outputfile << tciJsonString << endl;
                }
            }
        }

        else if (msgType == static_cast<int>(msgType::clearRequest))
        {
            priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
            if (bLog == true)
            {
                auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                outputfile << "Following empty Schedule will send to TCI  at time " << timenow << endl;
                outputfile << tciJsonString << endl;
            }
        }
    }

    outputfile.close();
}