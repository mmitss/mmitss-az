#pragma once
#include <vector>
#include <string>
#include "Schedule.h"
#include "TrafficSignalPlan.h"
#include "RequestList.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::string;
using std::vector;


// using std::ios;
using std::ofstream;

class ScheduleManager
{
private:
    bool bEVStatus{};
    vector<RequestList> priorityRequestList;
    vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus;
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan;
    
    vector<Schedule> ring1_TCISchedule;
    vector<Schedule> ring2_TCISchedule;

    vector<double> leftCriticalPoints_PhaseDuration_Ring1;
    vector<double> leftCriticalPoints_PhaseDuration_Ring2;
    vector<double> rightCriticalPoints_PhaseDuration_Ring1;
    vector<double> rightCriticalPoints_PhaseDuration_Ring2;
    vector<double> leftCriticalPoints_GreenTime_Ring1;
    vector<double> leftCriticalPoints_GreenTime_Ring2;
    vector<double> rightCriticalPoints_GreenTime_Ring1;
    vector<double> rightCriticalPoints_GreenTime_Ring2;

    vector<int> plannedSignalGroupInRing1;
    vector<int> plannedSignalGroupInRing2;
    vector<int> omitPhases;

public:
    ScheduleManager();
    ScheduleManager(vector<RequestList> requestList, vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus, vector<TrafficControllerData::TrafficSignalPlan> signalPlan, bool EVStatus);
    ~ScheduleManager();

    void readOptimalSignalPlan();
    void obtainRequiredSignalGroup();
    void createEventList();
    void getOmitPhases();
    string createScheduleJsonString();
};
