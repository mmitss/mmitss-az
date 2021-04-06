/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  SolverDataManager.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script manage the data required for solving the optimization model.
  2. This script writes the dat file required for solving the optimization model.
*/
#include <algorithm>
#include "SolverDataManager.h"
#include "msgEnum.h"

const double MAXGREEN = 100.0;

SolverDataManager::SolverDataManager()
{
}

SolverDataManager::SolverDataManager(vector<RequestList> requestList)
{
    if (!requestList.empty())
        priorityRequestList = requestList;
}

SolverDataManager::SolverDataManager(vector<RequestList> dilemmaZoneList, vector<RequestList> requestList, vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus, vector<TrafficControllerData::TrafficSignalPlan> signalPlan, double EV_Weight, double EV_SplitPhase_Weight, double Transit_Weight, double Truck_Weight, double DZ_Request_Weight, double Coordination_Weight)
{
    if (!requestList.empty())
        priorityRequestList = requestList;

    if (!signalStatus.empty())
        trafficControllerStatus = signalStatus;

    if (!signalPlan.empty())
        trafficSignalPlan = signalPlan;

    if (!dilemmaZoneList.empty())
        dilemmaZoneRequestList = dilemmaZoneList;

    EmergencyVehicleWeight = EV_Weight; 
    EmergencyVehicleSplitPhaseWeight = EV_SplitPhase_Weight;
    TransitWeight = Transit_Weight; 
    TruckWeight = Truck_Weight; 
    DilemmaZoneRequestWeight = DZ_Request_Weight;
    CoordinationWeight = Coordination_Weight;
}

/*
    - This method will obtain requested signal group information from the priority request list and store them in requestedSignalGroup list.
    - If the request is for signal coordination, it will not be added in the list. 
    - The list will be used to increase the gmax by 15% for transit and truck priority requests. If gmax is stretched for coordination, coordinated phases remain green even after the splits.
*/
vector<int> SolverDataManager::getRequestedSignalGroupFromPriorityRequestList()
{
    for (size_t i = 0; i < priorityRequestList.size(); i++)
    {
        if (priorityRequestList[i].vehicleType != SignalCoordinationVehicleType)
            requestedSignalGroup.push_back(priorityRequestList[i].requestedPhase);
    }
    removeDuplicateSignalGroup();

    return requestedSignalGroup;
}

/*
    - This method is responsible for removing the duplicate signal group number from requestedSignalGroup list.
        -If there is multiple priority request for same signal group then there will be duplicate signal group in requestedSignalGroup list.
*/
void SolverDataManager::removeDuplicateSignalGroup()
{
    auto end = requestedSignalGroup.end();
    for (auto it = requestedSignalGroup.begin(); it != end; ++it)
        end = std::remove(it + 1, end, *it);

    requestedSignalGroup.erase(end, requestedSignalGroup.end());
}

/*
    - This function is responsible for finding associated signal group from another ring for requested signal group
    - At first all requested signal group information are stored in another temporary vector
    - Associated signal group is obtained by +/- 4. If requested phase is in ring 1 add 4. If equested phase is in ring 2 substract 4.
    - Check if the associated signal group is enabled for the intersection
    - Append associated signal group information in the orignal signal group list.
    - Remove the duplicate phase number
*/
void SolverDataManager::addAssociatedSignalGroup()
{
    vector<int> tempListOfRequestedSignalGroup = requestedSignalGroup;
    int associatedSignalGroup{};
    int tempRequestedSignalGroup{};

    for (auto i = requestedSignalGroup.begin(); i != requestedSignalGroup.end(); ++i)
    {
        tempRequestedSignalGroup = *i;
        if (tempRequestedSignalGroup < 5)
        {
            associatedSignalGroup = tempRequestedSignalGroup + 4;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                      [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == associatedSignalGroup; });
            if (findSignalGroup != trafficSignalPlan.end())
                tempListOfRequestedSignalGroup.push_back(associatedSignalGroup);
        }
        else
        {
            associatedSignalGroup = tempRequestedSignalGroup - 4;
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                      [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == associatedSignalGroup; });
            if (findSignalGroup != trafficSignalPlan.end())
                tempListOfRequestedSignalGroup.push_back(associatedSignalGroup);
        }
    }
    requestedSignalGroup = tempListOfRequestedSignalGroup;
    removeDuplicateSignalGroup();
}

/*
    - This function will increase the  value of green max by 15% if there is Transit or Truck in the priority request list.
*/
void SolverDataManager::modifyGreenMax()
{
    for (auto i = requestedSignalGroup.begin(); i != requestedSignalGroup.end(); ++i)
    {
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                  [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == *i; });
        if (findSignalGroup != trafficSignalPlan.end())
            findSignalGroup->maxGreen = findSignalGroup->maxGreen * 1.15;
    }
}

