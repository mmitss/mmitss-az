#pragma once
struct RequestList
{
    int vehicleID{};
    int vehicleType{};
    int basicVehicleRole{};
    int requestedPhase{};
    double vehicleETA{};
    double vehicleETA_Duration{};
    double vehicleSpeed{};
    double vehicleDistanceFromStopBar{};
    bool dilemmaZoneStatus{};

    void reset()
    {
        vehicleID = 0;
        vehicleType = 0;
        basicVehicleRole = 0;
        requestedPhase = 0;
        vehicleETA = 0.0;
        vehicleETA_Duration = 0.0;
        vehicleSpeed = 0.0;
        vehicleDistanceFromStopBar = 0.0;
        dilemmaZoneStatus = false;
    }
};