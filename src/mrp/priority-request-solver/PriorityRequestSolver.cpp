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
#include <algorithm>
#include <cmath>
#include "msgEnum.h"
#include <time.h>

PriorityRequestSolver::PriorityRequestSolver()
{
    readConfigFile();
    getPriorityWeights();
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
            displayConsoleData("Message type is unknown");
    }

    return messageType;
}

/*
    - This method is responsible for creating priority request list received from the PRS as Json String.
*/
void PriorityRequestSolver::createPriorityRequestList(string jsonString)
{
    RequestList requestList;
    priorityRequestList.clear();
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    displayConsoleData("Received Priority Request List from PRS");
    loggingData("Received Priority Request List from PRS");
    loggingData(jsonString);

    int noOfRequest = (jsonObject["PriorityRequestList"]["noOfRequest"]).asInt();

    for (int i = 0; i < noOfRequest; i++)
    {
        requestList.reset();
        requestList.vehicleID = jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleID"].asInt();
        requestList.vehicleType = jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleType"].asInt();
        requestList.basicVehicleRole = jsonObject["PriorityRequestList"]["requestorInfo"][i]["basicVehicleRole"].asInt();
        requestList.vehicleETA = jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"].asDouble();
        requestList.vehicleETA_Duration = jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"].asDouble();
        requestList.requestedPhase = jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"].asInt();
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
void PriorityRequestSolver::setDilemmaZoneRequesStatus()
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
                priorityRequestList[i].dilemmaZoneStatus = true;
        }
    }
}