/*
    -If signal phase is on rest or elapsed green time is more than gmax, then elapsed green time will be set as min green time.
*/
void SolverDataManager::modifyCurrentSignalStatus()
{
    int temporaryPhase{};
    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        temporaryPhase = trafficControllerStatus[i].startingPhase1;

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                   [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
        if (findSignalGroupInList(temporaryPhase) == true)
        {
            if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->minGreen)
                trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->minGreen;
        }

        else
        {
            if (trafficControllerStatus[i].elapsedGreen1 > findSignalGroup1->maxGreen - findSignalGroup1->minGreen)
                trafficControllerStatus[i].elapsedGreen1 = findSignalGroup1->maxGreen - findSignalGroup1->minGreen;
        }

        temporaryPhase = trafficControllerStatus[i].startingPhase2;

        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                   [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
        if (findSignalGroupInList(temporaryPhase) == true)
        {
            if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->minGreen)
                trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->minGreen;
        }
        else
        {
            if (trafficControllerStatus[i].elapsedGreen2 > findSignalGroup2->maxGreen - findSignalGroup2->minGreen)
                trafficControllerStatus[i].elapsedGreen2 = findSignalGroup2->maxGreen - findSignalGroup2->minGreen;
        }
    }
}

/*
    - If emergency vehicle sends priority request, this method checks whether signal group from signal plan is in the priority request list or not
*/
bool SolverDataManager::findSignalGroupInList(int signalGroup)
{
    bool findSignalGroup{false};
    for (size_t j = 0; j < priorityRequestList.size(); j++)
    {
        vector<RequestList>::iterator findSignalGroupInRequestList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
                                                                                  [&](RequestList const &p) { return p.requestedPhase == signalGroup; });

        if (findSignalGroupInRequestList != priorityRequestList.end())
            findSignalGroup = true;
        else
            findSignalGroup = false;
    }

    return findSignalGroup;
}

