/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  priority-request-solver-main.cpp
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
    const int signalCoordinationRequestGeneratorPortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["SignalCoordination"].asInt());
    const string LOCALHOST = jsonObject["HostIp"].asString();
    
    char receiveBuffer[40960];
    char receivedSignalStatusBuffer[40960];
    char receivedCoordinationPlanBuffer[40960];
    int msgType{};
    string tciJsonString{};

    priorityRequestSolver.logging();
    priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), priorityRequestSolver.getSignalTimingPlanRequestString());

    while (true)
    {
        priorityRequestSolverSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);
        
        msgType = priorityRequestSolver.getMessageType(receivedJsonString);

        // Received static signal timing plan will be managed and mod file will be written.
        if (msgType == static_cast<int>(msgType::signalPlan))
        {
            cout << "Received Signal Timing Plan " << priorityRequestSolver.getSeconds() << endl;
            priorityRequestSolver.getCurrentSignalTimingPlan(receivedJsonString);
        }

        // Received split data will be used to manage trafficSignalPlan_SignalCoordination vector
        else if (msgType == static_cast<int>(msgType::splitData))
        {
            cout << "Received Split Data for Signal Coordination at time " << priorityRequestSolver.getSeconds() << endl;
            priorityRequestSolver.getSignalCoordinationTimingPlan(receivedJsonString);
        }

        else if (msgType == static_cast<int>(msgType::priorityRequest))
        {
            priorityRequestSolver.createPriorityRequestList(receivedJsonString);
            cout << "Received Priority Request from PRS at time " << priorityRequestSolver.getSeconds() << endl;

            // If split data is not available, split data request message will send to signal-coordination-request-generator.
            // Received split data will be used to manage trafficSignalPlan_SignalCoordination vector
            if (priorityRequestSolver.checkSignalCoordinationTimingPlan() == true)
            {
                priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(signalCoordinationRequestGeneratorPortNo),priorityRequestSolver.getSignalCoordinationTimingPlanRequestString());
                priorityRequestSolverSocket.receiveData(receivedCoordinationPlanBuffer, sizeof(receivedCoordinationPlanBuffer));
                
                std::string receivedCoordinationPlanString(receivedCoordinationPlanBuffer);
                cout << "Received Split Data for Signal Coordination at time " << priorityRequestSolver.getSeconds() << endl;
                
                if (priorityRequestSolver.getMessageType(receivedCoordinationPlanString) == static_cast<int>(msgType::splitData))
                    priorityRequestSolver.getSignalCoordinationTimingPlan(receivedCoordinationPlanString);
            }

            // A JSON formatted message will send to TCI to obtain current signal status
            // A different port number is used for this communication so that PRSolver doesn't miss any messages from the PRS since blocking socket are using here
            priorityRequestSolver_To_TCI_Interface_Socket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), priorityRequestSolver.getCurrentSignalStatusRequestString());
            priorityRequestSolver_To_TCI_Interface_Socket.receiveData(receivedSignalStatusBuffer, sizeof(receivedSignalStatusBuffer));
            
            std::string receivedSignalStatusString(receivedSignalStatusBuffer);
            cout << "Received Signal Status at time " << priorityRequestSolver.getSeconds() << endl;
            
            if (priorityRequestSolver.getMessageType(receivedSignalStatusString) == static_cast<int>(msgType::currentPhaseStatus))
                priorityRequestSolver.getCurrentSignalStatus(receivedSignalStatusString);

            // An optimal signal timing schedule will be formulated based on the priority request list, current signal status and current signal timing plan.
            tciJsonString = priorityRequestSolver.getScheduleforTCI();
            
            // The validated schedule will send to the TCI
            if(priorityRequestSolver.getOptimalSolutionValidationStatus())
                priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
            
            // If requires (logging is "True" in the config file), received priority requests list, dat file, Results.txt file, optimal schedule will be written in the log file.
            priorityRequestSolver.loggingOptimizationData(receivedJsonString, receivedSignalStatusString, tciJsonString);
            
        }

        // If clear request message is received from the PRS, a clear request schedule will be formulated and send to the TCI.
        else if (msgType == static_cast<int>(msgType::clearRequest))
        {
            cout << "Received Clear Request at time " << priorityRequestSolver.getSeconds() << endl;
            tciJsonString = priorityRequestSolver.getClearCommandScheduleforTCI();
            priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
            
            // If requires (logging is "True" in the config file), clear request message and schedule will be written in the log file.
            priorityRequestSolver.loggingClearRequestData(tciJsonString);
        }
    }
}