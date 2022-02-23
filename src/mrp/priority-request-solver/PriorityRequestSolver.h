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

#include <sstream>
#include "ScheduleManager.h"
#include "SolverDataManager.h"
#include "OptimizationModelManager.h"
#include "TrafficConrtollerStatusManager.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::stringstream;
using std::vector;

#define SignalCoordinationVehicleType 20
#define MinuteToSecondCoversion 60.0
#define HourToSecondConversion 3600.0
#define EmergencyVehicle 2
#define Transit 6
#define Truck 9
#define CoordinationVehicleType 20

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
  bool transitOrTruckRequestStatus{};
  bool optimalSolutionStatus{};
  bool logging{};
  bool consoleOutput{};
  double EmergencyVehicleWeight{1.0};
  double EmergencyVehicleSplitPhaseWeight{0.1};
  double TransitWeight{1.0};
  double TruckWeight{1.0};
  double DilemmaZoneRequestWeight{20.0};
  double CoordinationWeight{0.1};
  double FlexibilityWeight{0.01};
  double MaximumGreen{};
  double cycleLength{};
  double offset{};
  double coordinationStartTime{};
  double priorityWeightsCheckedTime{};
  double earlyReturnedValue1{0.0};
  double earlyReturnedValue2{0.0};
  string scheduleJsonString{};
  string logFileName{};
  ofstream logFile;

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
  vector<int> conflictingPedCallList{};
  vector<int> dummyPhasesList{};

public:
  PriorityRequestSolver();
  ~PriorityRequestSolver();

  void createPriorityRequestList(string jsonString);
  void setDilemmaZoneRequesStatus();
  void modifyPriorityRequestList();
  void modifySignalTimingPlan();
  void modifyCoordinationSignalTimingPlan();
  void managePriorityRequestListForEV();
  void GLPKSolver();
  void setCurrentSignalTimingPlan(string jsonString);
  void setSignalCoordinationTimingPlan(string jsonString);
  void setOptimizationInput();
  void getRequestedSignalGroup();
  void getEVPhases();
  void getEVTrafficSignalPlan();
  void getCurrentSignalStatus(string jsonString);
  void validateEVTrafficSignalPlan();
  void getPriorityWeights();
  void getDummyPhases();
  void readConfigFile();
  void loggingData(string logString);
  void displayConsoleData(string consoleString);
  void loggingOptimizationData();  
  void printSignalPlan();
  string getScheduleforTCI();
  string getClearCommandScheduleforTCI();
  string getSignalTimingPlanRequestString();
  string getCurrentSignalStatusRequestString();
  string getSignalCoordinationTimingPlanRequestString();
  string getTimePhaseDiagramMessageString();
  int getMessageType(string jsonString);
  double getCoefficientOfFrictionValue(double vehicleSpeed);
  double getCurrentTime();
  bool findEmergencyVehicleRequestInList();
  bool findTransitOrTruckRequestInList();
  bool findCoordinationRequestInList();
  bool getOptimalSolutionValidationStatus();
  bool checkTrafficSignalTimingPlanStatus();
  bool checkSignalCoordinationTimingPlanStatus();
  bool checkUpdatesForPriorityWeights();
};