/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  OptimizationModelManager.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. Header file for OptimizationModelManager class
*/

#include <vector>
#include <iostream>
#include <fstream>
#include "TrafficSignalPlan.h"

using std::vector;
using std::ifstream;
using std::ios;
using std::ofstream;

class OptimizationModelManager
{
private:
    double FlexibilityWeight{0.01};
public:
    OptimizationModelManager(double Flexibility_Weight);
    ~OptimizationModelManager();

    void generateModFile(int noOfPhase, vector<int> PhaseNumber, vector<int> P11, vector<int> P12, vector<int> P21, vector<int> P22);
    void generateEVModFile(vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan_EV, vector<int> EV_P11, vector<int> EV_P12, vector<int> EV_P21, vector<int> EV_P22);
};


