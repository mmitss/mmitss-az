#pragma once

struct BsmPhaseStatus
{
    int vehicleId{};
    int vehicleType{};
    double vehicleSpeed_MeterPerSecond{};
    int vehicleLaneId{};
    // int vehicleApproachId{};
    int vehicleSignalGroup{};
    double vehicleDistanceFromStopBar{};
    int vehicleLocationOnMap{};
    int phaseStatus{};
    double phaseElapsedTime{};
    

    void reset()
    {
        vehicleId = 0;
        vehicleType = 0;
        vehicleSpeed_MeterPerSecond = 0.0;
        vehicleLaneId = 0;
        // vehicleApproachId = 0;
        vehicleSignalGroup = 0;
        vehicleDistanceFromStopBar = 0.0;
        vehicleLocationOnMap = 0;
        phaseStatus = 0;
        phaseElapsedTime = 0;
        
    }
};
