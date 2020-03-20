
#pragma once
#include <iostream>
#include <vector>
#include "TrafficSignalPlan.h"

using std::cout;
using std::endl;
using std::ifstream;
// using std::ios;
// using std::ofstream;
using std::string;
// using std::stringstream;
using std::vector;

class SignalCoordination
{
private:
    int noOfPhase{};
    int noOfPhasesInRing1{};
    int noOfPhasesInRing2{};
    double coordinationStartTime{};
    double coordinationEndTime{};
    double offsetTime{};
    double cycleLength{};
    bool bCoordination = false;

    vector<TrafficControllerData::TrafficConrtollerStatus> trafficControllerStatus;
    vector<TrafficControllerData::TrafficSignalPlan> trafficSignalPlan;

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

public:
    SignalCoordination();
    ~SignalCoordination();

    void generateVirtualCoordinationPriorityRequest();
    void getCurrentSignalStatus();
    void readCurrentSignalTimingPlan();
    void getCoordinationTime();
    void getCurrentTime();
    void getFirstCycleLength();
    bool checkCoordinationTimeOfTheDay();
};
