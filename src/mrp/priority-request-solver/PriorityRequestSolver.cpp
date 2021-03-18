/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorityRequestSolver.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script contains method to manage the priority requests in a list.
  2. This script manage the current signal timing plan and current signal status required for solving the optimization model.
*/

#include "PriorityRequestSolver.h"
#include "glpk.h"
#include <stdio.h>
#include <sys/time.h>
#include "json/json.h"
#include <algorithm>
#include <cmath>
#include <UdpSocket.h>
#include "msgEnum.h"

PriorityRequestSolver::PriorityRequestSolver()
{
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
    delete reader;

    EmergencyVehicleWeight = jsonObject["PriorityParameter"]["EmergencyVehicleWeight"].asDouble();
    EmergencyVehicleSplitPhaseWeight = jsonObject["PriorityParameter"]["EmergencyVehicleSplitPhaseWeight"].asDouble();
    TransitWeight = jsonObject["PriorityParameter"]["TransitWeight"].asDouble();
    TruckWeight = jsonObject["PriorityParameter"]["TruckWeight"].asDouble();
    DilemmaZoneRequestWeight = jsonObject["PriorityParameter"]["DilemmaZoneRequestWeight"].asDouble();
    CoordinationWeight = jsonObject["PriorityParameter"]["CoordinationWeight"].asDouble();
}

/*
	- Method for identifying the message type.
*/
int PriorityRequestSolver::getMessageType(string jsonString)
{
    int messageType{};
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    if (parsingSuccessful == true)
    {
        if ((jsonObject["MsgType"]).asString() == "PriorityRequest")
            messageType = static_cast<int>(msgType::priorityRequest);

        else if ((jsonObject["MsgType"]).asString() == "ClearRequest")
            messageType = static_cast<int>(msgType::clearRequest);

        else if ((jsonObject["MsgType"]).asString() == "ActiveTimingPlan")
            messageType = static_cast<int>(msgType::signalPlan);

        else if ((jsonObject["MsgType"]).asString() == "CurrNextPhaseStatus")
            messageType = static_cast<int>(msgType::currentPhaseStatus);

        else if ((jsonObject["MsgType"]).asString() == "ActiveCoordinationPlan")
            messageType = static_cast<int>(msgType::splitData);

        else
            std::cout << "Message type is unknown" << std::endl;
    }

    return messageType;
}

/*
    - This method is responsible for creating priority request list received from the PRS as Json String.
*/
void PriorityRequestSolver::createPriorityRequestList(string jsonString)
{
    RequestList requestList;
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    priorityRequestList.clear();
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;
    
    cout << "[" << currentTime << "] Received Priority Request List from PRS"<< endl;

    int noOfRequest = (jsonObject["PriorityRequestList"]["noOfRequest"]).asInt();

    for (int i = 0; i < noOfRequest; i++)
    {
        requestList.reset();
        requestList.vehicleID = jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleID"].asInt();
        requestList.vehicleType = jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleType"].asInt();
        requestList.basicVehicleRole = jsonObject["PriorityRequestList"]["requestorInfo"][i]["basicVehicleRole"].asInt();
        requestList.laneID = jsonObject["PriorityRequestList"]["requestorInfo"][i]["inBoundLaneID"].asInt();
        requestList.vehicleETA = jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"].asDouble();
        requestList.vehicleETA_Duration = jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"].asDouble();
        requestList.requestedPhase = jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"].asInt();
        requestList.prioritystatus = jsonObject["PriorityRequestList"]["requestorInfo"][i]["priorityRequestStatus"].asInt();
        requestList.vehicleLatitude = jsonObject["PriorityRequestList"]["requestorInfo"][i]["latitude_DecimalDegree"].asDouble();
        requestList.vehicleLongitude = jsonObject["PriorityRequestList"]["requestorInfo"][i]["longitude_DecimalDegree"].asDouble();
        requestList.vehicleElevation = jsonObject["PriorityRequestList"]["requestorInfo"][i]["elevation_Meter"].asDouble();
        requestList.vehicleHeading = jsonObject["PriorityRequestList"]["requestorInfo"][i]["heading_Degree"].asDouble();
        requestList.vehicleSpeed = jsonObject["PriorityRequestList"]["requestorInfo"][i]["speed_MeterPerSecond"].asDouble();
        requestList.vehicleDistanceFromStopBar = requestList.vehicleSpeed * 3.28084 * requestList.vehicleETA;
        priorityRequestList.push_back(requestList);
    }
}

/*
    - Obtain Steady State Vehicle Deceleration Value in ft
*/
double PriorityRequestSolver::getCoefficientOfFrictionValue(double vehicleSpeed)
{
    double coefficientOfFrictionValue{};

    if (vehicleSpeed >= 20.0 && vehicleSpeed < 25.0)
        coefficientOfFrictionValue = 0.40;

    else if (vehicleSpeed >= 25.0 && vehicleSpeed < 30.0)
        coefficientOfFrictionValue = 0.38;

    else if (vehicleSpeed >= 30.0 && vehicleSpeed < 35.0)
        coefficientOfFrictionValue = 0.35;

    else if (vehicleSpeed >= 35.0 && vehicleSpeed < 40.0)
        coefficientOfFrictionValue = 0.34;

    else if (vehicleSpeed >= 40.0 && vehicleSpeed < 45.0)
        coefficientOfFrictionValue = 0.32;

    else if (vehicleSpeed >= 45.0 && vehicleSpeed < 50.0)
        coefficientOfFrictionValue = 0.31;

    else if (vehicleSpeed >= 50.0 && vehicleSpeed < 55.0)
        coefficientOfFrictionValue = 0.30;

    else if (vehicleSpeed >= 55.0 && vehicleSpeed < 60.0)
        coefficientOfFrictionValue = 0.30;

    else if (vehicleSpeed >= 60.0 && vehicleSpeed < 65.0)
        coefficientOfFrictionValue = 0.29;

    else if (vehicleSpeed >= 65.0 && vehicleSpeed < 70.0)
        coefficientOfFrictionValue = 0.29;

    else if (vehicleSpeed >= 70.0 && vehicleSpeed < 75.0)
        coefficientOfFrictionValue = 0.28;

    return coefficientOfFrictionValue;
}
/*
    - The following method responsible for checking whether any heavy vehicle is in the dilemma zoe or not.
*/
void PriorityRequestSolver::createDilemmaZoneRequestList()
{
    double initialVehicleSpeed{};
    double stoppingSightDistance{};
    double perceptionResponseTime = 2.5;
    double coefficientOfFriction{};

    dilemmaZoneRequestList.clear();

    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        if (priorityRequestList[i].vehicleType == 9 && (priorityRequestList[i].requestedPhase == trafficControllerStatus[0].startingPhase1 || priorityRequestList[i].requestedPhase == trafficControllerStatus[0].startingPhase2))
        {
            initialVehicleSpeed = priorityRequestList[i].vehicleSpeed * 2.23694;
            coefficientOfFriction = getCoefficientOfFrictionValue(initialVehicleSpeed);
            stoppingSightDistance = 1.47 * perceptionResponseTime * initialVehicleSpeed + std::pow(initialVehicleSpeed, 2) / (30 * coefficientOfFriction);
            if (priorityRequestList[i].vehicleDistanceFromStopBar <= stoppingSightDistance)

                dilemmaZoneRequestList.push_back(priorityRequestList[i]);
        }
    }
}

