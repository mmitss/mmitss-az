#pragma once

#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

class PriorityConfig {
public:
    double dCoordinationWeight;
    int iCoordinatedPhase[2];
    double dCoordCycle;
    double dCoordinationPhaseSplit[8];
    double dCoordOffset;
    double dTransitWeight;
    double dTruckWeight;

    int iNumberOfCoordinatedPhase;
    bool bSimilarCoordinationSplit;
    double dSmallerCoordinationSplit;
    double dLargerCoordinationSplit;
    double dSplitDifference;

public:
    PriorityConfig();

    ~PriorityConfig();

    void setPriorityConfig(double dCoordinationWeight, int iCoordinatedPhase[2], double dCoordCycle,
                           double dCoordinationPhaseSplit[8], double dCoordOffset, double dTransitWeight,
                           double dTruckWeight);

    void readPriorityConfig(const char *filename);
};
