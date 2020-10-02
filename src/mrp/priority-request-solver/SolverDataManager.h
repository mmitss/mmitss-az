/***********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  SolverDataManager.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. Header file for SolverDataManager class
*/
#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "RequestList.h"
#include "TrafficSignalPlan.h"

using std::vector;
using std::ios;
using std::ofstream;
using std::cout;
using std::endl;


class SolverDataManager
{
private:
    int numberOfTransitInList{};
    int numberOfTruckInList{};
    int numberOfEVInList{};
    int numberOfEVSplitRequestInList{};
    double maxEV_ETA{};
    double maxEV_ETA_Duration{};
    vector<int> requestedSignalGroup;

    vector<RequestList> priorityRequestList;
    vector<RequestList> dilemmaZoneRequestList;
    vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus;
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan;

public:
    SolverDataManager();
    SolverDataManager(vector<RequestList> requestList);
    SolverDataManager(vector<RequestList> dilemmaZoneList, vector<RequestList> requestList, vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus, vector<TrafficControllerData::TrafficSignalPlan> signalPlan);
    
    ~SolverDataManager();

    vector<int> getRequestedSignalGroupFromPriorityRequestList();
    void removeDuplicateSignalGroup();
    void addAssociatedSignalGroup();
    void modifyGreenMax();
    void modifyCurrentSignalStatus();
    void findMaximumETAofEV();
    void generateDatFile(bool emergencyVehicleStatus);
    bool findSignalGroupInList(int signalGroup);
};