/*
    - If there is EV priority request in the list, following method will delete all the priority request from the list apart from EV request.
    - Check vehicle type for all the received request.
    - If vehicle type is not EV(For EV vehicleType is 2) remove that request from the list.
*/
void PriorityRequestSolver::modifyPriorityRequestList()
{
    int temporaryVehicleID{};

    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        temporaryVehicleID = priorityRequestList[i].vehicleID;
        if (priorityRequestList[i].vehicleType != 2)
        {
            vector<RequestList>::iterator findVehicleIDOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                             [&](RequestList const &p) { return p.vehicleID == temporaryVehicleID; });

            priorityRequestList.erase(findVehicleIDOnList);
            i--;
        }
        else
            noOfEVInList++;
    }
}

/*
    - This method check whether it is required to delete the split phase priority request.
    - For Single or Multiple EV priority request for same signal group:
        - This method appends starting phases into their respective ring barrier group
        - If ring barrier group is missing, then the method:
            - check whether starting phases left turn phase are same or not
            - if starting phases left turn phase are not same, remove left turn phases
            - add dummy through phases to the missing ring barrier group. suppose Starting Phase is {4,8} and requested Phase is {4,7}. Then split phase{7} is deleted by the method and two Dummy phase {2,6} will be added to formulate the problem properly. 
    - For Multiple EV priority request from different approach:
        - If all the requested signal group are in same Barrier grup, delete all the left turn priority request from the list.
        - If ring barrier group is missing, then the method adds dummy through phases to the missing ring barrier group.
*/
void PriorityRequestSolver::deleteSplitPhasesFromPriorityRequestList()
{
    int temporaryPhase{};
    int tempSignalGroup{};
    vector<int>::iterator it;
    vector<int> requestedEV_P11{};
    vector<int> requestedEV_P12{};
    vector<int> requestedEV_P21{};
    vector<int> requestedEV_P22{};

    //Append the requested signal group into their corresponding ring barrier vector.
    for (size_t i = 0; i < requestedSignalGroup.size(); i++)
    {
        if (requestedSignalGroup[i] == 1 || requestedSignalGroup[i] == 2)
            requestedEV_P11.push_back(requestedSignalGroup[i]);

        else if (requestedSignalGroup[i] == 3 || requestedSignalGroup[i] == 4)
            requestedEV_P12.push_back(requestedSignalGroup[i]);

        else if (requestedSignalGroup[i] == 5 || requestedSignalGroup[i] == 6)
            requestedEV_P21.push_back(requestedSignalGroup[i]);

        else if (requestedSignalGroup[i] == 7 || requestedSignalGroup[i] == 8)
            requestedEV_P22.push_back(requestedSignalGroup[i]);
    }
    /*
        - If a freight vehicle is trapped in the dilemma zone, the split phase priority request may require to remove (depends on the starting phase).
        - The split phase priority request which is associated with the starting phase will be removed from the priority request list.
        - For example, if starting phase is phase 2, the split phase priority request for phase 5 will be removed.
    */
    if (!dilemmaZoneRequestList.empty())
    {
        for (size_t i = 0; i < requestedSignalGroup.size(); i++)
        {
            if (requestedSignalGroup.at(i) == trafficControllerStatus[0].startingPhase1)
            {
                if (trafficControllerStatus[0].startingPhase1 == 2)
                    temporaryPhase = 5;

                else if (trafficControllerStatus[0].startingPhase1 == 4)
                    temporaryPhase = 7;

                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j = j - 1;
                    }
                }
                requestedSignalGroup.clear();
                getRequestedSignalGroup();
            }

            else if (requestedSignalGroup.at(i) == trafficControllerStatus[0].startingPhase2)
            {
                if (trafficControllerStatus[0].startingPhase2 == 6)
                    temporaryPhase = 1;

                else if (trafficControllerStatus[0].startingPhase2 == 8)
                    temporaryPhase = 3;

                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j = j - 1;
                    }
                }
                requestedSignalGroup.clear();
                getRequestedSignalGroup();
            }
        }
    }
    // If there is an EV priority request and number of requested signal phase is not more than 2, append the starting phases into their corresponding ring barrier vector.
    if (noOfEVInList > 0 && requestedSignalGroup.size() <= 2)
    {
        for (size_t j = 0; j < trafficControllerStatus.size(); j++)
        {
            if ((trafficControllerStatus[j].startingPhase1 == 1) || (trafficControllerStatus[j].startingPhase1 == 2))
                requestedEV_P11.push_back(trafficControllerStatus[j].startingPhase1);

            else if ((trafficControllerStatus[j].startingPhase1 == 3) || (trafficControllerStatus[j].startingPhase1 == 4))
                requestedEV_P12.push_back(trafficControllerStatus[j].startingPhase1);

            else if ((trafficControllerStatus[j].startingPhase1 == 5) || (trafficControllerStatus[j].startingPhase1 == 6))
                requestedEV_P21.push_back(trafficControllerStatus[j].startingPhase1);

            else if ((trafficControllerStatus[j].startingPhase1 == 7) || (trafficControllerStatus[j].startingPhase1 == 8))
                requestedEV_P22.push_back(trafficControllerStatus[j].startingPhase1);

            if ((trafficControllerStatus[j].startingPhase2 == 1) || (trafficControllerStatus[j].startingPhase2 == 2))
                requestedEV_P11.push_back(trafficControllerStatus[j].startingPhase2);

            else if ((trafficControllerStatus[j].startingPhase2 == 3) || (trafficControllerStatus[j].startingPhase2 == 4))
                requestedEV_P12.push_back(trafficControllerStatus[j].startingPhase2);

            else if ((trafficControllerStatus[j].startingPhase2 == 5) || (trafficControllerStatus[j].startingPhase2 == 6))
                requestedEV_P21.push_back(trafficControllerStatus[j].startingPhase2);

            else if ((trafficControllerStatus[j].startingPhase2 == 7) || (trafficControllerStatus[j].startingPhase2 == 8))
                requestedEV_P22.push_back(trafficControllerStatus[j].startingPhase2);
        }
        //If first ring-barrier group is empty, the left turn phases (3,7) of second ring-barrier group can be removed (if they are not starting phase). Dummy through phases (2,6) will be inserted in the first ring-barrier group.
        if (requestedEV_P11.empty() && requestedEV_P21.empty())
        {
            if ((trafficControllerStatus[0].startingPhase1 != 3))
            {
                temporaryPhase = 3;
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j--;
                    }
                }
            }

            if ((trafficControllerStatus[0].startingPhase2 != 7))
            {
                temporaryPhase = 7;
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j--;
                    }
                }
            }
            requestedSignalGroup.clear();
            getRequestedSignalGroup();

            vector<int> dummyPhases{2, 6};
            for (size_t i = 0; i < dummyPhases.size(); i++)
            {
                tempSignalGroup = dummyPhases[i];
                it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), tempSignalGroup);
                if (it == requestedSignalGroup.end())
                    requestedSignalGroup.push_back(tempSignalGroup);
            }
        }
        //If second ring-barrier group is empty, the left turn phases (1,5) of first ring-barrier groupcan be removed (if they are not starting phase). Dummy through phases (4,8) will be inserted in the second ring-barrier group.
        else if (requestedEV_P12.empty() && requestedEV_P22.empty())
        {
            if ((trafficControllerStatus[0].startingPhase1 != 1))
            {
                temporaryPhase = 1;
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j--;
                    }
                }
            }

            if ((trafficControllerStatus[0].startingPhase2 != 5))
            {
                temporaryPhase = 5;
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j--;
                    }
                }
            }
            requestedSignalGroup.clear();
            getRequestedSignalGroup();
            vector<int> dummyPhases{4, 8};
            for (size_t i = 0; i < dummyPhases.size(); i++)
            {
                tempSignalGroup = dummyPhases[i];
                it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), tempSignalGroup);
                if (it == requestedSignalGroup.end())
                    requestedSignalGroup.push_back(tempSignalGroup);
            }
        }
    }
    // If there is an EV priority request and number of requested signal phase is more than 2, the split phase priority request can be removed (if opposite barrier group is empty)).
    else if (noOfEVInList > 0 && requestedSignalGroup.size() > 2)
    {
        if (requestedEV_P11.empty() && requestedEV_P21.empty())
        {
            vector<int> LeftTurnPhases{3, 7};
            for (size_t i = 0; i < LeftTurnPhases.size(); i++)
            {
                temporaryPhase = LeftTurnPhases.at(i);
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j--;
                    }
                }
            }
            requestedSignalGroup.clear();
            getRequestedSignalGroup();
            vector<int> dummyPhases{2, 6};
            for (size_t i = 0; i < dummyPhases.size(); i++)
            {
                tempSignalGroup = dummyPhases[i];
                it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), tempSignalGroup);
                if (it == requestedSignalGroup.end())
                    requestedSignalGroup.push_back(tempSignalGroup);
            }
        }

        else if (requestedEV_P12.empty() && requestedEV_P22.empty())
        {
            vector<int> LeftTurnPhases{1, 5};
            for (size_t i = 0; i < LeftTurnPhases.size(); i++)
            {
                temporaryPhase = LeftTurnPhases.at(i);
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j--;
                    }
                }
            }
            requestedSignalGroup.clear();
            getRequestedSignalGroup();
            vector<int> dummyPhases{4, 8};
            for (size_t i = 0; i < dummyPhases.size(); i++)
            {
                tempSignalGroup = dummyPhases[i];
                it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), tempSignalGroup);
                if (it == requestedSignalGroup.end())
                    requestedSignalGroup.push_back(tempSignalGroup);
            }
        }
        // For T-intersection when EV are approaching from major and minor street simulatenously, Following logic is required.
        // If minor street is already green then delete the split phase information.
        else if (trafficControllerStatus[0].startingPhase1 == 4 && trafficControllerStatus[0].startingPhase2 == 8)
        {
            vector<int> LeftTurnPhases{3, 7};
            for (size_t i = 0; i < LeftTurnPhases.size(); i++)
            {
                temporaryPhase = LeftTurnPhases.at(i);
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j--;
                    }
                }
            }
            requestedSignalGroup.clear();
            getRequestedSignalGroup();
        }

        else if (trafficControllerStatus[0].startingPhase1 == 2 && trafficControllerStatus[0].startingPhase2 == 6)
        {
            vector<int> LeftTurnPhases{1, 5};
            for (size_t i = 0; i < LeftTurnPhases.size(); i++)
            {
                temporaryPhase = LeftTurnPhases.at(i);
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p) { return p.requestedPhase == temporaryPhase; });

                    if (findSignalGroupOnList != priorityRequestList.end())
                    {
                        priorityRequestList.erase(findSignalGroupOnList);
                        j--;
                    }
                }
            }
            requestedSignalGroup.clear();
            getRequestedSignalGroup();
        }
    }
}