/*
    - This function is responsible for creating Data file for glpk Solver based on priority request list and TCI data.
*/
void SolverDataManager::generateDatFile(bool emergencyVehicleStatus)
{
    vector<int>::iterator it;
    int vehicleClass{};
    int coordinationVehicleType = 20;
    int numberOfRequest{};
    int numberOfCoordinationRequest{};
    int ReqSeq = 1;
    int dilemmaZoneReq = 1;
    double ETA_Range{};
    ofstream fs;

    fs.open("/nojournal/bin/OptimizationModelData.dat", ios::out);
    fs << "data;\n";

    for (size_t i = 0; i < trafficControllerStatus.size(); i++)
    {
        fs << "param SP1:=" << trafficControllerStatus[i].startingPhase1 << ";" << endl;
        fs << "param SP2:=" << trafficControllerStatus[i].startingPhase2 << ";" << endl;
        fs << "param init1:=" << trafficControllerStatus[i].initPhase1 << ";" << endl;
        fs << "param init2:=" << trafficControllerStatus[i].initPhase2 << ";" << endl;
        fs << "param Grn1 :=" << trafficControllerStatus[i].elapsedGreen1 << ";" << endl;
        fs << "param Grn2 :=" << trafficControllerStatus[i].elapsedGreen2 << ";" << endl;
    }

    fs << "param y          \t:=";
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
        fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].yellowChange;
    fs << ";\n";

    fs << "param red          \t:=";
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
        fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].redClear;
    fs << ";\n";

    fs << "param gmin      \t:=";
    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
        fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].minGreen;
    fs << ";\n";

    fs << "param gmax      \t:=";
    if (emergencyVehicleStatus == true)
    {
        for (size_t i = 0; i < trafficSignalPlan.size(); i++)
            fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << MAXGREEN;
    }
    
    else
    {
        for (size_t i = 0; i < trafficSignalPlan.size(); i++)
            fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].maxGreen;
    }
    fs << ";\n";

    fs << "param priorityType:= ";

    if (!priorityRequestList.empty() && dilemmaZoneRequestList.empty())
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            vehicleClass = 0;
            numberOfRequest++;

            if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::special))
            {
                numberOfEVInList++;
                vehicleClass = VehicleClass_EmergencyVehicle;
            }

            else if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::bus))
            {
                numberOfTransitInList++;
                vehicleClass = VehicleClass_Transit;
            }

            else if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::axleCnt4))
            {
                numberOfTruckInList++;
                vehicleClass = VehicleClass_Truck;
            }

            else if (priorityRequestList[i].vehicleType == coordinationVehicleType)
            {
                numberOfCoordinationRequest++;
                vehicleClass = VehicleClass_Coordination;
            }

            fs << numberOfRequest;
            fs << " " << vehicleClass << " ";
        }
        while (numberOfRequest < 20)
        {
            numberOfRequest++;
            fs << numberOfRequest;
            fs << " ";
            fs << 0;
            fs << " ";
        }
        fs << " ;  \n";
    }

    else if (!priorityRequestList.empty() && !dilemmaZoneRequestList.empty())
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            vehicleClass = 0;
            numberOfRequest++;

            if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::special) && priorityRequestList[i].requestedPhase % 2 == 0)
            {
                numberOfEVInList++;
                vehicleClass = VehicleClass_EmergencyVehicle;
            }

            else if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::special) && priorityRequestList[i].requestedPhase % 2 != 0)
            {
                numberOfEVSplitRequestInList++;
                vehicleClass = VehicleClass_EmergencyVehicleSplitRequest;
            }

            else if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::bus))
            {
                numberOfTransitInList++;
                vehicleClass = VehicleClass_Transit;
            }

            else if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::axleCnt4))
            {
                numberOfTruckInList++;
                vehicleClass = VehicleClass_Truck;
            }

            else if (priorityRequestList[i].vehicleType == coordinationVehicleType)
            {
                numberOfCoordinationRequest++;
                vehicleClass = VehicleClass_Coordination;
            }

            fs << numberOfRequest;
            fs << " " << vehicleClass << " ";
        }
        while (numberOfRequest < 20)
        {
            numberOfRequest++;
            fs << numberOfRequest;
            fs << " ";
            fs << 0;
            fs << " ";
        }
        fs << " ;  \n";
    }

    else
        fs << " 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0 9 0 10 0 11 0 12 0 13 0 14 0 15 0 16 0 17 0 18 0 19 0 20 0 ; \n";

    fs << "param PrioWeight:= ";

    fs << " 1 ";
    if (numberOfEVInList > 0)
        fs << EmergencyVehicleWeight;

    else
        fs << 0;

    fs << " 2 ";
    if (numberOfTransitInList > 0)
        fs << TransitWeight;

    else
        fs << 0;

    fs << " 3 ";
    if (numberOfTruckInList > 0)
        fs << TruckWeight;

    else
        fs << 0;

    fs << " 4 ";
    if (numberOfEVSplitRequestInList > 0)
        fs << EmergencyVehicleSplitPhaseWeight;

    else
        fs << 0;

    fs << " 5 ";
    if (numberOfCoordinationRequest > 0)
        fs << CoordinationWeight;

    else
        fs << 0;

    fs << " 6 0 7 0 8 0 9 0 10 0 ; \n"; 

    if (!dilemmaZoneRequestList.empty())
    {
        fs << "param DilemmaZoneWeight:= " << DilemmaZoneRequestWeight << ";\n";
        fs << "param Dl (tr): 1 2 3 4 5 6 7 8:=\n";
        for (size_t i = 0; i < dilemmaZoneRequestList.size(); i++)
        {
            fs << dilemmaZoneReq << "  ";
            for (size_t j = 1; j < 9; j++)
            {
                if (dilemmaZoneRequestList[i].requestedPhase == static_cast<int>(j))
                    fs << dilemmaZoneRequestList[i].vehicleETA << "\t";

                else
                    fs << ".\t";
            }
            dilemmaZoneReq++;
            fs << "\n";
        }
        fs << ";\n";
        dilemmaZoneReq = 1;

        fs << "param Du (tr): 1 2 3 4 5 6 7 8:=\n";
        for (size_t i = 0; i < dilemmaZoneRequestList.size(); i++)
        {
            fs << dilemmaZoneReq << "  ";
            for (size_t j = 1; j < 9; j++)
            {
                if (dilemmaZoneRequestList[i].requestedPhase == static_cast<int>(j))
                    fs << dilemmaZoneRequestList[i].vehicleETA + dilemmaZoneRequestList[i].vehicleETA_Duration << "\t";

                else
                    fs << ".\t";
            }
            dilemmaZoneReq++;
            fs << "\n";
        }
        fs << ";\n";
    }

    fs << "param Rl (tr): 1 2 3 4 5 6 7 8:=\n";

    if (!priorityRequestList.empty())
    {
        ETA_Range = 4.0;

        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            fs << ReqSeq << "  ";
            for (size_t j = 1; j < 9; j++)
            {
                if (priorityRequestList[i].requestedPhase == static_cast<int>(j) && priorityRequestList[i].vehicleType != coordinationVehicleType)
                {
                    if (priorityRequestList[i].vehicleETA <= ETA_Range + 1.0)
                        fs << 1.0 << "\t";
                    else
                        fs << priorityRequestList[i].vehicleETA - ETA_Range << "\t";
                }
                else if (priorityRequestList[i].requestedPhase == static_cast<int>(j) && priorityRequestList[i].vehicleType == coordinationVehicleType)
                    fs << priorityRequestList[i].vehicleETA << "\t";
                
                else
                    fs << ".\t";
            }
            ReqSeq++;
            fs << "\n";
        }
    }

    fs << ";\n";
    ReqSeq = 1;

    fs << "param Ru (tr): 1 2 3 4 5 6 7 8:=\n";

    if (!priorityRequestList.empty())
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            fs << ReqSeq << "  ";
            for (size_t j = 1; j < 9; j++)
            {
                if (priorityRequestList[i].requestedPhase == static_cast<int>(j))
                    fs << priorityRequestList[i].vehicleETA + priorityRequestList[i].vehicleETA_Duration << "\t";

                else
                    fs << ".\t";
            }
            ReqSeq++;
            fs << "\n";
        }
    }

    fs << ";\n";
    fs << "end;";
    fs.close();
}

SolverDataManager::~SolverDataManager()
{
}