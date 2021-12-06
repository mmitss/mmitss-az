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

using std::cout;
using std::endl;
using std::ios;
using std::ofstream;
using std::vector;

#define VehicleClass_EmergencyVehicle 1
#define VehicleClass_Transit 2
#define VehicleClass_Truck 3
#define VehicleClass_DilemmaZone 4
#define VehicleClass_Coordination 5
#define VehicleClass_EmergencyVehicleSplitRequest 6
#define CoordinationVehicleType 20
#define Maximum_Number_Of_Priority_Request 15

class SolverDataManager
{
private:
  int numberOfTransitInList{};
  int numberOfTruckInList{};
  int numberOfEVInList{};
  int numberOfEVSplitRequestInList{};
  int numberOfDilemmaZoneRequestInList{};
  int numberOfCoordinationRequestInCycle1{};
  int numberOfCoordinationRequestInCycle2{};
  double maxEV_ETA{};
  double maxEV_ETA_Duration{};
  double EmergencyVehicleWeight{1.0};
  double EmergencyVehicleSplitPhaseWeight{0.1};
  double TransitWeight{1.0};
  double TruckWeight{1.0};
  double DilemmaZoneRequestWeight{20.0};
  double CoordinationWeight{0.1};
  vector<int> requestedSignalGroup{};
  vector<RequestList> priorityRequestList;
  vector<RequestList> dilemmaZoneRequestList;
  vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus;
  vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan;
  vector<int> conflictingPedCallList{};
  bool pedCallStatus{false};

public:
  SolverDataManager();
  SolverDataManager(vector<RequestList> requestList);
  SolverDataManager(vector<RequestList> dilemmaZoneList, vector<RequestList> requestList,
                    vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus,
                    vector<TrafficControllerData::TrafficSignalPlan> signalPlan, vector<int> listOfConflictingPedCall,
                    double EV_Weight, double EV_SplitPhase_Weight, double Transit_Weight, double Truck_Weight,
                    double DZ_Request_Weight, double Coordination_Weight);

  SolverDataManager(vector<RequestList> dilemmaZoneList, vector<RequestList> requestList,
                    vector<TrafficControllerData::TrafficConrtollerStatus> signalStatus,
                    vector<TrafficControllerData::TrafficSignalPlan> signalPlan, vector<int> listOfConflictingPedCall,
                    vector<int> requested_Signal_Group, double EV_Weight, double EV_SplitPhase_Weight, double Transit_Weight, 
                    double Truck_Weight, double DZ_Request_Weight, double Coordination_Weight);

  ~SolverDataManager();
  
  void removeDuplicateSignalGroup();
  void addAssociatedSignalGroup();
  void modifyGreenMax(bool emergencyVehicleStatus);
  void modifyGreenTimeForConflictingPedCalls();
  void modifyGreenTimeForCurrentPedCalls();
  void modifyCurrentSignalStatus(vector<int> P11, vector<int> P12, vector<int> P21, vector<int> P22);
  void adjustGreenTimeForPedCall(vector<int> P11, vector<int> P12, vector<int> P21, vector<int> P22);
  void generateDatFile(bool emergencyVehicleStatus, double earlyReturnValue1, double earlyReturnValue2, int coordinatedPhase1, int coordinatedPhase2);
  void validateGmaxForEVSignalTimingPlan(vector<int> EV_P11, vector<int> EV_P12, vector<int> EV_P21, vector<int> EV_P22);
  double calulateGmax(vector<int>PhaseGroup);
  bool findSignalGroupInList(int signalGroup);
  vector<int> getRequestedSignalGroupFromPriorityRequestList();
  vector<RequestList>getPriorityRequestList();
};