/*
    - This method manage all the required input data for the GLPK solver.
    - This function also calls SolverDataManager class to write the dat dile
*/
void PriorityRequestSolver::setOptimizationInput()
{
    if (emergencyVehicleStatus == true)
    {
        OptimizationModelManager optimizationModelManager;

        createDilemmaZoneRequestList();
        modifyPriorityRequestList();
        getRequestedSignalGroup();
        deleteSplitPhasesFromPriorityRequestList();
        getEVPhases();
        getEVTrafficSignalPlan();
        optimizationModelManager.generateEVModFile(trafficSignalPlan_EV, EV_P11, EV_P12, EV_P21, EV_P22);
        SolverDataManager solverDataManager(dilemmaZoneRequestList, priorityRequestList, trafficControllerStatus, trafficSignalPlan_EV, EmergencyVehicleWeight, EmergencyVehicleSplitPhaseWeight, TransitWeight, TruckWeight, DilemmaZoneRequestWeight, CoordinationWeight);
        solverDataManager.generateDatFile(emergencyVehicleStatus);
    }

    else if (signalCoordinationRequestStatus == true)
    {
        SolverDataManager solverDataManager(dilemmaZoneRequestList, priorityRequestList, trafficControllerStatus, trafficSignalPlan_SignalCoordination, EmergencyVehicleWeight, EmergencyVehicleSplitPhaseWeight, TransitWeight, TruckWeight, DilemmaZoneRequestWeight, CoordinationWeight);
        solverDataManager.getRequestedSignalGroupFromPriorityRequestList();
        solverDataManager.addAssociatedSignalGroup();
        solverDataManager.modifyGreenMax();
        solverDataManager.generateDatFile(emergencyVehicleStatus);
    }

    else
    {
        SolverDataManager solverDataManager(dilemmaZoneRequestList, priorityRequestList, trafficControllerStatus, trafficSignalPlan, EmergencyVehicleWeight, EmergencyVehicleSplitPhaseWeight, TransitWeight, TruckWeight, DilemmaZoneRequestWeight, CoordinationWeight);
        solverDataManager.getRequestedSignalGroupFromPriorityRequestList();
        solverDataManager.addAssociatedSignalGroup();
        solverDataManager.modifyGreenMax();
        solverDataManager.generateDatFile(emergencyVehicleStatus);
    }
}

/*
    - This method is getters for obtaining all the requested phase form the priority request list.
*/
void PriorityRequestSolver::getRequestedSignalGroup()
{
    SolverDataManager solverDataManager(priorityRequestList);
    requestedSignalGroup.clear();
    requestedSignalGroup = solverDataManager.getRequestedSignalGroupFromPriorityRequestList();
}

/*
    - Method of solving the request in the priority request list  based on mod and dat files
    - Solution will be written in the /nojournal/bin/Results.txt file
*/
void PriorityRequestSolver::GLPKSolver()
{
    double startOfSolve{};
    double endOfSolve{};

    startOfSolve = getSeconds();

    char modFile[128] = "/nojournal/bin/OptimizationModel.mod";
    glp_prob *mip;
    glp_tran *tran;
    int ret{};
    int success = 1;
    if (emergencyVehicleStatus == true)
        strcpy(modFile, "/nojournal/bin/OptimizationModel_EV.mod");

    mip = glp_create_prob();
    tran = glp_mpl_alloc_wksp();

    ret = glp_mpl_read_model(tran, modFile, 1);

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating model\n");
        goto skip;
    }

    ret = glp_mpl_read_data(tran, "/nojournal/bin/OptimizationModelData.dat");

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating data\n");
        goto skip;
    }

    ret = glp_mpl_generate(tran, NULL);
    if (ret != 0)
    {
        fprintf(stderr, "Error on generating model\n");
        goto skip;
    }

    glp_mpl_build_prob(tran, mip);
    glp_simplex(mip, NULL);
    success = glp_intopt(mip, NULL);
    endOfSolve = getSeconds();
    cout << "Success=" << success << endl;
    cout << "Time of Solve " << endOfSolve - startOfSolve << endl;
    ret = glp_mpl_postsolve(tran, mip, GLP_MIP);
    if (ret != 0)
        fprintf(stderr, "Error on postsolving model\n");
skip:
    glp_mpl_free_wksp(tran);
    glp_delete_prob(mip);
}

