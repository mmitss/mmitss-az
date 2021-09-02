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
#include "PriorityRequestSolver.h"
#include "SolverDataManager.h"

int main()
{
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
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
    const int timePhaseDiagramToolPortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["TimePhaseDiagramTool"].asInt());

    const string LOCALHOST = jsonObject["HostIp"].asString();

    char receiveBuffer[40960];
    char receivedSignalStatusBuffer[40960];
    char receivedCoordinationPlanBuffer[40960];
    int msgType{};
    string tciJsonString{};
    string timePhaseDiagramJsonString{};

    priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), priorityRequestSolver.getSignalTimingPlanRequestString());
    // priorityRequestSolver.terminateProgram();

    while (true)
    {
        priorityRequestSolverSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);

        msgType = priorityRequestSolver.getMessageType(receivedJsonString);

        /*
            - If received message contains static signal timing plan, it will be managed and mod file will be written.
            - If received message contains split data, it will be used to manage trafficSignalPlan_SignalCoordination vector.
            - If received message contains priority requests, it will be used to manage the priority requests list.
            - If clear request message is received from the PRS, a clear request schedule will be formulated and send to the TCI.
        */
        if (msgType == static_cast<int>(msgType::signalPlan))
            priorityRequestSolver.setCurrentSignalTimingPlan(receivedJsonString);

        else if (msgType == static_cast<int>(msgType::splitData))
            priorityRequestSolver.setSignalCoordinationTimingPlan(receivedJsonString);

        else if (msgType == static_cast<int>(msgType::priorityRequest))
        {
            if (priorityRequestSolver.checkTrafficSignalTimingPlanStatus())
            {
                priorityRequestSolver.createPriorityRequestList(receivedJsonString);
                /*
                    - If split data is not available, split data request message will send to signal-coordination-request-generator.
                    - Received split data will be used to manage trafficSignalPlan_SignalCoordination vector
                */
                if (priorityRequestSolver.checkSignalCoordinationTimingPlanStatus())
                {
                    priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(signalCoordinationRequestGeneratorPortNo), priorityRequestSolver.getSignalCoordinationTimingPlanRequestString());
                    priorityRequestSolverSocket.receiveData(receivedCoordinationPlanBuffer, sizeof(receivedCoordinationPlanBuffer));
                    std::string receivedCoordinationPlanString(receivedCoordinationPlanBuffer);

                    if (priorityRequestSolver.getMessageType(receivedCoordinationPlanString) == static_cast<int>(msgType::splitData))
                        priorityRequestSolver.setSignalCoordinationTimingPlan(receivedCoordinationPlanString);
                }

                /*
                    - A JSON formatted message will send to TCI to obtain current signal status
                    - A different port number is used for this communication so that PRSolver doesn't miss any messages from the PRS since blocking socket are using here
                */
                priorityRequestSolver_To_TCI_Interface_Socket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), priorityRequestSolver.getCurrentSignalStatusRequestString());
                priorityRequestSolver_To_TCI_Interface_Socket.receiveData(receivedSignalStatusBuffer, sizeof(receivedSignalStatusBuffer));
                std::string receivedSignalStatusString(receivedSignalStatusBuffer);

                if (priorityRequestSolver.getMessageType(receivedSignalStatusString) == static_cast<int>(msgType::currentPhaseStatus))
                    priorityRequestSolver.getCurrentSignalStatus(receivedSignalStatusString);

                /*
                    - An optimization problem will be solved based on the priority request list, current signal status and current signal timing plan.
                    - An optimal signal timing schedule will be formulated based on optimal solution.
                    - The validated schedule will send to the TCI.
                */
                tciJsonString = priorityRequestSolver.getScheduleforTCI();
                if (priorityRequestSolver.getOptimalSolutionValidationStatus())
                    priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
                    
                timePhaseDiagramJsonString = priorityRequestSolver.getTimePhaseDiagramMessageString();
                priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(timePhaseDiagramToolPortNo), timePhaseDiagramJsonString);
                

                //If requires, check for the priority weights update from the config file
                if (priorityRequestSolver.checkUpdatesForPriorityWeights())
                    priorityRequestSolver.getPriorityWeights();
            }
            // If traffic signal timing plan is not available, a signal timing plan request will be sent
            else
                priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), priorityRequestSolver.getSignalTimingPlanRequestString());
        }

        else if (msgType == static_cast<int>(msgType::clearRequest))
        {
            tciJsonString = priorityRequestSolver.getClearCommandScheduleforTCI();
            priorityRequestSolverSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trafficControllerPortNo), tciJsonString);
        }
    }

    priorityRequestSolverSocket.closeSocket();
    priorityRequestSolver_To_TCI_Interface_Socket.closeSocket();

    return 0;
}