/*
    - If priority weight is zero for a particular type of request, following method removes those requests which matches the corresponding vehicle type.
    - If there is EV priority request in the list, following method deletes all the priority requests from the list apart from EV request.
        - Checks vehicle type for all the received request.
        - If vehicle type is not EV(For EV vehicleType is 2) remove that request from the list.
*/
void PriorityRequestSolver::modifyPriorityRequestList()
{
    int temporaryVehicleID{};

    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        temporaryVehicleID = priorityRequestList[i].vehicleID;

        if ((priorityRequestList[i].vehicleType == EmergencyVehicle && EmergencyVehicleWeight == 0.0) ||
            (priorityRequestList[i].vehicleType == Transit && TransitWeight == 0.0) || (priorityRequestList[i].vehicleType == Truck && TruckWeight == 0.0) ||
            (priorityRequestList[i].vehicleType == CoordinationVehicleType && CoordinationWeight == 0.0))
        {
            vector<RequestList>::iterator findVehicleIDOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                             [&](RequestList const &p)
                                                                             { return p.vehicleID == temporaryVehicleID; });

            priorityRequestList.erase(findVehicleIDOnList);
            i--;
        }

        else if (emergencyVehicleStatus && priorityRequestList[i].vehicleType != EmergencyVehicle && !priorityRequestList[i].dilemmaZoneStatus)
        {
            vector<RequestList>::iterator findVehicleIDOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                             [&](RequestList const &p)
                                                                             { return p.vehicleID == temporaryVehicleID; });

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
void PriorityRequestSolver::managePriorityRequestListForEV()
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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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

            if ((trafficControllerStatus[j].startingPhase2 == 5) || (trafficControllerStatus[j].startingPhase2 == 6))
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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
        //If second ring-barrier group is empty, the left turn phases (1,5) of first ring-barrier group can be removed (if they are not starting phase). Dummy through phases (4,8) will be inserted in the second ring-barrier group.
        else if (requestedEV_P12.empty() && requestedEV_P22.empty())
        {
            if ((trafficControllerStatus[0].startingPhase1 != 1))
            {
                temporaryPhase = 1;
                for (size_t j = 0; j < priorityRequestList.size(); j++)
                {
                    vector<RequestList>::iterator findSignalGroupOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
                                                                                       [&](RequestList const &p)
                                                                                       { return p.requestedPhase == temporaryPhase; });

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
    - If there is EV priority request, the method creates an instance of OptimizationModelManager class to create dynamic mod file for EV.
    - This function also calls SolverDataManager class to write the dat dile
*/
void PriorityRequestSolver::setOptimizationInput()
{
    if (emergencyVehicleStatus)
    {
        OptimizationModelManager optimizationModelManager(FlexibilityWeight);

        setDilemmaZoneRequesStatus();
        modifyPriorityRequestList();
        getRequestedSignalGroup();
        managePriorityRequestListForEV();
        getEVPhases();
        getEVTrafficSignalPlan();
        optimizationModelManager.generateEVModFile(trafficSignalPlan_EV, EV_P11, EV_P12, EV_P21, EV_P22);

        SolverDataManager solverDataManager(dilemmaZoneRequestList, priorityRequestList, trafficControllerStatus,
                                            trafficSignalPlan_EV, conflictingPedCallList, requestedSignalGroup, EmergencyVehicleWeight,
                                            EmergencyVehicleSplitPhaseWeight, TransitWeight, TruckWeight,
                                            DilemmaZoneRequestWeight, CoordinationWeight);

        solverDataManager.modifyGreenMax(emergencyVehicleStatus);
        solverDataManager.modifyGreenTimeForConflictingPedCalls();
        solverDataManager.modifyGreenTimeForCurrentPedCalls();
        solverDataManager.validateGmaxForEVSignalTimingPlan(EV_P11, EV_P12, EV_P21, EV_P22);
        solverDataManager.adjustGreenTimeForPedCall(EV_P11, EV_P12, EV_P21, EV_P22);
        solverDataManager.modifyCurrentSignalStatus(EV_P11, EV_P12, EV_P21, EV_P22);
        solverDataManager.generateDatFile(emergencyVehicleStatus, 0.0, 0.0, 2, 6); //As a defult early return values are passing as 0 and coordinated phases as 2 and 6
    }

    else if (transitOrTruckRequestStatus)
    {
        modifyPriorityRequestList();
        SolverDataManager solverDataManager(dilemmaZoneRequestList, priorityRequestList, trafficControllerStatus,
                                            trafficSignalPlan, conflictingPedCallList, EmergencyVehicleWeight,
                                            EmergencyVehicleSplitPhaseWeight, TransitWeight, TruckWeight,
                                            DilemmaZoneRequestWeight, CoordinationWeight);

        solverDataManager.getRequestedSignalGroupFromPriorityRequestList();
        solverDataManager.addAssociatedSignalGroup();
        solverDataManager.modifyGreenMax(emergencyVehicleStatus);
        solverDataManager.modifyGreenTimeForConflictingPedCalls();
        solverDataManager.modifyGreenTimeForCurrentPedCalls();
        solverDataManager.adjustGreenTimeForPedCall(P11, P12, P21, P22);
        solverDataManager.modifyCurrentSignalStatus(P11, P12, P21, P22);
        solverDataManager.generateDatFile(emergencyVehicleStatus, 0.0, 0.0, 2, 6);
    }

    else if (signalCoordinationRequestStatus)
    {
        modifyPriorityRequestList();
        SolverDataManager solverDataManager(dilemmaZoneRequestList, priorityRequestList, trafficControllerStatus,
                                            trafficSignalPlan_SignalCoordination, conflictingPedCallList, EmergencyVehicleWeight,
                                            EmergencyVehicleSplitPhaseWeight, TransitWeight, TruckWeight,
                                            DilemmaZoneRequestWeight, CoordinationWeight);

        solverDataManager.getRequestedSignalGroupFromPriorityRequestList();
        solverDataManager.addAssociatedSignalGroup();
        solverDataManager.modifyGreenMax(emergencyVehicleStatus);
        solverDataManager.modifyGreenTimeForConflictingPedCalls();
        solverDataManager.modifyGreenTimeForCurrentPedCalls();
        solverDataManager.adjustGreenTimeForPedCall(P11, P12, P21, P22);
        solverDataManager.modifyCurrentSignalStatus(P11, P12, P21, P22);
        solverDataManager.generateDatFile(emergencyVehicleStatus, earlyReturnedValue1, earlyReturnedValue2, coordinatedPhase1, coordinatedPhase2);
        priorityRequestList = solverDataManager.getPriorityRequestList();
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
    - Solution will be written in the /nojournal/bin/OptimizationResults.txt file
*/
void PriorityRequestSolver::GLPKSolver()
{
    double startOfSolve = getPosixTimestamp();
    double endOfSolve{};
    int ret{};
    int fail{0};
    char modFile[128] = "/nojournal/bin/OptimizationModel.mod";
    glp_prob *mip;
    glp_tran *tran;

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

    fail = glp_intopt(mip, NULL);
    endOfSolve = getPosixTimestamp();

    if (!fail)
        displayConsoleData("Successfully solved the optimization problem");

    else
        displayConsoleData(" Failed to solved the optimization problem successfully");

    displayConsoleData("Time requires to Solve the optimization problem is " + std::to_string(endOfSolve - startOfSolve));
    loggingData("Time requires to Solve the optimization problem is " + std::to_string(endOfSolve - startOfSolve));

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
    - This function doesn't solve the optimization problem and send the schedule when priority request list is empty (priority request list is empty when priority weight is zero). 
*/
string PriorityRequestSolver::getScheduleforTCI()
{
    scheduleJsonString.clear();
    setOptimizationInput();

    if (!priorityRequestList.empty())
    {
        GLPKSolver();

        if (emergencyVehicleStatus)
        {
            ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan_EV, emergencyVehicleStatus);

            scheduleManager.obtainRequiredSignalGroup();
            scheduleManager.readOptimalSignalPlan();
            scheduleManager.createEventList();
            optimalSolutionStatus = scheduleManager.validateOptimalSolution();
            scheduleJsonString = scheduleManager.createScheduleJsonString();
        }

        else if (transitOrTruckRequestStatus)
        {
            ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan, emergencyVehicleStatus);
            scheduleManager.obtainRequiredSignalGroup();
            scheduleManager.readOptimalSignalPlan();
            scheduleManager.createEventList();
            optimalSolutionStatus = scheduleManager.validateOptimalSolution();
            scheduleJsonString = scheduleManager.createScheduleJsonString();
        }

        else if (signalCoordinationRequestStatus)
        {
            ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan_SignalCoordination, emergencyVehicleStatus);
            scheduleManager.obtainRequiredSignalGroup();
            scheduleManager.readOptimalSignalPlan();
            scheduleManager.createEventList();
            optimalSolutionStatus = scheduleManager.validateOptimalSolution();
            scheduleJsonString = scheduleManager.createScheduleJsonString();
        }
    }

    else 
        optimalSolutionStatus = false;

    priorityRequestList.clear();
    dilemmaZoneRequestList.clear();
    trafficControllerStatus.clear();
    conflictingPedCallList.clear();
    earlyReturnedValue1 = 0.0;
    earlyReturnedValue2 = 0.0;

    return scheduleJsonString;
}

/*
    - This method is reponsible for creating a JSON formatted string to clear the schedule for TCI.
*/
string PriorityRequestSolver::getClearCommandScheduleforTCI()
{
    string clearScheduleJsonString{};
    ScheduleManager scheduleManager;

    clearScheduleJsonString = scheduleManager.createScheduleJsonString();

    displayConsoleData("Received Clear Request from PRS");
    loggingData("Received Clear Request from PRS");
    displayConsoleData("Clear Request message will send to TCI");
    loggingData("Clear Request message will send to TCI");
    loggingData(clearScheduleJsonString);

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

    // Delete the phases which are in the plannedEVPhases element
    for (size_t j = 0; j < plannedEVPhases.size(); j++)
    {
        temporaryPhase = plannedEVPhases.at(j);
        it = std::find(temporaryPhaseNumber.begin(), temporaryPhaseNumber.end(), temporaryPhase);

        if (it != temporaryPhaseNumber.end())
            temporaryPhaseNumber.erase(it);
    }

    // Delete the signal plan object which are in trafficSignalPlan_EV vector
    for (size_t i = 0; i < temporaryPhaseNumber.size(); i++)
    {
        temporaryPhase = temporaryPhaseNumber.at(i);

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan_EV), std::end(trafficSignalPlan_EV),
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p)
                                                                                                        { return p.phaseNumber == temporaryPhase; });

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

        if (trafficSignalPlan_EV[i].minGreen == 0)
        {
            if ((temporarySignalGroup % 2 == 0) && (temporarySignalGroup < FirstPhaseOfRing2))
                associatedSignalGroup = temporarySignalGroup + 4;

            else if ((temporarySignalGroup % 2 == 0) && (temporarySignalGroup > LastPhaseOfRing1))
                associatedSignalGroup = temporarySignalGroup - 4;

            else if ((temporarySignalGroup % 2 != 0) && (temporarySignalGroup < FirstPhaseOfRing2))
                associatedSignalGroup = temporarySignalGroup + 5;

            else if ((temporarySignalGroup % 2 != 0) && (temporarySignalGroup > LastPhaseOfRing1))
                associatedSignalGroup = temporarySignalGroup - 3;

            vector<TrafficControllerData::TrafficSignalPlan>::iterator findAssociatedSignalGroupOnList =
                std::find_if(std::begin(trafficSignalPlan_EV), std::end(trafficSignalPlan_EV),
                             [&](TrafficControllerData::TrafficSignalPlan const &p)
                             { return p.phaseNumber == associatedSignalGroup; });

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
    - The following method call TrafficConrtollerStatusManager class to manage current signal status.
*/
void PriorityRequestSolver::getCurrentSignalStatus(string jsonString)
{
    double elapsedTimeInCycle{};
    vector<double> coordinatedPhasesEarlyReturnValue{};

    displayConsoleData("Received Current Signal Status from TCI");
    loggingData("Received Current Signal Status from TCI");
    loggingData(jsonString);

    findEmergencyVehicleRequestInList();
    findTransitOrTruckRequestInList();
    findCoordinationRequestInList();

    if (transitOrTruckRequestStatus || emergencyVehicleStatus)
    {
        TrafficConrtollerStatusManager trafficConrtollerStatusManager(emergencyVehicleStatus, transitOrTruckRequestStatus, signalCoordinationRequestStatus, cycleLength, offset,
                                                                      coordinationStartTime, elapsedTimeInCycle, coordinatedPhase1, coordinatedPhase2,
                                                                      logging, consoleOutput, dummyPhasesList,
                                                                      trafficSignalPlan);

        trafficControllerStatus = trafficConrtollerStatusManager.getTrafficControllerStatus(jsonString);

        if (trafficConrtollerStatusManager.getConflictingPedCallStatus())
        {
            conflictingPedCallList = trafficConrtollerStatusManager.getConflictingPedCallList();
            displayConsoleData("Conflicting Ped Call is available!");
            loggingData("Conflicting Ped Call is available!");
        }
    }

    else if (!transitOrTruckRequestStatus && signalCoordinationRequestStatus)
    {
        double currentTimeOfToday = getCurrentTime();
        elapsedTimeInCycle = fmod((currentTimeOfToday - coordinationStartTime - offset), cycleLength);
        loggingData("The elapsed time in a cycle is " + std::to_string(elapsedTimeInCycle));

        TrafficConrtollerStatusManager trafficConrtollerStatusManager(emergencyVehicleStatus, transitOrTruckRequestStatus, signalCoordinationRequestStatus, cycleLength, offset,
                                                                      coordinationStartTime, elapsedTimeInCycle, coordinatedPhase1, coordinatedPhase2,
                                                                      logging, consoleOutput, dummyPhasesList,
                                                                      trafficSignalPlan_SignalCoordination);

        trafficControllerStatus = trafficConrtollerStatusManager.getTrafficControllerStatus(jsonString);

        if (trafficConrtollerStatusManager.getConflictingPedCallStatus())
        {
            conflictingPedCallList = trafficConrtollerStatusManager.getConflictingPedCallList();
            displayConsoleData("Conflicting Ped Call is available!");
            loggingData("Conflicting Ped Call is available!");
        }

        coordinatedPhasesEarlyReturnValue = trafficConrtollerStatusManager.getEarlyReturnValue();
        earlyReturnedValue1 = coordinatedPhasesEarlyReturnValue.at(0);
        earlyReturnedValue2 = coordinatedPhasesEarlyReturnValue.at(1);
    }
}

/*
    - Method for obtaining static traffic signal plan from TCI
*/
void PriorityRequestSolver::setCurrentSignalTimingPlan(string jsonString)
{
    OptimizationModelManager optimizationModelManager(FlexibilityWeight);
    TrafficControllerData::TrafficSignalPlan signalPlan;
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    displayConsoleData("Received Signal Timing Plan from TCI");
    loggingData("Received Signal Timing Plan from TCI");
    loggingData(jsonString);

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
    getDummyPhases();
    modifySignalTimingPlan();
}

/*
    - The following method modify the gmax of the traffic signal plan based on Split data
*/
void PriorityRequestSolver::setSignalCoordinationTimingPlan(string jsonString)
{
    TrafficControllerData::TrafficSignalPlan signalPlan;
    trafficSignalPlan_SignalCoordination.clear();
    double splitValue{};
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    string errors{};
    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    delete reader;

    displayConsoleData("Received Split Data form SignalCoordinationRequestGenerator");
    loggingData("Received Split Data form SignalCoordinationRequestGenerator");
    loggingData(jsonString);

    int noOfSplitData = (jsonObject["TimingPlan"]["NoOfPhase"]).asInt();
    cycleLength = jsonObject["CycleLength"].asDouble();
    offset = jsonObject["Offset"].asDouble();
    coordinationStartTime = jsonObject["CoordinationStartTime_Hour"].asDouble() * HourToSecondConversion + jsonObject["CoordinationStartTime_Minute"].asDouble() * MinuteToSecondCoversion;
    coordinatedPhase1 = jsonObject["CoordinatedPhase1"].asInt();
    coordinatedPhase2 = jsonObject["CoordinatedPhase2"].asInt();

    if (!trafficSignalPlan.empty())
    {
        for (int i = 0; i < noOfSplitData; i++)
        {
            splitValue = jsonObject["TimingPlan"]["SplitData"][i].asDouble();
            signalPlan.reset();

            signalPlan.phaseNumber = jsonObject["TimingPlan"]["PhaseNumber"][i].asInt();
            signalPlan.pedWalk = trafficSignalPlan[i].pedWalk;
            signalPlan.pedClear = trafficSignalPlan[i].pedClear;
            signalPlan.minGreen = trafficSignalPlan[i].minGreen;
            signalPlan.passage = trafficSignalPlan[i].passage;
            signalPlan.yellowChange = trafficSignalPlan[i].yellowChange;
            signalPlan.redClear = trafficSignalPlan[i].redClear;
            signalPlan.phaseRing = trafficSignalPlan[i].phaseRing;

            if (splitValue != 0)
                signalPlan.maxGreen = splitValue - trafficSignalPlan[i].yellowChange - trafficSignalPlan[i].redClear;

            trafficSignalPlan_SignalCoordination.push_back(signalPlan);
        }
        modifyCoordinationSignalTimingPlan();
    }
}

/*
    - Method for getting the phases are not in use, based on the Gmin value in the traffic signal plan
*/
void PriorityRequestSolver::getDummyPhases()
{
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        if (trafficSignalPlan[i].minGreen == 0)
            dummyPhasesList.push_back(trafficSignalPlan[i].phaseNumber);
    }
}

/*
    - Method printing the traffic signal plan
*/
void PriorityRequestSolver::printSignalPlan()
{
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
        cout << trafficSignalPlan[i].phaseNumber << " " << trafficSignalPlan[i].phaseRing << " " << trafficSignalPlan[i].minGreen << endl;
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
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p)
                                                                                                        { return p.phaseNumber == temporarySignalGroup; });

        if ((temporarySignalGroup % 2 == 0) && (trafficSignalPlan[i].minGreen == 0))
        {
            if (temporarySignalGroup < FirstPhaseOfRing2)
                temporaryCompitableSignalGroup = temporarySignalGroup + NumberOfPhasePerRing;
            else if (temporarySignalGroup > LastPhaseOfRing1)
                temporaryCompitableSignalGroup = temporarySignalGroup - NumberOfPhasePerRing;

            vector<TrafficControllerData::TrafficSignalPlan>::iterator findCompitableSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                      [&](TrafficControllerData::TrafficSignalPlan const &p)
                                                                                                                      { return p.phaseNumber == temporaryCompitableSignalGroup; });

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
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p)
                                                                                                        { return p.phaseNumber == temporarySignalGroup; });

        if ((temporarySignalGroup % 2 == 0) && (trafficSignalPlan_SignalCoordination[i].minGreen == 0 || trafficSignalPlan_SignalCoordination[i].maxGreen == 0))
        {
            if (temporarySignalGroup < FirstPhaseOfRing2)

                temporaryCompitableSignalGroup = temporarySignalGroup + 4;
            else if (temporarySignalGroup > LastPhaseOfRing1)
                temporaryCompitableSignalGroup = temporarySignalGroup - 4;

            vector<TrafficControllerData::TrafficSignalPlan>::iterator findCompitableSignalGroupOnList = std::find_if(std::begin(trafficSignalPlan_SignalCoordination), std::end(trafficSignalPlan_SignalCoordination),
                                                                                                                      [&](TrafficControllerData::TrafficSignalPlan const &p)
                                                                                                                      { return p.phaseNumber == temporaryCompitableSignalGroup; });

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
bool PriorityRequestSolver::findEmergencyVehicleRequestInList()
{
    emergencyVehicleStatus = false;

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

    return emergencyVehicleStatus;
}

/*
    - This method checks whether transit or truck priority request is in the list or not
*/
bool PriorityRequestSolver::findTransitOrTruckRequestInList()
{
    transitOrTruckRequestStatus = false;

    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        if ((priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::bus)) ||
            (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::axleCnt4)))
        {
            transitOrTruckRequestStatus = true;
            break;
        }

        else
            transitOrTruckRequestStatus = false;
    }

    return transitOrTruckRequestStatus;
}

