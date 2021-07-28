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
#include <iomanip>
#include <vector>
#include <iostream>
#include <fstream>
#include "TrafficSignalPlan.h"
#include "json/json.h"
#include "Timestamp.h"

using std::cout;
using std::endl;
using std::fixed;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::setprecision;
using std::showpoint;
using std::string;
using std::vector;

#define Initialize 0.0
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
  bool logging{};
  bool consoleOutput{};
  bool emergencyVehicleStatus{};
  bool coordinationRequestStatus{};
  bool transitOrTruckRequestStatus{};
  bool conflictingPhaseCallStatus{false};
  bool conflictingPedCallStatus{false};
  bool currentPedCallStatus{false};
  bool currentPedCallStatus1{false};
  bool currentPedCallStatus2{false};
  double cycleLength{0.0};
  double offset{0.0};
  double coordinationStartTime{0.0};
  double elapsedTimeInCycle{0.0};
  double permissivePeriod{0.0};
  int coordinatedPhase1{};
  int coordinatedPhase2{};
  vector<int> vehicleCallList{};
  vector<int> pedCallList{};
  vector<int> phaseCallList{};
  vector<int> dummyPhasesList{};
  vector<double> coordinatedPhasesEarlyReturnValue{};
  vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus{};
  vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan{};

public:
  TrafficConrtollerStatusManager(bool emergencyVehicle_Status, bool transitOrTruck_RequestStatus, bool coordination_Request_Status, double cycle_Length, double offset_Value,
                                 double coordination_StartTime, double elapsed_Time_In_Cycle, int coordinated_Phase1, int coordinated_Phase2,
                                 bool logging_Status, bool console_Output_Status, vector<int> listOfDummyPhases,
                                 vector<TrafficControllerData::TrafficSignalPlan> traffic_Signal_Timing_Plan);

  ~TrafficConrtollerStatusManager();

  void manageCurrentSignalStatus(string jsonString);
  void modifyTrafficControllerStatus();
  void validateTrafficControllerStatus();
  void validateElapsedGreenTime();
  void setConflictingPhaseCallStatus();
  void setConflictingPedCallStatus();
  void setPhaseCallList();
  void setCurrentPedCallStatus();
  void setCoordinationPermissivePeriod();
  bool getConflictingPedCallStatus();
  vector<double> getEarlyReturnValue();
  vector<int> getConflictingPedCallList();
  vector<TrafficControllerData::TrafficConrtollerStatus> getTrafficControllerStatus(string jsonString);
};