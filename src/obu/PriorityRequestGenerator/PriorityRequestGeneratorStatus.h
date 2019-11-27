/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorrityRequestGeneratorStatus.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the header file for PriorrityRequestGeneratorStatus.cpp
*/

#pragma once
#include <string>
#include <vector>
#include "PriorityRequestGenerator.h"
#include "MapManager.h"
#include "BasicVehicle.h"
#include "Position3D.h"
#include "Map.h"
#include "ActiveRequest.h"

class PriorityRequestGeneratorStatus
{
private:
  // int noOfRequest{};
  // int *vehicleID = new int[noOfRequest];
	// int *requestID = new int[noOfRequest];
	// int *msgCount = new int[noOfRequest];
	// int *inBoundLaneID = new int[noOfRequest];
	// int *inBoundApproachID = new int[noOfRequest];
	// int *basicVehicleRole = new int[noOfRequest];
	// int *expectedTimeOfArrival_Minute = new int[noOfRequest];
	// double *expectedTimeOfArrival_Second = new double[noOfRequest];
	// double *expectedTimeOfArrival_Duration = new double[noOfRequest];
	// int *priorityRequestStatus = new int[noOfRequest];
  std::vector<Map::AvailableMap> availableMapList;
  std::vector<ActiveRequest>ActiveRequestTable;

public:
    PriorityRequestGeneratorStatus();
    ~PriorityRequestGeneratorStatus();
    std::vector<Map::AvailableMap>getAavailableMapList(MapManager mapManager);
    std::vector<ActiveRequest>getActiveRequestTable(PriorityRequestGenerator PRG);
    // std::string priorityRequestGeneratorStatus2Json(PriorityRequestGenerator priorityRequestGenerator, BasicVehicle basicVehicle, std::vector<Map::AvailableMap>availableMapList, std::vector<ActiveRequest>ActiveRequestTable);
    std::string priorityRequestGeneratorStatus2Json(PriorityRequestGenerator priorityRequestGenerator, BasicVehicle basicVehicle, MapManager mapManager);
};


