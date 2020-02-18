struct RequestList
{
    int vehicleID{};
    int vehicleType{};
    int basicVehicleRole{};
    int laneID{};
    double vehicleETA{};
    double vehicleETA_Duration{};
    int requestedPhase{};
    int splitPhase{};
    int prioritystatus{};

    void reset()
    {
    vehicleID = 0;
    vehicleType = 0;
    basicVehicleRole = 0;
    laneID = 0;
    vehicleETA = 0.0;
    vehicleETA_Duration = 0.0;
    requestedPhase = 0;
    splitPhase = 0;
    prioritystatus = 0;
    }
};