/*
    - This method calls finEVInList() function to check whether emergency vehicle is in the list or not
    - This method also calls setOptimizationInput() to process all the input data for the GLPK solver and GLPKSolver() method to solve the optimization problem.
    - This function finally calls ScheduleManage class to process the schedule for the TCI in JSON string format.
*/
string PriorityRequestSolver::getScheduleforTCI()
{
    string scheduleJsonString{};

    findEVInList();
    findCoordinationRequestInList();
    setOptimizationInput();
    GLPKSolver();

    if (emergencyVehicleStatus == true)
    {
        ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan_EV, emergencyVehicleStatus);

        scheduleManager.obtainRequiredSignalGroup();
        scheduleManager.readOptimalSignalPlan();
        scheduleManager.createEventList();
        optimalSolutionStatus = scheduleManager.validateOptimalSolution();
        scheduleJsonString = scheduleManager.createScheduleJsonString();
    }

    else if (signalCoordinationRequestStatus == true)
    {
        ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan_SignalCoordination, emergencyVehicleStatus);
        scheduleManager.obtainRequiredSignalGroup();
        scheduleManager.readOptimalSignalPlan();
        scheduleManager.createEventList();
        optimalSolutionStatus = scheduleManager.validateOptimalSolution();
        scheduleJsonString = scheduleManager.createScheduleJsonString();
    }

    else
    {
        ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan, emergencyVehicleStatus);
        scheduleManager.obtainRequiredSignalGroup();
        scheduleManager.readOptimalSignalPlan();
        scheduleManager.createEventList();
        optimalSolutionStatus = scheduleManager.validateOptimalSolution();
        scheduleJsonString = scheduleManager.createScheduleJsonString();
    }

    priorityRequestList.clear();
    dilemmaZoneRequestList.clear();
    trafficControllerStatus.clear();

    return scheduleJsonString;
}

/*
    - This method is reponsible for creating a JSON formatted string to clear the schedule for TCI.
*/
string PriorityRequestSolver::getClearCommandScheduleforTCI()
{
    string clearScheduleJsonString{};
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    ScheduleManager scheduleManager;
    
    cout << "[" << currentTime << "] Received Clear Request"<< endl;

    clearScheduleJsonString = scheduleManager.createScheduleJsonString();

    return clearScheduleJsonString;
}

/*
    - If here is EV in priority request list, Dat file will have information about EV requested phases and current phases.
    - A vector called plannedEVPhases will hold the required signal group for EV priority.
*/
void PriorityRequestSolver::getEVPhases()
{
    int tempSignalGroup{};
    vector<int>::iterator it;
    plannedEVPhases.clear();
    plannedEVPhases = requestedSignalGroup;
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        tempSignalGroup = trafficControllerStatus[i].startingPhase1;
        it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), tempSignalGroup);
        if (it == requestedSignalGroup.end())
            plannedEVPhases.push_back(tempSignalGroup);

        tempSignalGroup = trafficControllerStatus[i].startingPhase2;
        it = std::find(requestedSignalGroup.begin(), requestedSignalGroup.end(), tempSignalGroup);
        if (it == requestedSignalGroup.end())
            plannedEVPhases.push_back(tempSignalGroup);
    }

    sort(plannedEVPhases.begin(), plannedEVPhases.end());
}

/*
    - This method is responsible for obtaining dynamic EV Traffic Signal Plan
    - EV Traffic Signal Plan doesn't contain the ommit phases information.
*/
void PriorityRequestSolver::getEVTrafficSignalPlan()
{
    int temporaryPhase{};
    vector<int> temporaryPhaseNumber{};
    temporaryPhaseNumber = PhaseNumber;
    vector<int>::iterator it;
    EV_P11.clear();
    EV_P12.clear();
    EV_P21.clear();
    EV_P22.clear();
    trafficSignalPlan_EV.clear();
    trafficSignalPlan_EV.insert(trafficSignalPlan_EV.end(), trafficSignalPlan.begin(), trafficSignalPlan.end());

    for (size_t j = 0; j < plannedEVPhases.size(); j++)
    {
        temporaryPhase = plannedEVPhases.at(j);
        it = std::find(temporaryPhaseNumber.begin(), temporaryPhaseNumber.end(), temporaryPhase);
        
        if (it != temporaryPhaseNumber.end())
            temporaryPhaseNumber.erase(it);

    }

    for (size_t i = 0; i < temporaryPhaseNumber.size(); i++)
    {
        temporaryPhase = temporaryPhaseNumber.at(i);

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan_EV), std::end(trafficSignalPlan_EV),
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

        if (findSignalGroupOnList != trafficSignalPlan_EV.end())
            trafficSignalPlan_EV.erase(findSignalGroupOnList);
    }

    //Obtain the phases in P11, P12, P21, P22
    for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
    {
        temporaryPhase = trafficSignalPlan_EV[i].phaseNumber;
        
        if (trafficSignalPlan_EV[i].phaseNumber < 3 && trafficSignalPlan_EV[i].phaseRing == 1)
            EV_P11.push_back(trafficSignalPlan_EV[i].phaseNumber);

        else if (trafficSignalPlan_EV[i].phaseNumber > 2 && trafficSignalPlan_EV[i].phaseNumber < 5 && trafficSignalPlan_EV[i].phaseRing == 1)
            EV_P12.push_back(trafficSignalPlan_EV[i].phaseNumber);

        else if (trafficSignalPlan_EV[i].phaseNumber > 4 && trafficSignalPlan_EV[i].phaseNumber < 7 && trafficSignalPlan_EV[i].phaseRing == 2)
            EV_P21.push_back(trafficSignalPlan_EV[i].phaseNumber);

        else if (trafficSignalPlan_EV[i].phaseNumber > 6 && trafficSignalPlan_EV[i].phaseRing == 2)
            EV_P22.push_back(trafficSignalPlan_EV[i].phaseNumber);
    }

    validateEVTrafficSignalPlan();
}

/*
    - The following method will fill the missing data in EVTrafficSignalPlan.
    - For example if phase 5 data is missing then fill up the information with phase 2 data
*/
void PriorityRequestSolver::validateEVTrafficSignalPlan()
{
    int temporarySignalGroup{};
    int associatedSignalGroup{};
    for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
    {
        temporarySignalGroup = trafficSignalPlan_EV[i].phaseNumber;
        if (temporarySignalGroup % 2 == 0 && trafficSignalPlan_EV[i].minGreen == 0 && temporarySignalGroup < 5)
            associatedSignalGroup = temporarySignalGroup + 4;

        else if (temporarySignalGroup % 2 == 0 && trafficSignalPlan_EV[i].minGreen == 0 && temporarySignalGroup > 4)
            associatedSignalGroup = temporarySignalGroup - 4;

        else if (temporarySignalGroup % 2 != 0 && trafficSignalPlan_EV[i].minGreen == 0 && temporarySignalGroup < 5)
            associatedSignalGroup = temporarySignalGroup + 5;

        else if (temporarySignalGroup % 2 != 0 && trafficSignalPlan_EV[i].minGreen == 0 && temporarySignalGroup > 4)
            associatedSignalGroup = temporarySignalGroup - 3;

        else
            continue;

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findAssociatedSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan_EV), std::end(trafficSignalPlan_EV),
                                                                                                                  [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == associatedSignalGroup; });

        if (findAssociatedSignalGroupOnList != trafficSignalPlan_EV.end())
        {
            trafficSignalPlan_EV[i].pedWalk = findAssociatedSignalGroupOnList->pedWalk;
            trafficSignalPlan_EV[i].pedClear = findAssociatedSignalGroupOnList->pedClear;
            trafficSignalPlan_EV[i].minGreen = findAssociatedSignalGroupOnList->minGreen;
            trafficSignalPlan_EV[i].passage = findAssociatedSignalGroupOnList->passage;
            trafficSignalPlan_EV[i].maxGreen = findAssociatedSignalGroupOnList->maxGreen;
            trafficSignalPlan_EV[i].yellowChange = findAssociatedSignalGroupOnList->yellowChange;
            trafficSignalPlan_EV[i].redClear = findAssociatedSignalGroupOnList->redClear;
        }
    }
}

