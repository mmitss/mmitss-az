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
    vector<int> requestedSignalGroup;

public:
    SolverDataManager(/* args */);
    ~SolverDataManager();

    void getRequestedSignalGroupFromPriorityRequestList(vector<RequestList> priorityRequestList);
    void removeDuplicateSignalGroup();
    void addAssociatedSignalGroup(vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan);
    void modifyGreenMax(vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan);
    void generateDatFile(vector<RequestList> priorityRequestList, vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan);
};
