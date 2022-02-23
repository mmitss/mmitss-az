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
#include <numeric>
#include "SolverDataManager.h"
#include "msgEnum.h"

SolverDataManager::SolverDataManager()
{
}

SolverDataManager::SolverDataManager(vector<RequestList> requestList)
{
    if (!requestList.empty())
        priorityRequestList = requestList;
}

SolverDataManager::SolverDataManager(vector<RequestList> dilemmaZoneList, vector<RequestList> requestList,
                                     vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus,
                                     vector<TrafficControllerData::TrafficSignalPlan> signalPlan, vector<int> listOfConflictingPedCall,
                                     double EV_Weight, double EV_SplitPhase_Weight, double Transit_Weight, double Truck_Weight,
                                     double DZ_Request_Weight, double Coordination_Weight)
{
    if (!requestList.empty())
        priorityRequestList = requestList;

    if (!signalStatus.empty())
        trafficControllerStatus = signalStatus;

    if (!signalPlan.empty())
        trafficSignalPlan = signalPlan;

    if (!dilemmaZoneList.empty())
        dilemmaZoneRequestList = dilemmaZoneList;

    if (!listOfConflictingPedCall.empty())
        conflictingPedCallList = listOfConflictingPedCall;

    EmergencyVehicleWeight = EV_Weight;
    EmergencyVehicleSplitPhaseWeight = EV_SplitPhase_Weight;
    TransitWeight = Transit_Weight;
    TruckWeight = Truck_Weight;
    DilemmaZoneRequestWeight = DZ_Request_Weight;
    CoordinationWeight = Coordination_Weight;
}

SolverDataManager::SolverDataManager(vector<RequestList> dilemmaZoneList, vector<RequestList> requestList,
                                     vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus,
                                     vector<TrafficControllerData::TrafficSignalPlan> signalPlan, vector<int> listOfConflictingPedCall,
                                     vector<int> requested_Signal_Group, double EV_Weight, double EV_SplitPhase_Weight, double Transit_Weight,
                                     double Truck_Weight, double DZ_Request_Weight, double Coordination_Weight)
{
    if (!requestList.empty())
        priorityRequestList = requestList;

    if (!signalStatus.empty())
        trafficControllerStatus = signalStatus;

    if (!signalPlan.empty())
        trafficSignalPlan = signalPlan;

    if (!dilemmaZoneList.empty())
        dilemmaZoneRequestList = dilemmaZoneList;

    if (!listOfConflictingPedCall.empty())
        conflictingPedCallList = listOfConflictingPedCall;

    if (!requested_Signal_Group.empty())
        requestedSignalGroup = requested_Signal_Group;

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
        if (priorityRequestList[i].vehicleType != CoordinationVehicleType)
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
void SolverDataManager::modifyGreenMax(bool emergencyVehicleStatus)
{
    int temporaryPhase{};

    for (size_t i = 0; i < requestedSignalGroup.size(); i++)
    {
        temporaryPhase = requestedSignalGroup.at(i);
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                  [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });
        if (emergencyVehicleStatus && findSignalGroup != trafficSignalPlan.end())
            findSignalGroup->maxGreen = findSignalGroup->maxGreen * 1.50;

        else if (!emergencyVehicleStatus && findSignalGroup != trafficSignalPlan.end())
            findSignalGroup->maxGreen = findSignalGroup->maxGreen * 1.15;
    }
}

