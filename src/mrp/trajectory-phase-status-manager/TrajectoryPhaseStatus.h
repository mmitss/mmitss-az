#pragma once
#include <vector>
#include "BsmPhaseStatus.h"
#include "BasicVehicle.h"
#include <json/json.h>

using std::vector;

struct TrajectoryPhaseStatus
{
    int vechileId{};
    double updateTime{};
    vector<BsmPhaseStatus>vechileInformation{};

    void reset()
    {
        vechileId = 0;
        vechileInformation.clear();
        updateTime = 0.0;
    }
};
