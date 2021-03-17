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
    /* data */
public:
    OptimizationModelManager();
    ~OptimizationModelManager();

    void generateModFile(int noOfPhase, vector<int> PhaseNumber, vector<int> P11, vector<int> P12, vector<int> P21, vector<int> P22);
    void generateEVModFile(vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan_EV, vector<int> EV_P11, vector<int> EV_P12, vector<int> EV_P21, vector<int> EV_P22);
};