/*
    - The function can modify gmin, gmax time for conflicting ped calls
        - If there is ped call on conflicting direction, the function will calculate the ped walk and ped clear time for those phases
        - If the phase gmin is less than the pedServiceTime (pedWalk and pedClear time), gmin will be set as pedServiceTime
        - If the phase gmax is less than the pedServiceTime (pedWalk and pedClear time), gmax will be set as pedServiceTime
*/
void SolverDataManager::modifyGreenTimeForConflictingPedCalls()
{
    int temporaryPhase{};
    double pedistrianServiceTime{};

    if (!conflictingPedCallList.empty())
    {
        pedCallStatus = true;
        for (size_t i = 0; i < conflictingPedCallList.size(); i++)
        {
            temporaryPhase = conflictingPedCallList.at(i);
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findConflictingPedCallSignalGroup = std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                                                                                                                        [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

            if (findConflictingPedCallSignalGroup != trafficSignalPlan.end())
            {
                pedistrianServiceTime = findConflictingPedCallSignalGroup->pedWalk + findConflictingPedCallSignalGroup->pedClear;

                if (findConflictingPedCallSignalGroup->minGreen < pedistrianServiceTime)
                    findConflictingPedCallSignalGroup->minGreen = pedistrianServiceTime;

                if (findConflictingPedCallSignalGroup->maxGreen < pedistrianServiceTime)
                    findConflictingPedCallSignalGroup->maxGreen = pedistrianServiceTime;
            }
        }
    }
}

void SolverDataManager::modifyGreenTimeForCurrentPedCalls()
{
    int temporaryPhase{};
    double pedistrianServiceTime{};

    if (trafficControllerStatus[0].currentPedCallStatus1)
    {
        pedCallStatus = true;
        temporaryPhase = trafficControllerStatus[0].startingPhase1;
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 =
            std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                         [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

        pedistrianServiceTime = findSignalGroup1->pedWalk + findSignalGroup1->pedClear;

        if (findSignalGroup1->minGreen < pedistrianServiceTime)
            findSignalGroup1->minGreen = pedistrianServiceTime;

        if (findSignalGroup1->maxGreen < pedistrianServiceTime)
            findSignalGroup1->maxGreen = pedistrianServiceTime;
    }

    if (trafficControllerStatus[0].currentPedCallStatus2)
    {
        pedCallStatus = true;
        temporaryPhase = trafficControllerStatus[0].startingPhase2;
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 =
            std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                         [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

        pedistrianServiceTime = findSignalGroup2->pedWalk + findSignalGroup2->pedClear;

        if (findSignalGroup2->minGreen < pedistrianServiceTime)
            findSignalGroup2->minGreen = pedistrianServiceTime;

        if (findSignalGroup2->maxGreen < pedistrianServiceTime)
            findSignalGroup2->maxGreen = pedistrianServiceTime;
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
void SolverDataManager::generateDatFile(bool emergencyVehicleStatus, double earlyReturnedValue1, double earlyReturnedValue2, int coordinatedPhase1, int coordinatedPhase2)
{
    vector<int>::iterator it;
    int vehicleClass{};
    int coordinationVehicleType = 20;
    int numberOfRequest{};
    int numberOfCoordinationRequest{};
    int ReqSeq = 1;
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

    if (!emergencyVehicleStatus)
    {
        fs << "param EarlyReturnValue1:=" << earlyReturnedValue1 << ";" << endl;
        fs << "param EarlyReturnValue2:=" << earlyReturnedValue2 << ";" << endl;
        fs << "param CoordinatedPhase1:=" << coordinatedPhase1 << ";" << endl;
        fs << "param CoordinatedPhase2:=" << coordinatedPhase2 << ";" << endl;
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

    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
        fs << "\t" << trafficSignalPlan[i].phaseNumber << "\t" << trafficSignalPlan[i].maxGreen;

    fs << ";\n";

    fs << "param priorityType:= ";

    if (!priorityRequestList.empty())
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            if (i < Maximum_Number_Of_Priority_Request)
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

                else if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::axleCnt4) && !priorityRequestList[i].dilemmaZoneStatus)
                {
                    numberOfTruckInList++;
                    vehicleClass = VehicleClass_Truck;
                }

                else if (priorityRequestList[i].vehicleType == static_cast<int>(MsgEnum::vehicleType::axleCnt4) && priorityRequestList[i].dilemmaZoneStatus)
                {
                    numberOfDilemmaZoneRequestInList++;
                    vehicleClass = VehicleClass_DilemmaZone;
                }

                else if (priorityRequestList[i].vehicleType == coordinationVehicleType)
                {
                    numberOfCoordinationRequest++;
                    vehicleClass = VehicleClass_Coordination;
                }

                fs << numberOfRequest;
                fs << " " << vehicleClass << " ";
            }
        }
        while (numberOfRequest < Maximum_Number_Of_Priority_Request)
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
        fs << " 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0 9 0 10 0 11 0 12 0 13 0 14 0 15 0 ; \n";

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
    if (numberOfDilemmaZoneRequestInList > 0)
        fs << DilemmaZoneRequestWeight;

    else
        fs << 0;

    fs << " 5 ";
    if (numberOfCoordinationRequest > 0)
        fs << CoordinationWeight;

    else
        fs << 0;

    fs << " 6 ";
    if (numberOfEVSplitRequestInList > 0)
        fs << EmergencyVehicleSplitPhaseWeight;

    else
        fs << 0;

    fs << " 7 0 8 0 9 0 10 0 ; \n";

    fs << "param Rl (tr): 1 2 3 4 5 6 7 8:=\n";

    if (!priorityRequestList.empty())
    {
        ETA_Range = 4.0;

        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            if (i < Maximum_Number_Of_Priority_Request)
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
    }

    fs << ";\n";
    ReqSeq = 1;

    fs << "param Ru (tr): 1 2 3 4 5 6 7 8:=\n";

    if (!priorityRequestList.empty())
    {
        for (size_t i = 0; i < priorityRequestList.size(); i++)
        {
            if (i < Maximum_Number_Of_Priority_Request)
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
    }

    fs << ";\n";
    fs << "end;";
    fs.close();
}
/*
    - The method will sorted all the phases based on their ring barrier group. EV_P11 and EV_P21 phases are in ring barrier group1 whereas, EV_P12 and EV_P22 phases are in ring barrier group2
    - Gmax value for each phases per ring barrier group will be stored in the corresponding vector
    - Largest gmax value per ring barrier group will be computed
*/
void SolverDataManager::validateGmaxForEVSignalTimingPlan(vector<int> EV_P11, vector<int> EV_P12, vector<int> EV_P21, vector<int> EV_P22)
{
    vector<int> phasesForRingBarrierGroup1{};
    vector<int> phasesForRingBarrierGroup2{};
    vector<double> gmaxForRingBarrierGroup1{};
    vector<double> gmaxForRingBarrierGroup2{};
    vector<int>::iterator it1;
    vector<int>::iterator it2;
    int temporaryPhase{};
    double largestGmaxForRingBarrierGroup1{};
    double largestGmaxForRingBarrierGroup2{};

    phasesForRingBarrierGroup1.insert(phasesForRingBarrierGroup1.end(), EV_P11.begin(), EV_P11.end());
    phasesForRingBarrierGroup2.insert(phasesForRingBarrierGroup2.end(), EV_P12.begin(), EV_P12.end());

    for (size_t i = 0; i < EV_P21.size(); i++)
        phasesForRingBarrierGroup1.push_back(EV_P21.at(i));

    for (size_t i = 0; i < EV_P22.size(); i++)
        phasesForRingBarrierGroup2.push_back(EV_P22.at(i));

    for (size_t i = 0; i < phasesForRingBarrierGroup1.size(); i++)
    {
        temporaryPhase = phasesForRingBarrierGroup1.at(i);
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup =
            std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                         [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

        gmaxForRingBarrierGroup1.push_back(findSignalGroup->maxGreen);
    }

    for (size_t i = 0; i < phasesForRingBarrierGroup2.size(); i++)
    {
        temporaryPhase = phasesForRingBarrierGroup2.at(i);
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup =
            std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                         [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

        gmaxForRingBarrierGroup2.push_back(findSignalGroup->maxGreen);
    }

    largestGmaxForRingBarrierGroup1 = *max_element(gmaxForRingBarrierGroup1.begin(), gmaxForRingBarrierGroup1.end());
    largestGmaxForRingBarrierGroup2 = *max_element(gmaxForRingBarrierGroup2.begin(), gmaxForRingBarrierGroup2.end());

    for (size_t i = 0; i < trafficSignalPlan.size(); i++)
    {
        if (trafficSignalPlan[i].minGreen > 0)
        {
            temporaryPhase = trafficSignalPlan[i].phaseNumber;

            if (std::find(phasesForRingBarrierGroup1.begin(), phasesForRingBarrierGroup1.end(), temporaryPhase) != phasesForRingBarrierGroup1.end())
                trafficSignalPlan[i].maxGreen = largestGmaxForRingBarrierGroup1;

            else if (std::find(phasesForRingBarrierGroup2.begin(), phasesForRingBarrierGroup2.end(), temporaryPhase) != phasesForRingBarrierGroup2.end())
                trafficSignalPlan[i].maxGreen = largestGmaxForRingBarrierGroup2;
        }
    }
}

/*
    - If there is a ped call for one direction, it is required to adjust the slack time. 
    - For example, {"currentPhases": [{"Phase": 4, "State": "green", "ElapsedTime": 219, "ElapsedTimeInGMax": 214, "RemainingGMax": 222, "PedState": "ped_clear"}, {"Phase": 8, "State": "green", "ElapsedTime": 69, "ElapsedTimeInGMax": 68, "RemainingGMax": 222, "PedState": "do_not_walk"}], "MsgType": "CurrNextPhaseStatus", "nextPhases": [0], "vehicleCalls": [2, 5], "totalVehicleCalls": 2, "pedestrianCalls": [2, 4], "totalPedestrianCalls": 2}
    - The following method add the slack to the through phase.
*/
void SolverDataManager::adjustGreenTimeForPedCall(vector<int> P11, vector<int> P12, vector<int> P21, vector<int> P22)
{
    double P11_GreenTime{};
    double P12_GreenTime{};
    double P21_GreenTime{};
    double P22_GreenTime{};
    int temporaryPhase{};
    if (!conflictingPedCallList.empty() || trafficControllerStatus[0].currentPedCallStatus1 || trafficControllerStatus[0].currentPedCallStatus2)
    {
        P11_GreenTime = calulateGmax(P11);
        P12_GreenTime = calulateGmax(P12);
        P21_GreenTime = calulateGmax(P21);
        P22_GreenTime = calulateGmax(P22);

        if (P11_GreenTime > P21_GreenTime)
        {
            temporaryPhase = P21.back();
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup =
                std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                             [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

            findSignalGroup->maxGreen = findSignalGroup->maxGreen + P11_GreenTime - P21_GreenTime;
        }

        else if (P21_GreenTime > P11_GreenTime)
        {
            temporaryPhase = P11.back();
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup =
                std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                             [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

            findSignalGroup->maxGreen = findSignalGroup->maxGreen + P21_GreenTime - P11_GreenTime;
        }

        if (P12_GreenTime > P22_GreenTime)
        {
            temporaryPhase = P22.back();
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup =
                std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                             [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

            findSignalGroup->maxGreen = findSignalGroup->maxGreen + P12_GreenTime - P22_GreenTime;
        }

        else if (P22_GreenTime > P12_GreenTime)
        {
            temporaryPhase = P12.back();
            vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup =
                std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                             [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

            findSignalGroup->maxGreen = findSignalGroup->maxGreen + P22_GreenTime - P12_GreenTime;
        }
    }
}

double SolverDataManager::calulateGmax(vector<int> PhaseGroup)
{
    int temporaryPhase{};
    double sumOfGmax{};
    vector<double> greenTime{};

    for (size_t i = 0; i < PhaseGroup.size(); i++)
    {
        temporaryPhase = PhaseGroup.at(i);
        vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup =
            std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                         [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

        greenTime.push_back(findSignalGroup->maxGreen);
    }
    sumOfGmax = accumulate(greenTime.begin(), greenTime.end(), 0);

    return sumOfGmax;
}

/*
    - If starting phases (for example, phase 4 and 8) are the last phase of their respective ring barrier group and remaining green time are different, solver may get infeasible solution 
    - The following method will find the maximum remaining green time and adjust the elapsed green time based on that.
*/
void SolverDataManager::modifyCurrentSignalStatus(vector<int> P11, vector<int> P12, vector<int> P21, vector<int> P22)
{
    double largetRemainingGreenTime{};
    double startingPhase1RemainingGreenTime{};
    double startingPhase2RemainingGreenTime{};
    int temporaryPhase{};
    vector<double> startingPhasesRemainingGreenTime{};

    temporaryPhase = trafficControllerStatus[0].startingPhase1;
    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup1 =
        std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                     [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

    temporaryPhase = trafficControllerStatus[0].startingPhase2;
    vector<TrafficControllerData::TrafficSignalPlan>::iterator findSignalGroup2 =
        std::find_if(std::begin(trafficSignalPlan), std::end(trafficSignalPlan),
                     [&](TrafficControllerData::TrafficSignalPlan const &p) { return p.phaseNumber == temporaryPhase; });

    if (trafficControllerStatus[0].elapsedGreen1 > findSignalGroup1->maxGreen)
        trafficControllerStatus[0].elapsedGreen1 = findSignalGroup1->maxGreen;

    if (trafficControllerStatus[0].elapsedGreen2 > findSignalGroup2->maxGreen)
        trafficControllerStatus[0].elapsedGreen2 = findSignalGroup2->maxGreen;

    startingPhase1RemainingGreenTime = findSignalGroup1->maxGreen - trafficControllerStatus[0].elapsedGreen1;
    startingPhasesRemainingGreenTime.push_back(startingPhase1RemainingGreenTime);

    startingPhase2RemainingGreenTime = findSignalGroup2->maxGreen - trafficControllerStatus[0].elapsedGreen2;
    startingPhasesRemainingGreenTime.push_back(startingPhase2RemainingGreenTime);

    if ((trafficControllerStatus[0].startingPhase1 == P11.back() || trafficControllerStatus[0].startingPhase1 == P12.back()) &&
        (trafficControllerStatus[0].startingPhase2 == P21.back() || trafficControllerStatus[0].startingPhase2 == P22.back()))
    {

        largetRemainingGreenTime = *max_element(startingPhasesRemainingGreenTime.begin(), startingPhasesRemainingGreenTime.end());

        if (largetRemainingGreenTime != startingPhase1RemainingGreenTime)
            trafficControllerStatus[0].elapsedGreen1 = findSignalGroup1->maxGreen - largetRemainingGreenTime;

        if (largetRemainingGreenTime != startingPhase2RemainingGreenTime)
            trafficControllerStatus[0].elapsedGreen2 = findSignalGroup2->maxGreen - largetRemainingGreenTime;
    }

    if (trafficControllerStatus[0].elapsedGreen1 < 0.0)
        trafficControllerStatus[0].elapsedGreen1 = 0.0;

    if (trafficControllerStatus[0].elapsedGreen2 < 0.0)
        trafficControllerStatus[0].elapsedGreen2 = 0.0;
}

vector<RequestList> SolverDataManager::getPriorityRequestList()
{
    return priorityRequestList;
}

SolverDataManager::~SolverDataManager()
{
}