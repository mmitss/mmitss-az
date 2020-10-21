/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorrityRequestGenerator.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the header file for SrmManager.cpp
*/
#pragma once
#include "PriorityRequestGenerator.h"
#include "ActiveRequest.h"
#include "MapManager.h"
#include <cmath>

#define minimumDistance 5
#define kmToMeter 1000

class SrmManager
{
protected:
    vector<ActiveRequest> ActiveRequestTable{};
    vector<Map::ActiveMap> activeMapList{};
    vector<BusStopInformation> busStopList{};
    int intersectionID{};
    int vehicleID{};
    int vehicleType{};
    int vehicleIntersectionStatus{};
    int signalGroup{};
    int priorityRequestType{};
    int approachNo{};
    double vehicleSpeed{};
    double vehicleETA{};
    double ETA_Duration{};
    double srmTimeGapValue{};
    double requestTimedOutValue{};
    double busStopLattitude{};
    double busStopLongitude{};
    bool lightSirenStatus{};
    bool busStopPassedStatus{};
    bool requestSendStatus {false};
    bool activeMapStatus{false};

public:
    SrmManager(PriorityRequestGenerator priorityRequestGenerator, vector<Map::ActiveMap> active_Map_List, vector<ActiveRequest>ART, BasicVehicle basicVehicle);
    ~SrmManager();

    string createSRMJsonObject(PriorityRequestGenerator priorityRequestGenerator, BasicVehicle basicVehicle, SignalRequest signalRequest, MapManager mapManager);
    void setPriorityRequestType(int priority_Request_Type);
    
    double haversine(double lat1, double lon1, double lat2, double lon2);
    bool findNearestBusStopLocation();
    bool checkPassedNearestBusStop(BasicVehicle basicVehicle);
    bool checkRequestSendingRequirement();
    bool checkRequestSendingRequirement(vector<BusStopInformation> bus_Stop_List, BasicVehicle basicVehicle);
    bool checkRequestSendingRequirement(bool light_Siren_Status);
};