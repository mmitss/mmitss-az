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
  1. 
*/

#include "PriorityRequestSolver.h"
#include "glpk.h"
#include <fstream>
#include <stdio.h>
#include <sys/time.h>
#include "json/json.h"
#include <algorithm>
#include <cmath>
#include <UdpSocket.h>

PriorityRequestSolver::PriorityRequestSolver()
{
}

/*
	- Method for identifying the message type.
*/
int PriorityRequestSolver::getMessageType(string jsonString)
{
    int messageType{};
    Json::Value jsonObject;
    Json::Reader reader;
    reader.parse(jsonString.c_str(), jsonObject);

    if ((jsonObject["MsgType"]).asString() == "PriorityRequest")
    {
        messageType = static_cast<int>(msgType::priorityRequest);
    }

    else if ((jsonObject["MsgType"]).asString() == "ClearRequest")
    {
        messageType = static_cast<int>(msgType::clearRequest);
    }

    else if ((jsonObject["MsgType"]).asString() == "CurrNextPhaseStatus")
    {
        messageType = static_cast<int>(msgType::currentPhaseStatus);
    }

    else if ((jsonObject["MsgType"]).asString() == "ActiveTimingPlan")
    {
        messageType = static_cast<int>(msgType::signalPlan);
    }

    else
        std::cout << "Message type is unknown" << std::endl;

    return messageType;
}

