#pragma once
#include "../../vsp/priority-request-generator/PriorityRequestGenerator.h"
#include "../../vsp/priority-request-generator/MapManager.h"
#include "../../vsp/priority-request-generator/PriorityRequestGeneratorStatus.h"

using std::string;

struct ServerList
{
    int vehicleID{};
    string vehicleType{};
    double updateTime{};
    double vehicleLatitude{};
    double vehicleLongitude{};
    double vehicleElevation{};
    PriorityRequestGenerator PRG;
    MapManager mapManager;
    PriorityRequestGeneratorStatus prgStatus;
    SignalRequest signalRequest;
    SignalStatus signalStatus;
    
    void reset()
    {
        vehicleID = 0;
        vehicleType = "";
        updateTime = 0.0;
        vehicleLatitude = 0.0;
        vehicleLongitude = 0.0;
        vehicleElevation = 0.0;
    }
};
