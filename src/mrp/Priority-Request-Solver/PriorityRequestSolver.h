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
  1. 
*/

#pragma once
#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>
#include "ScheduleManager.h"
#include "SolverDataManager.h"
#include "TrafficSignalPlan.h"
// #include "RequestList.h"
#include "Schedule.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::stringstream;
using std::vector;

enum msgType
{
  priorityRequest = 1,
  clearRequest = 0,
  currentPhaseStatus = 3,
  signalPlan = 4
};

class PriorityRequestSolver
{
private:
  int noOfPhase{};
  int noOfPhasesInRing1{};
  int noOfPhasesInRing2{};
  int noOfEVInList{};
  bool bEVStatus{};
  bool bLogging{};

  vector<RequestList> priorityRequestList;
  vector<RequestList> dilemmaZoneRequestList;
  vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus;
  vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan;
  vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan_EV;
  vector<int> PhaseNumber;
  vector<double> PedWalk;
  vector<double> PedClear;
  vector<double> MinGreen;
  vector<double> Passage;
  vector<double> MaxGreen;
  vector<double> YellowChange;
  vector<double> RedClear;
  vector<int> PhaseRing;
  vector<int> P11;
  vector<int> P12;
  vector<int> P21;
  vector<int> P22;
  vector<int> EV_P11;
  vector<int> EV_P12;
  vector<int> EV_P21;
  vector<int> EV_P22;
  vector<int> requestedSignalGroup;
  vector<int> plannedEVPhases;


public:
  PriorityRequestSolver();
  ~PriorityRequestSolver();

  void createPriorityRequestList(string jsonString);
  void modifyPriorityRequestList();
  void createDilemmaZoneRequestList();
  void deleteSplitPhasesFromPriorityRequestList();
  void GLPKSolver();
  void readCurrentSignalTimingPlan(string jsonString);
  void generateModFile();
  void generateEVModFile();
  void setOptimizationInput();
  void getRequestedSignalGroup();
  void getEVPhases();
  void getEVTrafficSignalPlan();
  void getCurrentSignalStatus(string receivedJsonString);
  void loggingData(string jsonString);
  void loggingTCIData(string jsonString);
  string getScheduleforTCI();
  string getClearCommandScheduleforTCI();
  string getSignalTimingPlanRequestString();
  string getCurrentSignalStatusRequestString();
  int getMessageType(string jsonString);
  double GetSeconds();
  double getCoefficientOfFrictionValue(double vehicleSpeed);
  bool findEVInList();
  bool logging();
  void printSignalPlan();
};
