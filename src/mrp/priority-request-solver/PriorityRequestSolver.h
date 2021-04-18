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
  bool logging{};
  bool consoleOutput{};
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
  double priorityWeightsCheckedTime{};
  string fileName{};
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
  void setOptimizationInput();
  void getRequestedSignalGroup();
  void getEVPhases();
  void getEVTrafficSignalPlan();
  void getCurrentSignalStatus(string jsonString);
  void validateEVTrafficSignalPlan();
  void getPriorityWeights();
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
  int getMessageType(string jsonString);
  double getCoefficientOfFrictionValue(double vehicleSpeed);
  bool findEVInList();
  bool findCoordinationRequestInList();
  bool getOptimalSolutionValidationStatus();
  bool checkTrafficSignalTimingPlanStatus();
  bool checkSignalCoordinationTimingPlanStatus();
  bool checkUpdatesForPriorityWeights();
};