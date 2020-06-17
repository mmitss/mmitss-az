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
    double maxEV_ETA{};
    double maxEV_ETA_Duration{};
    vector<int> requestedSignalGroup;

    vector<RequestList> priorityRequestList;
    vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus;
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan;

public:
    SolverDataManager();
    SolverDataManager(vector<RequestList> requestList);
    SolverDataManager(vector<RequestList> requestList, vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus, vector<TrafficControllerData::TrafficSignalPlan> signalPlan);
    
    ~SolverDataManager();

    vector<int> getRequestedSignalGroupFromPriorityRequestList();
    void removeDuplicateSignalGroup();
    void addAssociatedSignalGroup();
    void modifyGreenMax();
    void findMaximumETAofEV();
    void generateDatFile(bool bEVStatus);


    // void getRequestedSignalGroupFromPriorityRequestList(vector<RequestList> priorityRequestList);
    // void removeDuplicateSignalGroup();
    // void addAssociatedSignalGroup(vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan);
    // void modifyGreenMax(vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan);
    // void generateDatFile(vector<RequestList> priorityRequestList, vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan);
};
