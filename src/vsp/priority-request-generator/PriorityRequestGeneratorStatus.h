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
#include "PriorityRequestGenerator.h"
#include "MapManager.h"
#include "BasicVehicle.h"
#include "Position3D.h"
#include "Map.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

class PriorityRequestGeneratorStatus
{
private:
  vector<Map::AvailableMap> availableMapList;
  vector<ActiveRequest>ActiveRequestTable;

public:
    PriorityRequestGeneratorStatus();
    ~PriorityRequestGeneratorStatus();
    void setAvailableMapList(PriorityRequestGenerator priorityRequestGenerator);
    void setActiveRequestTable(PriorityRequestGenerator priorityRequestGenerator);
    string priorityRequestGeneratorStatus2Json(PriorityRequestGenerator priorityRequestGenerator, BasicVehicle basicVehicle);
};


