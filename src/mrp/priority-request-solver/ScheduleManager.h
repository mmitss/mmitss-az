/***********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  ScheduleManager.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. Header file for ScheduleManager class
*/
#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "Schedule.h"
#include "TrafficSignalPlan.h"
#include "RequestList.h"
#include "Timestamp.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::string;
using std::vector;
using std::ofstream;
using std::fixed;
using std::showpoint;
using std::setprecision;

class ScheduleManager
{
private:
    bool emergencyVehicleStatus{};
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
    bool validateOptimalSolution();
};
