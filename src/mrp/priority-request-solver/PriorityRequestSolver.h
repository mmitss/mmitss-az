/***********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorityRequestSolver.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. Header file for PriorityRequestSolver class
*/

#pragma once
#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>
#include "ScheduleManager.h"
#include "SolverDataManager.h"
#include "TrafficSignalPlan.h"
#include "Schedule.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::stringstream;
using std::vector;

#define SignalCoordinationVehicleType 20
#define Initialize 0.0
#define Tolerance 1.0
#define NumberOfStartingPhase 2
#define NumberOfPhasePerRing 4
#define FirstPhaseOfRing1 1
#define FirstPhaseOfRing2 5
#define LastPhaseOfRing1 4
#define LastPhaseOfRing2 8
#define MinuteToSecondCoversion 60.0
#define HourToSecondConversion 3600.0


enum msgType
{
  priorityRequest = 1,
  clearRequest = 2,
  signalPlan = 3,
  currentPhaseStatus = 4,
  splitData = 5
};

class PriorityRequestSolver
{
private:
  int noOfPhase{};
  int noOfEVInList{};
  int coordinatedPhase1{};
  int coordinatedPhase2{};
  bool emergencyVehicleStatus{};
  bool signalCoordinationRequestStatus{};
  bool optimalSolutionStatus{};
  bool loggingStatus{};
  double EmergencyVehicleWeight{};
  double EmergencyVehicleSplitPhaseWeight{};
  double TransitWeight{};
  double TruckWeight{};
  double DilemmaZoneRequestWeight{};
  double CoordinationWeight{};
  double MaximumGreen{};
  double cycleLength{};
  double offset{};
  double coordinationStartTime{};

  vector<RequestList> priorityRequestList{};
  vector<RequestList> dilemmaZoneRequestList{};
  vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus{};
  vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan{};
  vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan_EV{};
  vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan_SignalCoordination{};
  vector<int> PhaseNumber{};
  vector<double> PedWalk{};
  vector<double> PedClear{};
  vector<double> MinGreen{};
  vector<double> Passage{};
  vector<double> MaxGreen{};
  vector<double> YellowChange{};
  vector<double> RedClear{};
  vector<int> PhaseRing{};
  vector<int> P11{};
  vector<int> P12{};
  vector<int> P21{};
  vector<int> P22{};
  vector<int> EV_P11{};
  vector<int> EV_P12{};
  vector<int> EV_P21{};
  vector<int> EV_P22{};
  vector<int> requestedSignalGroup{};
  vector<int> plannedEVPhases{};

public:
  PriorityRequestSolver();
  ~PriorityRequestSolver();

  void createPriorityRequestList(string jsonString);
  void createDilemmaZoneRequestList();
  void modifyPriorityRequestList();
  void modifySignalTimingPlan();
  void modifyCoordinationSignalTimingPlan();
  void deleteSplitPhasesFromPriorityRequestList();
  void GLPKSolver();
  void getCurrentSignalTimingPlan(string jsonString);
  void getSignalCoordinationTimingPlan(string jsonString);
  void generateModFile();
  void generateEVModFile();
  void setOptimizationInput();
  void getRequestedSignalGroup();
  void getEVPhases();
  void getEVTrafficSignalPlan();
  void getCurrentSignalStatus(string jsonString);
  void modifyTrafficControllerStatus();
  void validateTrafficControllerStatus();
  void validateEVTrafficSignalPlan();
  void loggingSignalPlanData(string jsonString);
  void loggingSplitData(string jsonString);
  void loggingOptimizationData(string priorityRequestString, string signalStatusString, string scheduleString);
  void loggingClearRequestData(string jsonString);
  void printSignalPlan();
  string getScheduleforTCI();
  string getClearCommandScheduleforTCI();
  string getSignalTimingPlanRequestString();
  string getCurrentSignalStatusRequestString();
  int getMessageType(string jsonString);
  double getSeconds();
  double getCurrentTime();
  double getCoefficientOfFrictionValue(double vehicleSpeed);
  bool findEVInList();
  bool findCoordinationRequestInList();
  bool getOptimalSolutionValidationStatus();
  bool logging();
};