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

public:
    ScheduleManager();
    ~ScheduleManager();

    void readOptimalSignalPlan();
    void obtainRequiredSignalGroup(vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan);
    void createEventList(vector<RequestList> priorityRequestList, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan);
    string createScheduleJsonString();
};
