#pragma once
#include "string"

struct requestEntry
{
    int vehicleID;
    int basicVehicleRole;
    int vehicleLaneID;
    int vehicleApproachID;
    double vehicleETA;
    double vehicleETADuration;
    int requestedPhase;
    int prsStatus;

    void reset()
    {

        vehicleID = 0;
        basicVehicleRole = 0;
        vehicleLaneID = 0;
        vehicleApproachID = 0;
        vehicleETA = 0.0;
        vehicleETADuration = 0.0;
        requestedPhase = 0;
        prsStatus = 0;
    }
};