/*
    - To get the current phase status a request message has to send to the TCI
    - This method is responsible for creating that message as a JSON string. 
*/
string PriorityRequestSolver::getCurrentSignalStatusRequestString()
{
    string currentPhaseStatusRequestJsonString{};
    Json::Value jsonObject;
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";

    jsonObject["MsgType"] = "CurrNextPhaseRequest";
    currentPhaseStatusRequestJsonString = Json::writeString(builder, jsonObject);

    return currentPhaseStatusRequestJsonString;
}

/*
    - If new priority request is received this method will obtain the current traffic signal Status.
    - If the current phase is in yellow or red state in the json string, the method set next phase as starting phase.
    - If the current phase is in yellow or red state, the method calculates the init (time to start starting phase) value.
    - If red clearance time for both phases are not same, one phase can be in red rest. In that case init time can be negative. The method sets init time same as the init time of other starting phase..
    - If starting phase is on rest or elapsed green time is more than gmax, the method sets the elapsed green time min green time.
*/
void PriorityRequestSolver::getCurrentSignalStatus(string jsonString)
{
    int temporaryCurrentPhase{};
    int temporaryNextPhase{};
    string temporaryPhaseState{};
    double temporaryElaspedTime{};
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    TrafficControllerData::TrafficConrtollerStatus tcStatus;
    trafficControllerStatus.clear();

    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;
    
    cout << "[" << currentTime << "] Received Current Signal Status" << endl;

    const Json::Value values = jsonObject["currentPhases"];

    for (int i = 0; i < NumberOfStartingPhase; i++)
    {
        for (size_t j = 0; j < values[i].getMemberNames().size(); j++)
        {
            if (values[i].getMemberNames()[j] == "Phase")
                temporaryCurrentPhase = values[i][values[i].getMemberNames()[j]].asInt();

            else if (values[i].getMemberNames()[j] == "State")
                temporaryPhaseState = values[i][values[i].getMemberNames()[j]].asString();

            else if (values[i].getMemberNames()[j] == "ElapsedTime")
                temporaryElaspedTime = (values[i][values[i].getMemberNames()[j]].asDouble()) / 10.0;
        }

        if (temporaryCurrentPhase < FirstPhaseOfRing2 && temporaryPhaseState == "green")
        {
            tcStatus.startingPhase1 = temporaryCurrentPhase;
            tcStatus.initPhase1 = Initialize;
            tcStatus.elapsedGreen1 = temporaryElaspedTime;
        }

        else if (temporaryCurrentPhase > LastPhaseOfRing1 && temporaryPhaseState == "green")
        {
            tcStatus.startingPhase2 = temporaryCurrentPhase;
            tcStatus.initPhase2 = Initialize;
            tcStatus.elapsedGreen2 = temporaryElaspedTime;
        }

        else if (temporaryPhaseState == "yellow")
        {
            for (int k = 0; k < NumberOfStartingPhase; k++)
            {
                temporaryNextPhase = (jsonObject["nextPhases"][k]).asInt();
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCurrentPhase; });
                if (temporaryNextPhase > 0 && temporaryNextPhase < FirstPhaseOfRing2)
                {
                    tcStatus.startingPhase1 = temporaryNextPhase;
                    tcStatus.initPhase1 = findSignalGroup->yellowChange + findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen1 = Initialize;
                }
                else if (temporaryNextPhase > LastPhaseOfRing1 && temporaryNextPhase <= LastPhaseOfRing2)
                {
                    tcStatus.startingPhase2 = temporaryNextPhase;
                    tcStatus.initPhase2 = findSignalGroup->yellowChange + findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen2 = Initialize;
                }
            }
        }

        else if (temporaryPhaseState == "red")
        {
            for (int k = 0; k < NumberOfStartingPhase; k++)
            {
                temporaryNextPhase = (jsonObject["nextPhases"][k]).asInt();
                /*
                    - Current phase and next phase can be same in case of T intersection.
                    - For example, the scenario when phase 4 is only yellow/red since phase 8 is missing. phase 2 and 6 was green before phase 4.
                    - In this case current signal status message can be following: 
                        {"currentPhases": [{"Phase": 4, "State": "yellow", "ElapsedTime": 5}, {"Phase": 6, "State": "red", "ElapsedTime": 136}], 
                        "MsgType": "CurrNextPhaseStatus", "nextPhases": [6]}
                */
                if (temporaryCurrentPhase == temporaryNextPhase) 
                {
                    cout << "[" << currentTime << "] Current Phase and next phase is same" << endl;
                    break;
                }

                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCurrentPhase; });
                
                if (temporaryNextPhase > 0 && temporaryNextPhase < FirstPhaseOfRing2)
                {
                    tcStatus.startingPhase1 = temporaryNextPhase;
                    //If red clearance time for both phases are not same, One phase will be in red rest. In that case we will get negative init time.
                    if ((findSignalGroup->redClear - temporaryElaspedTime) < 0.0) 
                        tcStatus.initPhase1 = 0.5;
                    
                    else
                        tcStatus.initPhase1 = findSignalGroup->redClear - temporaryElaspedTime;
                    
                    tcStatus.elapsedGreen1 = Initialize;
                }

                else if (temporaryNextPhase > LastPhaseOfRing1 && temporaryNextPhase <= LastPhaseOfRing2)
                {
                    tcStatus.startingPhase2 = temporaryNextPhase;
                    
                    if ((findSignalGroup->redClear - temporaryElaspedTime) < 0.0)
                        tcStatus.initPhase2 = 0.5;
                    
                    else
                        tcStatus.initPhase2 = findSignalGroup->redClear - temporaryElaspedTime;
                    
                    tcStatus.elapsedGreen2 = Initialize;
                }
            }
        }
    }

    trafficControllerStatus.push_back(tcStatus);
    validateTrafficControllerStatus();
    modifyTrafficControllerStatus();
}
/*
- If there is coordination request for coordinated phases:
    - phase elapsed time will be gmin if elapsed time in cycle is less than the coordinated phases upper limit of Green Time
    - for coordinated phases amount early retun time is deducted from the elapsed time after passing the gmin
    - otherwise phase elapsed time will be gmax - tolerace(say 1 second)
- If there is no coordination request then- 
    If signal phase is on rest or elapsed green time is more than gmax, then elapsed green time will be set as min green time.
*/
void PriorityRequestSolver::modifyTrafficControllerStatus()
{
    double currentTime{};
    double earlyReturnedValue{};
    double upperLimitOfGreenTimeForCoordinatedPhase{};
    double elapsedTimeInCycle{};
    bool coordinationRequestStatus{false};
    int temporaryPhase{};

    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        if (priorityRequestList[i].vehicleType == SignalCoordinationVehicleType)
        {
            coordinationRequestStatus = true;
            break;
        }

        else
            coordinationRequestStatus = false;
    }

    if (coordinationRequestStatus)
    {
        currentTime = getCurrentTime();
        elapsedTimeInCycle = fmod((currentTime - coordinationStartTime), cycleLength);

        //Temporary logging code for debugging coordination
        ofstream outputfile;
        ifstream infile;

        if (loggingStatus)
        {
            outputfile.open(fileName, std::ios_base::app);
            double timeNow = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

            outputfile << "\nThe elapsed time in a cycle at time " << timeNow << " is " << elapsedTimeInCycle << endl;
            outputfile.close();
        }

        for (size_t i = 0; i < trafficControllerStatus.size(); i++)
        {
            // For coordinated phase
            if (trafficControllerStatus[i].startingPhase1 == coordinatedPhase1)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase1;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
                //compute the early returned and upper limit of green time
                if (elapsedTimeInCycle > offset)
                    earlyReturnedValue = trafficControllerStatus[i].elapsedGreen1 + offset - elapsedTimeInCycle;

                else
                    earlyReturnedValue = 0.0;
                upperLimitOfGreenTimeForCoordinatedPhase = offset + findSignalGroup1->maxGreen;

                if ((elapsedTimeInCycle - Tolerance) >= upperLimitOfGreenTimeForCoordinatedPhase && (elapsedTimeInCycle - Tolerance) < (upperLimitOfGreenTimeForCoordinatedPhase + PRSTimedOutValue))
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->maxGreen - Tolerance;

                else if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->minGreen && earlyReturnedValue > 0)
                    trafficControllerStatus[i].elapsedGreen1 = trafficControllerStatus[i].elapsedGreen1 - earlyReturnedValue;

                //If early return value is greater than the gmin, elapsed green time value may become negative. Then set the elaped green time as min green
                if (trafficControllerStatus[i].elapsedGreen1 < 0)
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;
            }
            // For non-coordinated phases
            else if (trafficControllerStatus[i].startingPhase1 != coordinatedPhase1)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase1;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

                if (trafficControllerStatus[i].elapsedGreen1 >= findSignalGroup1->maxGreen - Tolerance)
                    trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->maxGreen - Tolerance;
            }
            // For coordinated phase
            if (trafficControllerStatus[i].startingPhase2 == coordinatedPhase2)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase2;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
                //compute the early returned and upper limit of green time
                if (elapsedTimeInCycle > offset)
                    earlyReturnedValue = trafficControllerStatus[i].elapsedGreen2 + offset - elapsedTimeInCycle;

                else
                    earlyReturnedValue = 0.0;
                upperLimitOfGreenTimeForCoordinatedPhase = offset + findSignalGroup2->maxGreen;

                if (elapsedTimeInCycle - Tolerance > upperLimitOfGreenTimeForCoordinatedPhase && (elapsedTimeInCycle - Tolerance) < (upperLimitOfGreenTimeForCoordinatedPhase + PRSTimedOutValue))
                    trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->maxGreen - Tolerance;

                else if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->minGreen && earlyReturnedValue > 0)
                    trafficControllerStatus[i].elapsedGreen2 = trafficControllerStatus[i].elapsedGreen2 - earlyReturnedValue;

                //If early return value is greater than the gmin, elapsed green time value may become negative. Then set the elaped green time as min green
                if (trafficControllerStatus[i].elapsedGreen2 < 0)
                    trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
            }
            // For non-coordinated phases
            else if (trafficControllerStatus[i].startingPhase2 != coordinatedPhase2)
            {
                temporaryPhase = trafficControllerStatus[i].startingPhase2;
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                           [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
                if (trafficControllerStatus[i].elapsedGreen2 >= findSignalGroup2->maxGreen - Tolerance)
                    trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->maxGreen - Tolerance;
            }
        }
    }

    else
    {
        for (size_t i = 0; i < trafficControllerStatus.size(); i++)
        {
            temporaryPhase = trafficControllerStatus[i].startingPhase1;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                       [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
            if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->minGreen)
                trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;

            temporaryPhase = trafficControllerStatus[i].startingPhase2;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                       [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
            if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->minGreen)
                trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
        }
    }
}

