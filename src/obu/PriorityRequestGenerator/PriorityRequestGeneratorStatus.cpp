/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorrityRequestGeneratorStatus.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. 
*/

#include "PriorityRequestGeneratorStatus.h"
#include "json/json.h"

PriorityRequestGeneratorStatus::PriorityRequestGeneratorStatus()
{
}

std::string PriorityRequestGeneratorStatus::priorityRequestGeneratorStatus2Json(PriorityRequestGenerator priorityRequestGenerator, BasicVehicle basicVehicle, std::vector<Map::AvailableMap> availableMapList, std::vector<ActiveRequest> ActiveRequestTable)
{
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    std::string jsonString;

    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["secMark"] = basicVehicle.getSecMark_Second();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["vehicleID"] = basicVehicle.getTemporaryID() ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["vehicleType"] = priorityRequestGenerator.getVehicleType() ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["latitude_DecimalDegree"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["longitude_DecimalDegree"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["elevation_Meter"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["heading_Degree"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["speed"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["lane"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["signalGroup"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["prioritystatus"]["OnMap"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["prioritystatus"]["requestSent"] = ;
    for (unsigned int i = 0; i < availableMapList.size(); i++)
    jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"][i]["availableMaps"]["map_intersectionID"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"[i]]["availableMaps"]["map_descriptiveName"] = availableMapList[i].availableMapFileName;
    jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"][i]["availableMaps"]["map_active"] = ;
    jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"][i]["availableMaps"]["map_age"] = availableMapList[i].minuteOfYear;
    for (unsigned int i = 0; i < ActiveRequestTable.size(); i++)
    {
        jsonObject["PriorityRequestGeneratorStatus"]["activeRequestTable"][i]["vehicleID"] = ActiveRequestTable[i].vehicleID;
        jsonObject["PriorityRequestGeneratorStatus"]["activeRequestTable"][i]["requestID"] = ActiveRequestTable[i].requestID;
        jsonObject["PriorityRequestGeneratorStatus"]["activeRequestTable"][i]["msgCount"] = ActiveRequestTable[i].msgCount;
        jsonObject["PriorityRequestGeneratorStatus"]["activeRequestTable"][i]["basicVehicleRole"] = ActiveRequestTable[i].basicVehicleRole;
        jsonObject["PriorityRequestGeneratorStatus"]["activeRequestTable"][i]["inBoundLaneID"] = ActiveRequestTable[i].vehicleLaneID;
        jsonObject["PriorityRequestGeneratorStatus"]["activeRequestTable"][i]["inBoundApproachID"] = ActiveRequestTable[i].vehicleApproachID;
        jsonObject["PriorityRequestGeneratorStatus"]["activeRequestTable"][i]["ETA_Minute"] = int(ActiveRequestTable[i].vehicleETA);
        jsonObject["PriorityRequestGeneratorStatus"]["activeRequestTable"][i]["priorityRequestStatus"] = ActiveRequestTable[i].prsStatus;
    }
 
}

PriorityRequestGeneratorStatus::~PriorityRequestGeneratorStatus()
{
}