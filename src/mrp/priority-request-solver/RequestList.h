#pragma once
struct RequestList
{
    int vehicleID{};
    int vehicleType{};
    int basicVehicleRole{};
    int laneID{};
    int requestedPhase{};
    int prioritystatus{};
    double vehicleETA{};
    double vehicleETA_Duration{};
    double vehicleLatitude{};
    double vehicleLongitude{};
    double vehicleElevation{};
    double vehicleHeading{};
    double vehicleSpeed{};
    double vehicleDistanceFromStopBar{};

    void reset()
    {
        vehicleID = 0;
        vehicleType = 0;
        basicVehicleRole = 0;
        laneID = 0;
        requestedPhase = 0;
        prioritystatus = 0;
        vehicleETA = 0.0;
        vehicleETA_Duration = 0.0;
        vehicleLatitude = 0.0;
        vehicleLongitude = 0.0;
        vehicleElevation = 0.0;
        vehicleHeading = 0.0;
        vehicleSpeed = 0.0;
        vehicleDistanceFromStopBar = 0.0;
    }
};