/*
    - The method is responsible for avoiding edge condition. It checks whether starting phase1 is in ring 1 and starting phase2 is in ring 2
    - For T-intersection starting phase can be zero. In that case, the method fills the the elapsed green time and the init time values with the elapsed green time and the init time values of other starting phase.
*/
void PriorityRequestSolver::validateTrafficControllerStatus()
{
    if (trafficControllerStatus[0].startingPhase1 == 0)
    {
        trafficControllerStatus[0].startingPhase1 = trafficControllerStatus[0].startingPhase2 - 4;
        trafficControllerStatus[0].elapsedGreen1 = trafficControllerStatus[0].elapsedGreen2;
        trafficControllerStatus[0].initPhase1 = trafficControllerStatus[0].initPhase2;
    }

    else if (trafficControllerStatus[0].startingPhase2 == 0)
    {
        trafficControllerStatus[0].startingPhase2 = trafficControllerStatus[0].startingPhase1 + 4;
        trafficControllerStatus[0].elapsedGreen2 = trafficControllerStatus[0].elapsedGreen1;
        trafficControllerStatus[0].initPhase2 = trafficControllerStatus[0].initPhase1;
    }
}

/*
    - Method for obtaining static traffic signal plan from TCI
*/
void PriorityRequestSolver::getCurrentSignalTimingPlan(string jsonString)
{
    OptimizationModelManager optimizationModelManager;
    TrafficControllerData::TrafficSignalPlan signalPlan;
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    cout << "[" << currentTime << "] Received Signal Timing Plan" << endl;
    loggingSignalPlanData(jsonString);

    trafficSignalPlan.clear();
    PhaseNumber.clear();
    PedWalk.clear();
    PedClear.clear();
    MinGreen.clear();
    Passage.clear();
    MaxGreen.clear();
    YellowChange.clear();
    RedClear.clear();
    PhaseRing.clear();
    P11.clear();
    P12.clear();
    P21.clear();
    P22.clear();

    noOfPhase = (jsonObject["TimingPlan"]["NoOfPhase"]).asInt();

    for (int i = 0; i < noOfPhase; i++)
        PhaseNumber.push_back((jsonObject["TimingPlan"]["PhaseNumber"][i]).asInt());

    for (int i = 0; i < noOfPhase; i++)
        PedWalk.push_back((jsonObject["TimingPlan"]["PedWalk"][i]).asDouble());

    for (int i = 0; i < noOfPhase; i++)
        PedClear.push_back((jsonObject["TimingPlan"]["PedClear"][i]).asDouble());

    for (int i = 0; i < noOfPhase; i++)
        MinGreen.push_back((jsonObject["TimingPlan"]["MinGreen"][i]).asDouble());

    for (int i = 0; i < noOfPhase; i++)
        Passage.push_back(((jsonObject["TimingPlan"]["Passage"][i]).asDouble()));

    for (int i = 0; i < noOfPhase; i++)
        MaxGreen.push_back((jsonObject["TimingPlan"]["MaxGreen"][i]).asDouble());

    for (int i = 0; i < noOfPhase; i++)
        YellowChange.push_back(((jsonObject["TimingPlan"]["YellowChange"][i]).asDouble()));

    for (int i = 0; i < noOfPhase; i++)
        RedClear.push_back(((jsonObject["TimingPlan"]["RedClear"][i]).asDouble()));

    for (int i = 0; i < noOfPhase; i++)
        PhaseRing.push_back((jsonObject["TimingPlan"]["PhaseRing"][i]).asInt());

    for (int i = 0; i < noOfPhase; i++)
    {
        signalPlan.phaseNumber = PhaseNumber[i];
        signalPlan.pedWalk = PedWalk[i];
        signalPlan.pedClear = PedClear[i];
        signalPlan.minGreen = MinGreen[i];
        signalPlan.passage = Passage[i];
        signalPlan.maxGreen = MaxGreen[i];
        signalPlan.yellowChange = YellowChange[i];
        signalPlan.redClear = RedClear[i];
        signalPlan.phaseRing = PhaseRing[i];
        trafficSignalPlan.push_back(signalPlan);
    }

    /*    
        - If phaseRing value is zero (0) for a phase that phase is inactive. 
        - To avoid the issue of inconsistant data, minGreen, maxGreen, passage, yellowChange and redClear value will be set as zero (0).
        - The phaseRing value will be also set.
    */
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {        
        if ((trafficSignalPlan[i].phaseRing == 0))
        {            
            trafficSignalPlan[i].minGreen = 0.0;
            trafficSignalPlan[i].maxGreen = 0.0;
            trafficSignalPlan[i].passage = 0.0;
            trafficSignalPlan[i].yellowChange = 0.0;
            trafficSignalPlan[i].redClear = 0.0;
            
            if (trafficSignalPlan[i].phaseNumber <= LastPhaseOfRing1)
                trafficSignalPlan[i].phaseRing = 1;
            
            else if (trafficSignalPlan[i].phaseNumber >= FirstPhaseOfRing2)
                trafficSignalPlan[i].phaseRing = 2;
        }

        else 
            continue;
    }

    //Obtain the phases in P11, P12, P21, P22
    for (int i = 0; i < noOfPhase; i++)
    {
        if (trafficSignalPlan[i].phaseNumber < 3 && trafficSignalPlan[i].phaseRing == 1)
            P11.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 2 && trafficSignalPlan[i].phaseNumber < 5 && trafficSignalPlan[i].phaseRing == 1)
            P12.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 4 && trafficSignalPlan[i].phaseNumber < 7 && trafficSignalPlan[i].phaseRing == 2)
            P21.push_back(trafficSignalPlan[i].phaseNumber);

        else if (trafficSignalPlan[i].phaseNumber > 6 && trafficSignalPlan[i].phaseRing == 2)
            P22.push_back(trafficSignalPlan[i].phaseNumber);
    }

    optimizationModelManager.generateModFile(noOfPhase, PhaseNumber, P11, P12, P21, P22);
    modifySignalTimingPlan();
}