/*
    - This method is responsible for creating priority request list received from the PRS as Json String.
*/
void PriorityRequestSolver::createPriorityRequestList(string jsonString)
{
    int noOfRequest{};
    RequestList requestList;
    Json::Value jsonObject;
    Json::Reader reader;

    priorityRequestList.clear();
    // loggingPRSData(jsonString);

    reader.parse(jsonString.c_str(), jsonObject);
    noOfRequest = (jsonObject["PriorityRequestList"]["noOfRequest"]).asInt();
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

    //This is optional. For priniting few attributes of the priority request list in the console
    // for (size_t i = 0; i < priorityRequestList.size(); i++)
    // {
    //     cout << priorityRequestList[i].vehicleID << " " << priorityRequestList[i].basicVehicleRole << " " << priorityRequestList[i].vehicleETA << endl;
    // }
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
    vector<int> requestedEV_P11;
    vector<int> requestedEV_P12;
    vector<int> requestedEV_P21;
    vector<int> requestedEV_P22;

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

    if (noOfEVInList > 0 && requestedSignalGroup.size() <= 2)
    {

        for (size_t j = 0; j < trafficControllerStatus.size(); j++)
        {

            if (trafficControllerStatus[j].startingPhase1 == 1 || trafficControllerStatus[j].startingPhase1 == 2)
                requestedEV_P11.push_back(trafficControllerStatus[j].startingPhase1);

            else if (trafficControllerStatus[j].startingPhase1 == 3 || trafficControllerStatus[j].startingPhase1 == 4)
                requestedEV_P12.push_back(trafficControllerStatus[j].startingPhase1);

            else if (trafficControllerStatus[j].startingPhase1 == 5 || trafficControllerStatus[j].startingPhase1 == 6)
                requestedEV_P21.push_back(trafficControllerStatus[j].startingPhase1);

            else if (trafficControllerStatus[j].startingPhase1 == 7 || trafficControllerStatus[j].startingPhase1 == 8)
                requestedEV_P22.push_back(trafficControllerStatus[j].startingPhase1);

            if (trafficControllerStatus[j].startingPhase2 == 1 || trafficControllerStatus[j].startingPhase2 == 2)
                requestedEV_P11.push_back(trafficControllerStatus[j].startingPhase2);

            else if (trafficControllerStatus[j].startingPhase2 == 3 || trafficControllerStatus[j].startingPhase2 == 4)
                requestedEV_P12.push_back(trafficControllerStatus[j].startingPhase2);

            else if (trafficControllerStatus[j].startingPhase2 == 5 || trafficControllerStatus[j].startingPhase2 == 6)
                requestedEV_P21.push_back(trafficControllerStatus[j].startingPhase2);

            else if (trafficControllerStatus[j].startingPhase2 == 7 || trafficControllerStatus[j].startingPhase2 == 8)
                requestedEV_P22.push_back(trafficControllerStatus[j].startingPhase2);
        }

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
        createDilemmaZoneRequestList();
        modifyPriorityRequestList();
        getRequestedSignalGroup();
        deleteSplitPhasesFromPriorityRequestList();
        getEVPhases();
        getEVTrafficSignalPlan();
        generateEVModFile();
        SolverDataManager solverDataManager(dilemmaZoneRequestList, priorityRequestList, trafficControllerStatus, trafficSignalPlan_EV);
        solverDataManager.generateDatFile(emergencyVehicleStatus);
    }

    else
    {
        SolverDataManager solverDataManager(dilemmaZoneRequestList, priorityRequestList, trafficControllerStatus, trafficSignalPlan);
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

    char modFile[128] = "/nojournal/bin/NewModel.mod";
    glp_prob *mip;
    glp_tran *tran;
    int ret{};
    int success = 1;
    if (emergencyVehicleStatus == true)
        strcpy(modFile, "/nojournal/bin/NewModel_EV.mod");

    mip = glp_create_prob();
    tran = glp_mpl_alloc_wksp();

    ret = glp_mpl_read_model(tran, modFile, 1);

    if (ret != 0)
    {
        fprintf(stderr, "Error on translating model\n");
        goto skip;
    }

    ret = glp_mpl_read_data(tran, "/nojournal/bin/NewModelData.dat");

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
    endOfSolve = GetSeconds();
    cout << "Success=" << success << endl;
    cout << "Time of Solve" << endOfSolve - startOfSolve << endl;
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
    setOptimizationInput();
    GLPKSolver();

    if (emergencyVehicleStatus == true)
    {
        ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan_EV, emergencyVehicleStatus);

        scheduleManager.obtainRequiredSignalGroup();
        scheduleManager.readOptimalSignalPlan();
        scheduleManager.createEventList();
        scheduleJsonString = scheduleManager.createScheduleJsonString();
    }

    else
    {
        ScheduleManager scheduleManager(priorityRequestList, trafficControllerStatus, trafficSignalPlan, emergencyVehicleStatus);

        scheduleManager.obtainRequiredSignalGroup();
        scheduleManager.readOptimalSignalPlan();
        scheduleManager.createEventList();
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

    ScheduleManager scheduleManager;
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
        {
            temporaryPhaseNumber.erase(it);
        }
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
}

/*
    - To get the current phase status a request message has to send to the TCI
    - This method is responsible for creating that message as a JSON string. 
*/
string PriorityRequestSolver::getCurrentSignalStatusRequestString()
{
    string currentPhaseStatusRequestJsonString{};
    Json::FastWriter fastWriter;
    Json::Value jsonObject;

    jsonObject["MsgType"] = "CurrNextPhaseRequest";
    currentPhaseStatusRequestJsonString = fastWriter.write(jsonObject);

    return currentPhaseStatusRequestJsonString;
}

/*
    - If new priority request is received this method will obtain the current traffic signal Status.
    - If the current phase is in yellow or red state in the json string, the method set next phase as starting phase.
    - If the current phase is in yellow or red state, the method calculates the init (time to start starting phase) value.
    - If red clearance time for both phases are not same, one phase can be in red rest. In that case init time can be negative. The method sets init time same as the init time of other starting phase..
    - If starting phase is on rest or elapsed green time is more than gmax, the method sets the elapsed green time min green time.
*/
void PriorityRequestSolver::getCurrentSignalStatus(string receivedJsonString)
{
    int temporaryPhase{};
    int temporaryCurrentPhase{};
    int temporaryNextPhase{};
    string temporaryPhaseState{};
    double temporaryElaspedTime{};

    TrafficControllerData::TrafficConrtollerStatus tcStatus;
    trafficControllerStatus.clear();

    Json::Value jsonObject_PhaseStatus;
    Json::Reader reader_PhaseStatus;
    reader_PhaseStatus.parse(receivedJsonString.c_str(), jsonObject_PhaseStatus);
    const Json::Value values = jsonObject_PhaseStatus["currentPhases"];

    // loggingTCIData(receivedJsonString);
    for (int i = 0; i < 2; i++)
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

        if (temporaryCurrentPhase < 5 && temporaryPhaseState == "green")
        {
            tcStatus.startingPhase1 = temporaryCurrentPhase;
            tcStatus.initPhase1 = 0.0;
            tcStatus.elapsedGreen1 = temporaryElaspedTime;
        }

        else if (temporaryCurrentPhase > 4 && temporaryPhaseState == "green")
        {
            tcStatus.startingPhase2 = temporaryCurrentPhase;
            tcStatus.initPhase2 = 0.0;
            tcStatus.elapsedGreen2 = temporaryElaspedTime;
        }

        else if (temporaryPhaseState == "yellow")
        {
            for (int k = 0; k < 2; k++)
            {
                temporaryNextPhase = (jsonObject_PhaseStatus["nextPhases"][k]).asInt();
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCurrentPhase; });
                if (temporaryNextPhase > 0 && temporaryNextPhase < 5)
                {
                    tcStatus.startingPhase1 = temporaryNextPhase;
                    tcStatus.initPhase1 = findSignalGroup->yellowChange + findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen1 = 0.0;
                }
                else if (temporaryNextPhase > 4 && temporaryNextPhase < 9)
                {
                    tcStatus.startingPhase2 = temporaryNextPhase;
                    tcStatus.initPhase2 = findSignalGroup->yellowChange + findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen2 = 0.0;
                }
            }
        }

        else if (temporaryPhaseState == "red")
        {
            for (int k = 0; k < 2; k++)
            {
                temporaryNextPhase = (jsonObject_PhaseStatus["nextPhases"][k]).asInt();
                if (temporaryCurrentPhase == temporaryNextPhase) //current phase and next phase can be same in case of T intersection.
                                                                 //Like the scenario when phase 4 is only yellow/red since phase 8 is missing. phase 2 and 6 was green before phase 4.
                                                                 //In this case current phase can be following :{"currentPhases": [{"Phase": 4, "State": "yellow", "ElapsedTime": 5}, {"Phase": 6, "State": "red", "ElapsedTime": 136}], "MsgType": "CurrNextPhaseStatus", "nextPhases": [6]}
                {
                    cout << "Current Phase and next phase is same" << endl;
                    break;
                }
                vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                          [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryCurrentPhase; });
                if (temporaryNextPhase > 0 && temporaryNextPhase < 5)
                {
                    tcStatus.startingPhase1 = temporaryNextPhase;
                    if ((findSignalGroup->redClear - temporaryElaspedTime) < 0.0) //If red clearance time for both phases are not same, One phase will be in red rest. In that case we will get negative init time.
                        tcStatus.initPhase1 = 0.5;
                    else
                        tcStatus.initPhase1 = findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen1 = 0.0;
                }
                else if (temporaryNextPhase > 4 && temporaryNextPhase < 9)
                {
                    tcStatus.startingPhase2 = temporaryNextPhase;
                    if ((findSignalGroup->redClear - temporaryElaspedTime) < 0.0)
                        tcStatus.initPhase2 = 0.5;
                    else
                        tcStatus.initPhase2 = findSignalGroup->redClear - temporaryElaspedTime;
                    tcStatus.elapsedGreen2 = 0.0;
                }
            }
        }
    }

    trafficControllerStatus.push_back(tcStatus);
    validateTrafficControllerStatus();
    // If signal phase is on rest or elapsed green time is more than gmax, then elapsed green time will be set as min green time.
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
    TrafficControllerData::TrafficSignalPlan signalPlan;

    Json::Value jsonObject;
    Json::Reader reader;

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
    reader.parse(jsonString.c_str(), jsonObject);
    const Json::Value values = jsonObject["TimingPlan"];
    noOfPhase = (jsonObject["TimingPlan"]["NoOfPhase"]).asInt();
    // cout << "Total Phase No: " << noOfPhase << endl;

    for (int i = 0; i < noOfPhase; i++)
    {
        PhaseNumber.push_back((jsonObject["TimingPlan"]["PhaseNumber"][i]).asInt());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PedWalk.push_back((jsonObject["TimingPlan"]["PedWalk"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PedClear.push_back((jsonObject["TimingPlan"]["PedClear"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        MinGreen.push_back((jsonObject["TimingPlan"]["MinGreen"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        Passage.push_back(((jsonObject["TimingPlan"]["Passage"][i]).asDouble()) / 10.0);
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        MaxGreen.push_back((jsonObject["TimingPlan"]["MaxGreen"][i]).asDouble());
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        YellowChange.push_back(((jsonObject["TimingPlan"]["YellowChange"][i]).asDouble()));
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        RedClear.push_back(((jsonObject["TimingPlan"]["RedClear"][i]).asDouble()));
    }

    for (int i = 0; i < noOfPhase; i++)
    {
        PhaseRing.push_back((jsonObject["TimingPlan"]["PhaseRing"][i]).asInt());
    }

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

    generateModFile();
    modifySignalTimingPlan();
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

void PriorityRequestSolver::modifySignalTimingPlan()
{
    int temporarySignalGroup{};
    int temporaryCompitableSignalGroup{};
    vector<TrafficControllerData::TrafficSignalPlan> temporaryTrafficSignalPlan;

    temporaryTrafficSignalPlan.insert(temporaryTrafficSignalPlan.end(), trafficSignalPlan.begin(), trafficSignalPlan.end());

    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        temporarySignalGroup = trafficSignalPlan[i].phaseNumber;

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroupOnList = std::find_if(std::begin(temporaryTrafficSignalPlan), std::end(temporaryTrafficSignalPlan),
                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporarySignalGroup; });

        if ((temporarySignalGroup % 2 == 0) && (trafficSignalPlan[i].minGreen == 0))
        {
            if (temporarySignalGroup < 5)
                temporaryCompitableSignalGroup = temporarySignalGroup + 4;
            if (temporarySignalGroup > 5)
                temporaryCompitableSignalGroup = temporarySignalGroup - 4;

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

        else if ((temporarySignalGroup % 2 != 0) && (trafficSignalPlan[i].minGreen == 0))
        {
            //temporaryTrafficSignalPlan.erase(findSignalGroupOnList);
            if (temporarySignalGroup < 5)
                temporaryCompitableSignalGroup = temporarySignalGroup + 5;
            if (temporarySignalGroup > 5)
                temporaryCompitableSignalGroup = temporarySignalGroup - 3;

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
    }
    trafficSignalPlan.clear();
    trafficSignalPlan.insert(trafficSignalPlan.end(), temporaryTrafficSignalPlan.begin(), temporaryTrafficSignalPlan.end());
}

/*
    - Method for generating Mod File for transit and truck
*/
void PriorityRequestSolver::generateModFile()
{
    ofstream FileMod;
    FileMod.open("/nojournal/bin/NewModel.mod", ios::out);
    // =================Defining the sets ======================

    if (P11.size() == 1)
        FileMod << "set P11:={" << P11[0] << "}; \n";

    else if (P11.size() == 2)
        FileMod << "set P11:={" << P11[0] << "," << P11[1] << "};  \n";

    if (P12.size() == 1)
        FileMod << "set P12:={" << P12[0] << "}; \n";

    else if (P12.size() == 2)
        FileMod << "set P12:={" << P12[0] << "," << P12[1] << "};  \n";

    if (P21.size() == 1)
        FileMod << "set P21:={" << P21[0] << "}; \n";

    else if (P21.size() == 2)
        FileMod << "set P21:={" << P21[0] << "," << P21[1] << "};  \n";

    if (P22.size() == 1)
        FileMod << "set P22:={" << P22[0] << "}; \n";

    else if (P22.size() == 2)
        FileMod << "set P22:={" << P22[0] << "," << P22[1] << "};  \n";

    FileMod << "set P:={";
    for (int i = 0; i < noOfPhase; i++)
    {
        if (i != noOfPhase - 1)
            FileMod << " " << PhaseNumber[i] << ",";
        else
            FileMod << " " << PhaseNumber[i];
    }
    FileMod << "};\n";

    FileMod << "set K  := {1..3};\n"; // Only two cycles ahead are considered in the model. But we should count the third cycle in the cycle set. Because, assume we are in the midle of cycle one. Therefore, we have cycle 1, 2 and half of cycle 3.
    FileMod << "set J  := {1..10};\n";
    FileMod << "set P2 := {1..8};\n";
    FileMod << "set T  := {1..10};\n"; // at most 10 different types of vehicle may be considered , EV are 1, Transit are 2, Trucks are 3

    FileMod << "set E:={1,2};\n";
    FileMod << "\n";
    //========================Parameters=========================

    FileMod << "param y    {p in P}, >=0,default 0;\n";
    FileMod << "param red  {p in P}, >=0,default 0;\n";
    FileMod << "param gmin {p in P}, >=0,default 0;\n";
    FileMod << "param gmax {p in P}, >=0,default 0;\n";
    FileMod << "param init1,default 0;\n";
    FileMod << "param init2,default 0;\n";
    FileMod << "param Grn1, default 0;\n";
    FileMod << "param Grn2, default 0;\n";
    FileMod << "param SP1,  integer,default 0;\n";
    FileMod << "param SP2,  integer,default 0;\n";
    FileMod << "param M:=9999,integer;\n";
    FileMod << "param Rl{p in P, j in J}, >=0,  default 0;\n";
    FileMod << "param Ru{p in P, j in J}, >=0,  default 0;\n";

    FileMod << "param PrioType { t in T}, >=0, default 0;  \n";
    FileMod << "param PrioWeigth { t in T}, >=0, default 0;  \n";
    FileMod << "param priorityType{j in J}, >=0, default 0;  \n";
    FileMod << "param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  \n";
    FileMod << "param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);\n";
    FileMod << "param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);\n";
    FileMod << "param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);\n";
    FileMod << "param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);\n";
    FileMod << "param ReqNo:=sum{p in P,j in J} active_pj[p,j];\n";

    // the following parameters added in order to consider case when the max green time in one barrier group expired but not in the other barrier group
    FileMod << "param sumOfGMax11, := sum{p in P11} (gmax[p]*coef[p,1]);\n";
    FileMod << "param sumOfGMax12, := sum{p in P12} (gmax[p]*coef[p,1]);\n";
    FileMod << "param sumOfGMax21, := sum{p in P21} (gmax[p]*coef[p,1]);\n";
    FileMod << "param sumOfGMax22, := sum{p in P22} (gmax[p]*coef[p,1]);\n";
    FileMod << "param barrier1GmaxSlack, := sumOfGMax11 - sumOfGMax21 ;\n";
    FileMod << "param barrier2GmaxSlack, := sumOfGMax12 - sumOfGMax22 ;\n";
    FileMod << "param gmaxSlack{p in P}, := (if coef[p,1]=0 then 0 else (if (p in P11) then gmax[p]*max(0,-barrier1GmaxSlack)/sumOfGMax11  else ( if (p in P21) then gmax[p]*max(0,+barrier1GmaxSlack)/sumOfGMax21  else ( if (p in P12) then gmax[p]*max(0,-barrier2GmaxSlack)/sumOfGMax12  else ( if (p in P22) then gmax[p]*max(0,barrier2GmaxSlack)/sumOfGMax22  else 0) ) ) )    ); \n";
    FileMod << "param gmaxPerRng{p in P,k in K}, := (if (k=1) then gmax[p]+gmaxSlack[p] else	gmax[p]);\n";

    FileMod << "\n";
    // ==================== VARIABLES =======================
    FileMod << "var t{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var g{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var v{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var d{p in P,j in J}, >=0;\n";
    FileMod << "var theta{p in P,j in J}, binary;\n";
    FileMod << "var ttheta{p in P,j in J}, >=0;\n";
    FileMod << "var PriorityDelay;\n";
    FileMod << "var Flex;\n";

    FileMod << "\n";

    // ===================Constraints==============================

    FileMod << "s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  \n";
    FileMod << "s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  \n";
    FileMod << "s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  \n";
    // # constraints in the same cycle in same P??
    FileMod << "s.t. Prec_11_11_c1{e in E,p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. Prec_12_12_c1{e in E,p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. Prec_21_21_c1{e in E,p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    FileMod << "s.t. Prec_22_22_c1{e in E,p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    // # constraints in the same cycle in connecting
    FileMod << "s.t. Prec_11_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
    FileMod << "s.t. Prec_11_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[2,1,e]+v[2,1,e];\n";
    FileMod << "s.t. Prec_21_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
    FileMod << "s.t. Prec_21_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[6,1,e]+v[6,1,e];\n";
    // #================ END of cycle 1======================#

    // # constraints in the same cycle in same P??
    FileMod << "s.t. Prec_11_11_c23{e in E,p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. Prec_12_12_c23{e in E,p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. Prec_21_21_c23{e in E,p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    FileMod << "s.t. Prec_22_22_c23{e in E,p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";

    // # constraints in the same cycle in connecting
    FileMod << "s.t. Prec_11_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
    FileMod << "s.t. Prec_11_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[2,k,e]+v[2,k,e];\n";
    FileMod << "s.t. Prec_21_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=5 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";
    FileMod << "s.t. Prec_21_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=9 and k>1 }:  t[p,k,e]=t[6,k,e]+v[6,k,e];\n";

    // # constraints in connecting in different cycles
    FileMod << "s.t. Prec_12_11_c23{e in E,p in P11, k in K: (card(P11)+p+1)=4 and k>1 }:    t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";
    FileMod << "s.t. Prec_22_11_c23{e in E,p in P11, k in K: (card(P11)+p+1+4)=8 and k>1 }:  t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";
    FileMod << "s.t. Prec_12_21_c23{e in E,p in P21, k in K: (card(P21)+p+1-4)=4 and k>1 }:  t[p,k,e]=t[4,k-1,e]+v[4,k-1,e];\n";
    FileMod << "s.t. Prec_22_21_c23{e in E,p in P21, k in K: (card(P21)+p+1)=8 and k>1 }:    t[p,k,e]=t[8,k-1,e]+v[8,k-1,e];\n";

    FileMod << "s.t. PhaseLen{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];\n";
    FileMod << "s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";
    FileMod << "s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";

    FileMod << "s.t. PrioDelay1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; \n";
    FileMod << "s.t. PrioDelay2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n";
    FileMod << "s.t. PrioDelay3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];\n";
    FileMod << "s.t. PrioDelay4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);\n";
    FileMod << "s.t. PrioDelay5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];\n";
    FileMod << "s.t. PrioDelay6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];\n";
    FileMod << "s.t. PrioDelay7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];\n";
    FileMod << "s.t. PrioDelay8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; \n";
    FileMod << "s.t. PrioDelay9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";

    FileMod << "s.t. Flexib: Flex= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];\n ";
    FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) )  - 0.01*Flex; \n "; // The coeficient to Flex should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points

    FileMod << "  minimize delay: PriorityDelay;     \n";
    //=============================Writing the Optimal Output into the /nojournal/bin/Results.txt file ==================================
    FileMod << "  \n";
    FileMod << "solve;  \n";
    FileMod << "  \n";
    FileMod << "printf \" \" > \"/nojournal/bin/Results.txt\";  \n";
    FileMod << "printf \"%3d  %3d \\n \",SP1, SP2 >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod << "printf \"%5.2f  %5.2f %5.2f  %5.2f \\n \",init1, init2,Grn1,Grn2 >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,1] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,2] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod << " } \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,1] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,2] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "printf \"%3d \\n \", ReqNo >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod << "  \n";
    FileMod << "for {p in P,j in J : Rl[p,j]>0}  \n";
    FileMod << " {  \n";
    FileMod << "   printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1)), Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>\"/nojournal/bin/Results.txt\";\n"; // the  term " coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1))" is used to know the request is served in which cycle. For example, aasume there is a request for phase 4. If the request is served in firsr cycle, the term will be 4, the second cycle, the term will be 14 and the third cycle, the term will be 24
    FileMod << " } \n";

    FileMod << "printf \"%5.2f \\n \", PriorityDelay + 0.01*Flex>>\"/nojournal/bin/Results.txt\"; \n";

    FileMod << "printf \"%5.2f \\n \", Flex >>\"/nojournal/bin/Results.txt\"; \n";
    FileMod << "printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    //------------- End of Print the Main body of mode----------------
    FileMod << "end;\n";
    FileMod.close();
}

/*
    -Dynamic mod file for EV. If there is multiple EV and they are requesting for different phase 
*/
void PriorityRequestSolver::generateEVModFile()
{
    int phasePosition{};
    ofstream FileMod;

    FileMod.open("/nojournal/bin/NewModel_EV.mod", ios::out);
    // =================Defining the sets ======================

    if (EV_P11.size() == 1)
        FileMod << "set P11:={" << EV_P11[0] << "}; \n";

    else if (EV_P11.size() == 2)
        FileMod << "set P11:={" << EV_P11[0] << "," << EV_P11[1] << "};  \n";

    if (EV_P12.size() == 1)
        FileMod << "set P12:={" << EV_P12[0] << "}; \n";

    else if (EV_P12.size() == 2)
        FileMod << "set P12:={" << EV_P12[0] << "," << EV_P12[1] << "};  \n";

    if (EV_P21.size() == 1)
        FileMod << "set P21:={" << EV_P21[0] << "}; \n";

    else if (EV_P21.size() == 2)
        FileMod << "set P21:={" << EV_P21[0] << "," << EV_P21[1] << "};  \n";

    if (EV_P22.size() == 1)
        FileMod << "set P22:={" << EV_P22[0] << "}; \n";

    else if (EV_P22.size() == 2)
        FileMod << "set P22:={" << EV_P22[0] << "," << EV_P22[1] << "};  \n";

    FileMod << "set P:={";
    for (size_t i = 0; i < trafficSignalPlan_EV.size(); i++)
    {
        if (i == trafficSignalPlan_EV.size() - 1)
            FileMod << trafficSignalPlan_EV[i].phaseNumber;
        else
            FileMod << trafficSignalPlan_EV[i].phaseNumber << ","
                    << " ";
    }

    FileMod << "};\n";

    FileMod << "set K  := {1..3};\n"; // Only two cycles ahead are considered in the model. But we should count the third cycle in the cycle set. Because, assume we are in the midle of cycle one. Therefore, we have cycle 1, 2 and half of cycle 3.
    FileMod << "set J  := {1..10};\n";
    FileMod << "set P2 := {1..8};\n";
    FileMod << "set T  := {1..10};\n"; // at most 10 different types of vehicle may be considered , EV are 1, Transit are 2, Trucks are 3

    FileMod << "set E:={1,2};\n";
    FileMod << "set DZ:={1,2};\n"; //For Dilemma Zone

    FileMod << "\n";
    // //========================Parameters=========================

    FileMod << "param y    {p in P}, >=0,default 0;\n";
    FileMod << "param red  {p in P}, >=0,default 0;\n";
    FileMod << "param gmin {p in P}, >=0,default 0;\n";
    FileMod << "param gmax {p in P}, >=0,default 0;\n";
    FileMod << "param init1,default 0;\n";
    FileMod << "param init2,default 0;\n";
    FileMod << "param Grn1, default 0;\n";
    FileMod << "param Grn2, default 0;\n";
    FileMod << "param SP1,  integer,default 0;\n";
    FileMod << "param SP2,  integer,default 0;\n";
    FileMod << "param M:=9999,integer;\n";
    FileMod << "param alpha:=100,integer;\n";
    FileMod << "param Rl{p in P, j in J}, >=0,  default 0;\n";
    FileMod << "param Ru{p in P, j in J}, >=0,  default 0;\n";

    // FileMod << "param cycle, :=" << dCoordinationCycle << ";\n"; //    # if we have coordination, the cycle length
    FileMod << "param cycle, :=" << 100 << ";\n";
    FileMod << "param PrioType { t in T}, >=0, default 0;  \n";
    FileMod << "param PrioWeigth { t in T}, >=0, default 0;  \n";
    FileMod << "param priorityType{j in J}, >=0, default 0;  \n";
    FileMod << "param priorityTypeWeigth{j in J, t in T}, := (if (priorityType[j]=t) then PrioWeigth[t] else 0);  \n";
    FileMod << "param active_pj{p in P, j in J}, integer, :=(if Rl[p,j]>0 then 1 else	0);\n";
    FileMod << "param coef{p in P,k in K}, integer,:=(if  (((p<SP1 and p<5) or (p<SP2 and p>4 )) and k==1) or (((p<5 and SP1<=p) or (p>4 and SP2<=p)) and k==3) then 0 else 1);\n";
    FileMod << "param PassedGrn1{p in P,k in K},:=(if ((p==SP1 and k==1))then Grn1 else 0);\n";
    FileMod << "param PassedGrn2{p in P,k in K},:=(if ((p==SP2 and k==1))then Grn2 else 0);\n";
    FileMod << "param ReqNo:=sum{p in P,j in J} active_pj[p,j];\n";
    /*************************DilemmaZone***************************/
    FileMod << "param Dl{p in P, dz in DZ}, >=0,  default 0;\n";
    FileMod << "param Du{p in P, dz in DZ}, >=0,  default 0;\n";
    FileMod << "param DilemmaZoneWeight, default 0;\n";
    // FileMod << "param DilemmaZoneExtention, default 0;\n";
    FileMod << "param active_dilemmazone_p{p in P, dz in DZ}, integer, :=(if Dl[p,dz]>0 then 1 else	0);\n";

    FileMod << "param gmaxPerRng{p in P,k in K}, := gmax[p];\n";
    FileMod << "\n";
    // ==================== VARIABLES =======================
    FileMod << "var t{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var g{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var v{p in P,k in K,e in E}, >=0;\n";
    FileMod << "var d{p in P,j in J}, >=0;\n";
    FileMod << "var theta{p in P,j in J}, binary;\n";
    FileMod << "var ttheta{p in P,j in J}, >=0;\n";
    FileMod << "var dilemmazone_d{p in P, dz in DZ}, >=0;\n";
    FileMod << "var dilemmazone_theta{p in P, dz in DZ}, binary;\n";
    FileMod << "var dilemmazone_ttheta{p in P, dz in DZ}, >=0;\n";
    FileMod << "var PriorityDelay;\n";
    FileMod << "var DilemmaZoneDelay;\n";
    FileMod << "var Flex;\n";

    FileMod << "\n";

    // ===================Constraints==============================

    FileMod << "s.t. initial{e in E,p in P:(p<SP1) or (p<SP2 and p>4)}: t[p,1,e]=0;  \n";
    FileMod << "s.t. initial1{e in E,p in P:p=SP1}: t[p,1,e]=init1;  \n";
    FileMod << "s.t. initial2{e in E,p in P:p=SP2}: t[p,1,e]=init2;  \n";
    // # constraints in the same cycle in same ring and barrier group
    if (EV_P11.size() > 1)
        FileMod << "s.t. Prec_11_11_c1{e in E,p in P11: (p+1)in P11 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P12.size() > 1)
        FileMod << "s.t. Prec_12_12_c1{e in E,p in P12: (p+1)in P12 and p>=SP1  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P21.size() > 1)
        FileMod << "s.t. Prec_21_21_c1{e in E,p in P21: (p+1)in P21 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    if (EV_P22.size() > 1)
        FileMod << "s.t. Prec_22_22_c1{e in E,p in P22: (p+1)in P22 and p>=SP2  }:  t[p+1,1,e]=t[p,1,e]+v[p,1,e];\n";
    // # constraints in the same cycle in connecting
    if (EV_P11.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. Prec_11_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[" << EV_P11[phasePosition] << ",1,e]+v[" << EV_P11[phasePosition] << ",1,e];\n";
    }

    if (EV_P11.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. Prec_11_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[" << EV_P11[phasePosition] << ",1,e]+v[" << EV_P11[phasePosition] << ",1,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. Prec_21_12_c1{e in E,p in P12: (card(P12)+p)<=5 and p>SP1  }:  t[p,1,e]=t[" << EV_P21[phasePosition] << ",1,e]+v[" << EV_P21[phasePosition] << ",1,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. Prec_21_22_c1{e in E,p in P22: (card(P22)+p)<=9 and p>SP2  }:  t[p,1,e]=t[" << EV_P21[phasePosition] << ",1,e]+v[" << EV_P21[phasePosition] << ",1,e];\n";
    }

    // #================ END of cycle 1======================#

    // # constraints in the same cycle in same ring and barrier group
    if (EV_P11.size() > 1)
        FileMod << "s.t. Prec_11_11_c23{e in E,p in P11, k in K: (p+1)in P11 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P12.size() > 1)
        FileMod << "s.t. Prec_12_12_c23{e in E,p in P12, k in K: (p+1)in P12 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P21.size() > 1)
        FileMod << "s.t. Prec_21_21_c23{e in E,p in P21, k in K: (p+1)in P21 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";
    if (EV_P22.size() > 1)
        FileMod << "s.t. Prec_22_22_c23{e in E,p in P22, k in K: (p+1)in P22 and k>1  }:  t[p+1,k,e]=t[p,k,e]+v[p,k,e];\n";

    // # constraints in the same cycle in connecting

    if (EV_P11.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. Prec_11_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=" << static_cast<int>(EV_P12.size()) + EV_P12[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P11[phasePosition] << ",k,e]+v[" << EV_P11[phasePosition] << ",k,e];\n";
    }

    if (EV_P11.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P11.size()) - 1;
        FileMod << "s.t. Prec_11_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=" << static_cast<int>(EV_P22.size()) + EV_P22[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P11[phasePosition] << ",k,e]+v[" << EV_P11[phasePosition] << ",k,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P12.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. Prec_21_12_c23{e in E,p in P12, k in K: coef[p,k]=1 and (card(P12)+p)=" << static_cast<int>(EV_P12.size()) + EV_P12[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P21[phasePosition] << ",k,e]+v[" << EV_P21[phasePosition] << ",k,e];\n";
    }

    if (EV_P21.size() > 0 && EV_P22.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P21.size()) - 1;
        FileMod << "s.t. Prec_21_22_c23{e in E,p in P22, k in K: coef[p,k]=1 and (card(P22)+p)=" << static_cast<int>(EV_P22.size()) + EV_P22[0] << " and k>1 }:  t[p,k,e]=t[" << EV_P21[phasePosition] << ",k,e]+v[" << EV_P21[phasePosition] << ",k,e];\n";
    }

    // # constraints in connecting in different cycles
    if (EV_P12.size() > 0 && EV_P11.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P12.size()) - 1;
        FileMod << "s.t. Prec_12_11_c23{e in E,p in P11, k in K: (card(P11)+p+1)=" << static_cast<int>(EV_P11.size()) + EV_P11[0] + 1 << " and k>1 }:    t[p,k,e]=t[" << EV_P12[phasePosition] << ",k-1,e]+v[" << EV_P12[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P22.size() > 0 && EV_P11.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P22.size()) - 1;
        FileMod << "s.t. Prec_22_11_c23{e in E,p in P11, k in K: (card(P11)+p+1+4)=" << static_cast<int>(EV_P11.size()) + EV_P11[0] + 1 + 4 << " and k>1 }:  t[p,k,e]=t[" << EV_P22[phasePosition] << ",k-1,e]+v[" << EV_P22[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P12.size() > 0 && EV_P21.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P12.size()) - 1;
        FileMod << "s.t. Prec_12_21_c23{e in E,p in P21, k in K: (card(P21)+p+1-4)=" << static_cast<int>(EV_P21.size()) + EV_P21[0] + 1 - 4 << " and k>1 }:  t[p,k,e]=t[" << EV_P12[phasePosition] << ",k-1,e]+v[" << EV_P12[phasePosition] << ",k-1,e];\n";
    }

    if (EV_P22.size() > 0 && EV_P21.size() > 0)
    {
        phasePosition = static_cast<int>(EV_P22.size()) - 1;
        FileMod << "s.t. Prec_22_21_c23{e in E,p in P21, k in K: (card(P21)+p+1)=" << static_cast<int>(EV_P21.size()) + EV_P21[0] + 1 << " and k>1 }:    t[p,k,e]=t[" << EV_P22[phasePosition] << ",k-1,e]+v[" << EV_P22[phasePosition] << ",k-1,e];\n";
    }

    FileMod << "s.t. PhaseLen{e in E,p in P, k in K}:  v[p,k,e]=(g[p,k,e]+y[p]+red[p])*coef[p,k];\n";
    FileMod << "s.t. GrnMax{e in E,p in P ,k in K}:  g[p,k,e]<=(gmaxPerRng[p,k]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";
    FileMod << "s.t. GrnMin{e in E,p in P ,k in K}:  g[p,k,e]>=(gmin[p]-PassedGrn1[p,k]-PassedGrn2[p,k])*coef[p,k]; \n";

    FileMod << "s.t. PrioDelay1{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Rl[p,j]; \n";
    FileMod << "s.t. PrioDelay2{e in E,p in P,j in J: active_pj[p,j]>0}:    M*theta[p,j]>=Ru[p,j]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n";
    FileMod << "s.t. PrioDelay3{e in E,p in P,j in J: active_pj[p,j]>0}:    d[p,j]>= ttheta[p,j]-Rl[p,j]*theta[p,j];\n";
    FileMod << "s.t. PrioDelay4{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Ru[p,j]-Rl[p,j])*(1-theta[p,j]);\n";
    FileMod << "s.t. PrioDelay5{e in E,p in P,j in J: active_pj[p,j]>0}:    ttheta[p,j]<=M*theta[p,j];\n";
    FileMod << "s.t. PrioDelay6{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-theta[p,j])<=ttheta[p,j];\n";
    FileMod << "s.t. PrioDelay7{e in E,p in P,j in J: active_pj[p,j]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-theta[p,j])>=ttheta[p,j];\n";
    FileMod << "s.t. PrioDelay8{e in E,p in P,j in J: active_pj[p,j]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Ru[p,j]-Rl[p,j])*theta[p,j]; \n";
    FileMod << "s.t. PrioDelay9{e in E,p in P,j in J: active_pj[p,j]>0}:    Ru[p,j]*theta[p,j] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";

    FileMod << "s.t. Flexib: Flex= sum{p in P,k in K} (t[p,k,2]-t[p,k,1])*coef[p,k];\n ";
    FileMod << "s.t. RD: PriorityDelay=( sum{p in P,j in J, tt in T} (priorityTypeWeigth[j,tt]*active_pj[p,j]*d[p,j] ) )  - 0.01*Flex; \n "; // The coeficient to Flex should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points

    /****************************************DilemmaZone Constraints ************************************/
    FileMod << "s.t. DilemmaZoneDelay1{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    dilemmazone_d[p,dz]>=(t[p,1,e]*coef[p,1]+t[p,2,e]*(1-coef[p,1]))-Dl[p,dz]; \n";
    FileMod << "s.t. DilemmaZoneDelay2{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    M*dilemmazone_theta[p,dz]>=Du[p,dz]-((t[p,1,e]+g[p,1,e])*coef[p,1]+(t[p,2,e]+g[p,2,e])*(1-coef[p,1]));\n";
    FileMod << "s.t. DilemmaZoneDelay3{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    dilemmazone_d[p,dz]>= dilemmazone_ttheta[p,dz]-Dl[p,dz]*dilemmazone_theta[p,dz];\n";
    FileMod << "s.t. DilemmaZoneDelay4{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    g[p,1,e]*coef[p,1]+g[p,2,e]*(1-coef[p,1])>= (Du[p,dz]-Dl[p,dz])*(1-dilemmazone_theta[p,dz]);\n";
    FileMod << "s.t. DilemmaZoneDelay5{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    dilemmazone_ttheta[p,dz]<=M*dilemmazone_theta[p,dz];\n";
    FileMod << "s.t. DilemmaZoneDelay6{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))-M*(1-dilemmazone_theta[p,dz])<=dilemmazone_ttheta[p,dz];\n";
    FileMod << "s.t. DilemmaZoneDelay7{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    (t[p,2,e]*coef[p,1]+t[p,3,e]*(1-coef[p,1]))+M*(1-dilemmazone_theta[p,dz])>=dilemmazone_ttheta[p,dz];\n";
    FileMod << "s.t. DilemmaZoneDelay8{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    g[p,2,e]*coef[p,1]+g[p,3,e]*(1-coef[p,1])>=(Du[p,dz]-Dl[p,dz])*dilemmazone_theta[p,dz]; \n";
    FileMod << "s.t. DilemmaZoneDelay9{e in E,p in P, dz in DZ: active_dilemmazone_p[p,dz]>0}:    Du[p,dz]*dilemmazone_theta[p,dz] <= (t[p,2,e]+g[p,2,e])*coef[p,1]+(t[p,3,e]+g[p,3,e])*(1-coef[p,1]) ; \n";

    FileMod << "s.t. DD: DilemmaZoneDelay=( sum{p in P, dz in DZ} (DilemmaZoneWeight*active_dilemmazone_p[p,dz]*dilemmazone_d[p,dz] ) )  - 0.01*Flex; \n"; // The coeficient to Flex should be small. Even with this small coeficient, the optimzation tried to open up flexibility for actuation between the left Critical Points and right Critical Points
                                                                                                                                                           /***************************************************************************************************/

    FileMod << "  minimize delay: PriorityDelay + DilemmaZoneDelay;     \n";

    //=============================Writing the Optimal Output into the /nojournal/bin/Results.txt file ==================================
    FileMod << "  \n";
    FileMod << "solve;  \n";
    FileMod << "  \n";
    FileMod << "printf \" \" > \"/nojournal/bin/Results.txt\";  \n";
    FileMod << "printf \"%3d  %3d \\n \",SP1, SP2 >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod << "printf \"%5.2f  %5.2f %5.2f  %5.2f \\n \",init1, init2,Grn1,Grn2 >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,1] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then v[p,k,2] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod << " } \n";
    FileMod << " \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,1] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "for {k in K}   \n";
    FileMod << " { \n";
    FileMod << "     for {p in P2} \n";
    FileMod << "        { \n";
    FileMod << "           printf \"%5.2f  \", if(p in P)  then g[p,k,2] else 0  >>\"/nojournal/bin/Results.txt\";   \n";
    FileMod << "        } \n";
    FileMod << "        printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    FileMod << " } \n";
    FileMod << "  \n";
    FileMod << "printf \"%3d \\n \", ReqNo >>\"/nojournal/bin/Results.txt\";  \n";
    FileMod << "  \n";
    FileMod << "for {p in P,j in J : Rl[p,j]>0}  \n";
    FileMod << " {  \n";
    FileMod << "   printf \"%d  %5.2f  %5.2f  %5.2f %d \\n \", p, Rl[p,j],Ru[p,j], d[p,j] , priorityType[j] >>\"/nojournal/bin/Results.txt\";\n"; // the  term " coef[p,1]*(p+10*(theta[p,j]))+(1-coef[p,1])*(p+10*(theta[p,j]+1))" is used to know the request is served in which cycle. For example, aasume there is a request for phase 4. If the request is served in firsr cycle, the term will be 4, the second cycle, the term will be 14 and the third cycle, the term will be 24
    FileMod << " } \n";

    FileMod << "printf \"%5.2f \\n \", PriorityDelay + 0.01*Flex>>\"/nojournal/bin/Results.txt\"; \n";

    FileMod << "printf \"%5.2f \\n \", Flex >>\"/nojournal/bin/Results.txt\"; \n";
    FileMod << "printf \" \\n \">>\"/nojournal/bin/Results.txt\";\n";
    //------------- End of Print the Main body of mode----------------
    FileMod << "end;\n";
    FileMod.close();
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
            if (priorityRequestList[i].vehicleType == 2)
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
    - PRSolver obtains the signal timing plan from the TCI. At the start of the program it asks TCI to send the static signal timing plan 
    - This method creats that signal timing plan request string.
*/
string PriorityRequestSolver::getSignalTimingPlanRequestString()
{
    std::string jsonString{};
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    jsonObject["MsgType"] = "TimingPlanRequest";
    jsonString = fastWriter.write(jsonObject);

    return jsonString;
}

double PriorityRequestSolver::GetSeconds()
{
    struct timeval tv_tt;
    gettimeofday(&tv_tt, NULL);
    return (static_cast<double>(tv_tt.tv_sec) + static_cast<double>(tv_tt.tv_usec) / 1.e6);
}

/*
    -Check whether to log data or not
*/
bool PriorityRequestSolver::logging()
{
    string logging{};
    ofstream outputfile;
    Json::Value jsonObject;
    Json::Reader reader;
    ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

    string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);
    logging = (jsonObject["Logging"]).asString();

    if (logging == "True")
    {
        loggingStatus = true;
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        outputfile.open("/nojournal/bin/log/PRSolverLog.txt");
        outputfile << "File opened at time : " << timenow << std::endl;
        outputfile.close();
    }
    else
        loggingStatus = false;

    return loggingStatus;
}

/*
    - Loggers to log priority request string, signal status string, NewModelData.dat and results.txt files
*/
void PriorityRequestSolver::loggingOptimizationData(string priorityRequestString, string signalStatusString, string scheduleString)
{
    ofstream outputfile;
    ifstream infile;

    if (loggingStatus == true)
    {
        // outputfile.open("/nojournal/bin/log/PRSolver_Log" + std::to_string(timenow) + ".txt");
        outputfile.open("/nojournal/bin/log/PRSolverLog.txt", std::ios_base::app);
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        outputfile << "\nFollowing Priority Request is received from PRS at time " << timenow << endl;
        outputfile << priorityRequestString << endl;

        outputfile << "\nFollowing Signal Status is received from TCI at time " << timenow << endl;
        outputfile << signalStatusString << endl;

        outputfile << "\nCurrent Dat File at time : " << timenow << endl;
        infile.open("/nojournal/bin/NewModelData.dat");
        for (string line; getline(infile, line);)
        {
            outputfile << line << endl;
        }
        infile.close();

        outputfile << "\nCurrent Results File at time : " << timenow << endl;
        infile.open("/nojournal/bin/Results.txt");
        for (std::string line; getline(infile, line);)
        {
            outputfile << line << endl;
        }
        infile.close();

        outputfile << "\nFollowing Schedule will send to TCI at time " << timenow << endl;
        outputfile << scheduleString << endl;

        outputfile.close();
    }
}

// void PriorityRequestSolver::loggingData(string jsonString)
// {
//     ofstream outputfile;
//     ifstream infile;

//     if (loggingStatus == true)
//     {
//         // outputfile.open("/nojournal/bin/log/PRSolver_Log" + std::to_string(timenow) + ".txt");
//         outputfile.open("/nojournal/bin/log/PRSolverLog.txt", std::ios_base::app);
//         auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

//         outputfile << "\nCurrent Dat File at time : " << timenow << endl;
//         infile.open("/nojournal/bin/NewModelData.dat");
//         for (string line; getline(infile, line);)
//         {
//             outputfile << line << endl;
//         }
//         infile.close();

//         outputfile << "\nCurrent Results File at time : " << timenow << endl;
//         infile.open("/nojournal/bin/Results.txt");
//         for (std::string line; getline(infile, line);)
//         {
//             outputfile << line << endl;
//         }
//         infile.close();

//         outputfile << "\nFollowing Schedule will send to TCI at time " << timenow << endl;
//         outputfile << jsonString << endl;

//         outputfile.close();
//     }
// }

/*
    - Loggers to log static signal timing plan data
*/
void PriorityRequestSolver::loggingSignalPlanData(string jsonString)
{
    if (loggingStatus == true)
    {
        ofstream outputfile;
        outputfile.open("/nojournal/bin/log/PRSolverLog.txt", std::ios_base::app);
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        outputfile << "\nFollowing Signal Plan is received from TCI at time " << timenow << endl;
        outputfile << jsonString << endl;
        outputfile.close();
    }
}

// void PriorityRequestSolver::loggingPRSData(string jsonString)
// {
//     if (loggingStatus == true)
//     {
//         ofstream outputfile;
//         outputfile.open("/nojournal/bin/log/PRSolverLog.txt", std::ios_base::app);
//         auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

//         outputfile << "\nFollowing data is received from PRS at time " << timenow << endl;
//         outputfile << jsonString << endl;
//         outputfile.close();
//     }
// }

/*
    - Loggers to log clear request string
*/
void PriorityRequestSolver::loggingClearRequestData(string jsonString)
{
    if (loggingStatus == true)
    {
        ofstream outputfile;
        outputfile.open("/nojournal/bin/log/PRSolverLog.txt", std::ios_base::app);
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        outputfile << "\nFollowing Clear Request is sent to TCI at time " << timenow << endl;
        outputfile << jsonString << endl;
        outputfile.close();
    }
}

PriorityRequestSolver::~PriorityRequestSolver()
{
}