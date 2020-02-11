/*
**********************************************************************************
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
#include "TrafficSignalPlan.h"
#include "RequestList.h"
#include "Schedule.h"
#include <sstream>

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::stringstream;
// using std::fstream;
using std::ifstream;
using std::ofstream;
using std::ios;

class PriorityRequestSolver
{
private:
    vector<RequestList> priorityRequestList;
    vector<Schedule::GLPKSchedule> glpkSchedule;
    vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus;
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan;
    vector<int>PhaseNumber;
    vector<double> PedWalk;
    vector<double> PedClear;
    vector<double> MinGreen;
    vector<double> Passage;
    vector<double> MaxGreen;
    vector<double> YellowChange;
    vector<double> RedClear;
    vector<int> PhaseRing;
    vector<int>requestedSignalGroup;
    vector<int>P11;
    vector<int>P12;
    vector<int>P21;
    vector<int>P22;
    int noOfPhase{};
    int numberOfTransitInList{};
    int numberOfTruckInList{};
    vector<double> leftCriticalPoints;
    vector<double> rightCriticalPoints;
    vector<double> leftCriticalPoints_GreenTime;
    vector<double> rightCriticalPoints_GreenTime;
    vector<int> a;


public:
    PriorityRequestSolver();
    ~PriorityRequestSolver();


    void createPriorityRequestList(string jsonString);
    void getCurrentSignalStatus();
    void getRequestedSignalGroupFromPriorityRequestList();
    void removeDuplicateSignalGroup();
    void addAssociatedSignalGroup();
    void modifyGreenMax();
    void generateDatFile();
    double GetSeconds();
    void GLPKSolver();
    bool GLPKSolutionValidation();
    void readOptimalPlan();
    void split(string strToSplit);
    // vector<TrafficSignalPlan>readCurrentSignalTimingPlan();
    void readCurrentSignalTimingPlan();
    void GenerateModFile();
    void printSignalPlan();
    void printvector();
    
    
};