/*
    - The following method modify the gmax of the traffic signal plan based on Split data
*/
void PriorityRequestSolver::getSignalCoordinationTimingPlan(string jsonString)
{
    TrafficControllerData::TrafficSignalPlan signalPlan;
    trafficSignalPlan_SignalCoordination.clear();
    double splitValue{};
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    cout << "[" << currentTime << "] Received Split Data for Signal Coordination" << endl;
    loggingSplitData(jsonString);

    
    int noOfSplitData = (jsonObject["TimingPlan"]["NoOfPhase"]).asInt();
    cycleLength = jsonObject["CycleLength"].asDouble();
    offset = jsonObject["Offset"].asDouble();
    coordinationStartTime = jsonObject["CoordinationStartTime_Hour"].asDouble() * HourToSecondConversion + jsonObject["CoordinationStartTime_Minute"].asDouble() * MinuteToSecondCoversion;
    coordinatedPhase1 = jsonObject["CoordinatedPhase1"].asInt();
    coordinatedPhase2 = jsonObject["CoordinatedPhase2"].asInt();

    for (int i = 0; i < noOfSplitData; i++)
    {
        splitValue = jsonObject["TimingPlan"]["SplitData"][i].asDouble();
        signalPlan.reset();

        signalPlan.phaseNumber = jsonObject["TimingPlan"]["PhaseNumber"][i].asInt();
        signalPlan.yellowChange = trafficSignalPlan[i].yellowChange;
        signalPlan.redClear = trafficSignalPlan[i].redClear;
        signalPlan.minGreen = trafficSignalPlan[i].minGreen;
        signalPlan.phaseRing = trafficSignalPlan[i].phaseRing;
        if (splitValue != 0)
            signalPlan.maxGreen = splitValue - trafficSignalPlan[i].yellowChange - trafficSignalPlan[i].redClear;

        trafficSignalPlan_SignalCoordination.push_back(signalPlan);
    }
    modifyCoordinationSignalTimingPlan();
}

/*
    - Method printing the traffic signal plan
*/
void PriorityRequestSolver::printSignalPlan()
{
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        cout << trafficSignalPlan[i].phaseNumber << " " << trafficSignalPlan[i].phaseRing << " " << trafficSignalPlan[i].minGreen << endl;
    }
}

/*
    - If Phase information is missing (for example T-intersection) for the through phases, the following method will fill the information based on through or associated phases
    - The method finds the missing through phases and copy the data of associated through phases. For example if phase 4 data is missing, then fill the missing data with phase 8 data.
*/
void PriorityRequestSolver::modifySignalTimingPlan()
{
    int temporarySignalGroup{};
    int temporaryCompitableSignalGroup{};

    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        temporarySignalGroup = trafficSignalPlan[i].phaseNumber;

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporarySignalGroup; });

        if ((temporarySignalGroup % 2 == 0) && (trafficSignalPlan[i].minGreen == 0))
        {
            if (temporarySignalGroup < FirstPhaseOfRing2)
                temporaryCompitableSignalGroup = temporarySignalGroup + NumberOfPhasePerRing;
            else if (temporarySignalGroup > LastPhaseOfRing1)
                temporaryCompitableSignalGroup = temporarySignalGroup - NumberOfPhasePerRing;

            vector<TrafficControllerData::TrafficSignalPlan>::iterator findCompitableSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                      [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCompitableSignalGroup; });

            findSignalGroupOnList->pedWalk = findCompitableSignalGroupOnList->pedWalk;
            findSignalGroupOnList->pedClear = findCompitableSignalGroupOnList->pedClear;
            findSignalGroupOnList->minGreen = findCompitableSignalGroupOnList->minGreen;
            findSignalGroupOnList->passage = findCompitableSignalGroupOnList->passage;
            findSignalGroupOnList->maxGreen = findCompitableSignalGroupOnList->maxGreen;
            findSignalGroupOnList->yellowChange = findCompitableSignalGroupOnList->yellowChange;
            findSignalGroupOnList->redClear = findCompitableSignalGroupOnList->redClear;
        }

        else
            continue;
    }
}

/*
    - The method contains the same kind of logic as modifySignalTimingPlan(). This method modifies coordination signal timing plan.
    - If Phase information is missing (for example T-intersection) for the through phases, the following method will fill the information based on through or associated phases
    - The method finds the missing through phases and copy the data of associated through phases. For example if phase 4 data is missing, then fill the missing data with phase 8 data.
*/
void PriorityRequestSolver::modifyCoordinationSignalTimingPlan()
{
    int temporarySignalGroup{};
    int temporaryCompitableSignalGroup{};

    for (size_t i = 0; i < trafficSignalPlan_SignalCoordination.size(); i++)
    {
        temporarySignalGroup = trafficSignalPlan_SignalCoordination[i].phaseNumber;

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporarySignalGroup; });

        if ((temporarySignalGroup % 2 == 0) && (trafficSignalPlan_SignalCoordination[i].minGreen == 0 || trafficSignalPlan_SignalCoordination[i].maxGreen == 0))
        {
            if (temporarySignalGroup < FirstPhaseOfRing2)

                temporaryCompitableSignalGroup = temporarySignalGroup + 4;
            else if (temporarySignalGroup > LastPhaseOfRing1)
                temporaryCompitableSignalGroup = temporarySignalGroup - 4;

            vector<TrafficControllerData::TrafficSignalPlan>::iterator findCompitableSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                                      [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCompitableSignalGroup; });

            findSignalGroupOnList->pedWalk = findCompitableSignalGroupOnList->pedWalk;
            findSignalGroupOnList->pedClear = findCompitableSignalGroupOnList->pedClear;
            findSignalGroupOnList->minGreen = findCompitableSignalGroupOnList->minGreen;
            findSignalGroupOnList->passage = findCompitableSignalGroupOnList->passage;
            findSignalGroupOnList->maxGreen = findCompitableSignalGroupOnList->maxGreen;
            findSignalGroupOnList->yellowChange = findCompitableSignalGroupOnList->yellowChange;
            findSignalGroupOnList->redClear = findCompitableSignalGroupOnList->redClear;
        }

        else
            continue;
    }
}