/*
    - This method checks whether signal coordination priority request is in the list or not
*/
bool PriorityRequestSolver::findCoordinationRequestInList()
{
    signalCoordinationRequestStatus = false;

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

    return signalCoordinationRequestStatus;
}

bool PriorityRequestSolver::getOptimalSolutionValidationStatus()
{
    if (optimalSolutionStatus)
    {
        loggingOptimizationData();
        displayConsoleData("The optimal schedule will send to TCI");
        loggingData("The optimal schedule will send to TCI");
        loggingData(scheduleJsonString);
    }

    else
    {
        displayConsoleData("The schedule will not send to TCI since it doesn't pass validation process");
        loggingData("The schedule will not send to TCI since it doesn't pass validation process");
    }

    return optimalSolutionStatus;
}

bool PriorityRequestSolver::checkUpdatesForPriorityWeights()
{
    bool priorityWeightsCheckingRequirement{false};
    double currentTime = getPosixTimestamp();

    if (currentTime - priorityWeightsCheckedTime > 120.0)
        priorityWeightsCheckingRequirement = true;

    return priorityWeightsCheckingRequirement;
}
/*
    - The following methods read the priority weights from the config file.
*/
void PriorityRequestSolver::getPriorityWeights()
{
    double currentTime = getPosixTimestamp();
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
    FlexibilityWeight = jsonObject["PriorityParameter"]["FlexibilityWeight"].asDouble();

    priorityWeightsCheckedTime = currentTime;
    displayConsoleData("priority requests weights are updated");
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

/*
    - The following method creates Json string message which will send to Time-Phase-Diagram-Tool for generating diagrams if require.
*/
string PriorityRequestSolver::getTimePhaseDiagramMessageString()
{
    std::string jsonString{};
    Json::Value jsonObject;
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";

    jsonObject["MsgType"] = "TimePhaseDiagram";
    jsonObject["OptimalSolutionStatus"] = optimalSolutionStatus;

    jsonString = Json::writeString(builder, jsonObject);

    return jsonString;
}

/*
    -Check whether static traffic signal timing plan is available or not
*/
bool PriorityRequestSolver::checkTrafficSignalTimingPlanStatus()
{
    bool trafficSignalTimingPlanStatus{false};

    if (!trafficSignalPlan.empty())
        trafficSignalTimingPlanStatus = true;

    return trafficSignalTimingPlanStatus;
}

/*
    -Check whether active signal coordination timing plan is available or not
*/
bool PriorityRequestSolver::checkSignalCoordinationTimingPlanStatus()
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
    -Check whether to log data or not
*/
void PriorityRequestSolver::readConfigFile()
{
    string intersectionName{};
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader *reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
    delete reader;

    time_t now = time(0);
    struct tm tstruct;
    char logFileOpenningTime[80];
    tstruct = *localtime(&now);
    strftime(logFileOpenningTime, sizeof(logFileOpenningTime), "%m%d%Y_%H%M%S", &tstruct);

    logging = jsonObject["Logging"].asBool();
    consoleOutput = jsonObject["ConsoleOutput"].asBool();
    intersectionName = jsonObject["IntersectionName"].asString();
    logFileName = "/nojournal/bin/log/" + intersectionName + "_prsolverLog_" + logFileOpenningTime + ".log";

    if (logging)
    {
        double timeStamp = getPosixTimestamp();
        logFile.open(logFileName);
        logFile << "[" << fixed << showpoint << setprecision(4) << timeStamp << "] [" << getVerboseTimestamp() << "] Open PRSolver log file " << intersectionName << " intersection" << endl;
    }
}

/*
	- Method for logging data in a file
*/
void PriorityRequestSolver::loggingData(string logString)
{
    double timestamp = getPosixTimestamp();

    if (logging)
    {
        logFile << "\n[" << fixed << showpoint << setprecision(4) << timestamp << "] [" << getVerboseTimestamp() << "] ";
        logFile << logString << endl;
    }
}

/*
	- Method for displaying console output
*/
void PriorityRequestSolver::displayConsoleData(string consoleString)
{
    double timestamp = getPosixTimestamp();

    if (consoleOutput)
    {
        cout << "\n[" << fixed << showpoint << setprecision(4) << timestamp << "] [" << getVerboseTimestamp() << "] ";
        cout << consoleString << endl;
    }
}

/*
    - Loggers to log priority request string, signal status string, OptimizationModelData.dat and OptimizationResults.txt files
*/
void PriorityRequestSolver::loggingOptimizationData()
{
    ifstream infile;

    if (logging)
    {
        double timeStamp = getPosixTimestamp();

        logFile << "\n[" << fixed << showpoint << setprecision(4) << timeStamp << "] [" << getVerboseTimestamp() << "] Current optimization data file is following:\n\n";
        infile.open("/nojournal/bin/OptimizationModelData.dat");
        for (string line; getline(infile, line);)
            logFile << line << endl;
        infile.close();

        logFile << "\n[" << fixed << showpoint << setprecision(4) << timeStamp << "] [" << getVerboseTimestamp() << "] Current optimization results file is following:\n\n";
        infile.open("/nojournal/bin/OptimizationResults.txt");
        for (std::string line; getline(infile, line);)
            logFile << line << endl;
        infile.close();
    }
}

PriorityRequestSolver::~PriorityRequestSolver()
{
    logFile.close();
}