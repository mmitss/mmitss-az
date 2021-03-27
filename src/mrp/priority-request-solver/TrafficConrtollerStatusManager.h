/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  TrafficConrtollerStatusManager.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. Header file for TrafficConrtollerStatusManager class
*/
#pragma once
#include <chrono>
#include <vector>
#include <iostream>
#include "TrafficSignalPlan.h"
#include "json/json.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::ios;
using std::ofstream;

#define Initialize 0.0
#define Tolerance 2.0
#define FirstPhaseOfRing1 1
#define FirstPhaseOfRing2 5
#define LastPhaseOfRing1 4
#define LastPhaseOfRing2 8
#define NumberOfStartingPhase 2
#define NumberOfPhasePerRing 4
#define PRS_Timed_Out_Value 10.0

class TrafficConrtollerStatusManager
{
private:
    bool loggingStatus{};
    bool coordinationRequestStatus{};
    double cycleLength{};
    double offset{};
    double coordinationStartTime{};
    int coordinatedPhase1{};
    int coordinatedPhase2{};
    string fileName{};

    vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus{};
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan{};
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan_SignalCoordination{};

public:
    TrafficConrtollerStatusManager(bool coordination_Request_Status, double cycle_Length, double offset_Value, double coordination_StartTime,
                                   int coordinated_Phase1, int coordinated_Phase2, bool logging_Status, string file_Name,
                                   vector<TrafficControllerData::TrafficSignalPlan> traffic_Signal_Timing_Plan, vector<TrafficControllerData::TrafficSignalPlan> trafficSignalCoordinationPlan);

    ~TrafficConrtollerStatusManager();

    void manageCurrentSignalStatus(string jsonString);
    void modifyTrafficControllerStatus();
    void validateTrafficControllerStatus();
    double getCurrentTime();
    vector<TrafficControllerData::TrafficConrtollerStatus> getTrafficControllerStatus(string jsonString);
};