/*
    - This method checks whether emergency vehicle priority request is in the list or not
*/
bool PriorityRequestSolver::findEVInList()
{
    if (priorityRequestList.empty())
        emergencyVehicleStatus = false;
    else
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::special))
            {
                emergencyVehicleStatus = true;
                break;
            }

            else
                emergencyVehicleStatus = false;
        }
    }

    return emergencyVehicleStatus;
}

/*
    - This method checks whether signal coordination priority request is in the list or not
*/
bool PriorityRequestSolver::findCoordinationRequestInList()
{
    if (priorityRequestList.empty())
        signalCoordinationRequestStatus = false;

    else
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            if (priorityRequestList[i].vehicleType == SignalCoordinationVehicleType)
            {
                signalCoordinationRequestStatus = true;
                break;
            }

            else
                signalCoordinationRequestStatus = false;
        }
    }

    return signalCoordinationRequestStatus;
}

bool PriorityRequestSolver::getOptimalSolutionValidationStatus()
{
    return optimalSolutionStatus;
}

/*
    - PRSolver obtains the signal timing plan from the TCI. At the start of the program it asks TCI to send the static signal timing plan 
    - This method creats that signal timing plan request string.
*/
string PriorityRequestSolver::getSignalTimingPlanRequestString()
{
    std::string jsonString{};
    Json::Value jsonObject;
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";

    jsonObject["MsgType"] = "TimingPlanRequest";
    jsonString = Json::writeString(builder, jsonObject);

    return jsonString;
}

/*
    - PRSolver obtains the coordination Coordination plan (split data) from the Signal Coordination Request Generator. At the start of the program it asks Signal Coordination Request Generator to send the active split data if it has
    - This method creats that split data request string.
*/
string PriorityRequestSolver::getSignalCoordinationTimingPlanRequestString()
{
    std::string jsonString{};
    Json::Value jsonObject;
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";

    jsonObject["MsgType"] = "CoordinationPlanRequest";
    jsonString = Json::writeString(builder, jsonObject);

    return jsonString;
}

double PriorityRequestSolver::getSeconds()
{
    struct timeval tv_tt;
    gettimeofday(&tv_tt, NULL);
    return (static_cast<double>(tv_tt.tv_sec) + static_cast<double>(tv_tt.tv_usec) / 1.e6);
}

double PriorityRequestSolver::getCurrentTime()
{
    double currentTime{};
    time_t s = 1;
    struct tm *current_time;

    // time in seconds
    s = time(NULL);

    // to get current time
    current_time = localtime(&s);

    currentTime = current_time->tm_hour * 3600.00 + current_time->tm_min * 60.00 + current_time->tm_sec;

    return currentTime;
}

/*
    -Check whether active signal coordination timing plan is available or not
*/
bool PriorityRequestSolver::checkSignalCoordinationTimingPlan()
{
    bool sendCoordinationPlanRequest{false};

    if (priorityRequestList.empty())
        sendCoordinationPlanRequest = false;
    else
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            if ((priorityRequestList[i].vehicleType == SignalCoordinationVehicleType) && trafficSignalPlan_SignalCoordination.size() == 0)
            {
                sendCoordinationPlanRequest = true;
                break;
            }

            else
                sendCoordinationPlanRequest = false;
        }
    }

    return sendCoordinationPlanRequest;
}

/*
    -Check whether to log data or not
*/
bool PriorityRequestSolver::logging()
{
    string logging{};
    string intersectionName{};
    ofstream outputfile;
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
    delete reader;

    logging = jsonObject["Logging"].asString();
    intersectionName = jsonObject["IntersectionName"].asString();
    fileName = "/nojournal/bin/log/PRSolverLog-" + intersectionName + ".txt";
    if (logging == "True")
    {
        loggingStatus = true;
        auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        outputfile.open(fileName);
        outputfile << "File opened at time : " << currentTime << std::endl;
        outputfile.close();
    }
    else
        loggingStatus = false;

    return loggingStatus;
}

/*
    - Loggers to log priority request string, signal status string, OptimizationModelData.dat and results.txt files
*/
void PriorityRequestSolver::loggingOptimizationData(string priorityRequestString, string signalStatusString, string scheduleString)
{
    ofstream outputfile;
    ifstream infile;

    if (loggingStatus == true)
    {
        // outputfile.open("/nojournal/bin/log/PRSolver_Log" + std::to_string(currentTime) + ".txt");
        outputfile.open(fileName, std::ios_base::app);
        double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

        outputfile << "\nFollowing Priority Request is received from PRS at time " << currentTime << endl;
        outputfile << priorityRequestString << endl;

        outputfile << "\nFollowing Signal Status is received from TCI at time " << currentTime << endl;
        outputfile << signalStatusString << endl;

        outputfile << "\nCurrent Dat File at time : " << currentTime << endl;
        infile.open("/nojournal/bin/OptimizationModelData.dat");

        for (string line; getline(infile, line);)
            outputfile << line << endl;

        infile.close();

        outputfile << "\nCurrent Results File at time : " << currentTime << endl;
        infile.open("/nojournal/bin/Results.txt");
        for (std::string line; getline(infile, line);)
            outputfile << line << endl;

        infile.close();

        outputfile << "\nFollowing Schedule will send to TCI at time " << currentTime << endl;
        outputfile << scheduleString << endl;

        outputfile.close();
    }
}

/*
    - Loggers to log static signal timing plan data
*/
void PriorityRequestSolver::loggingSignalPlanData(string jsonString)
{
    if (loggingStatus)
    {
        ofstream outputfile;
        outputfile.open(fileName, std::ios_base::app);
        double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

        outputfile << "\nFollowing Signal Plan is received from TCI at time " << currentTime << endl;
        outputfile << jsonString << endl;
        outputfile.close();
    }
}

/*
    - Loggers to log split data for signal coordinatio
*/
void PriorityRequestSolver::loggingSplitData(string jsonString)
{
    if (loggingStatus)
    {
        ofstream outputfile;
        outputfile.open(fileName, std::ios_base::app);
        double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

        outputfile << "\nFollowing Split Data is received from Signal Coordination Generator at time " << currentTime << endl;
        outputfile << jsonString << endl;
        outputfile.close();
    }
}
/*
    - Loggers to log clear request string
*/
void PriorityRequestSolver::loggingClearRequestData(string jsonString)
{
    if (loggingStatus)
    {
        ofstream outputfile;
        outputfile.open(fileName, std::ios_base::app);
        double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

        outputfile << "\nFollowing Clear Request is sent to TCI at time " << currentTime << endl;
        outputfile << jsonString << endl;
        outputfile.close();
    }
}

PriorityRequestSolver::~PriorityRequestSolver()
{
}