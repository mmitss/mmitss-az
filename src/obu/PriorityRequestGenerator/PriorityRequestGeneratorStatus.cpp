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

const double ETA_DURATION_SECOND = 2;

PriorityRequestGeneratorStatus::PriorityRequestGeneratorStatus()
{
}

std::vector<Map::AvailableMap>PriorityRequestGeneratorStatus::getAavailableMapList(MapManager mapManager)
{
	availableMapList = mapManager.getAavailableMapList();
	return availableMapList;
}

std::vector<ActiveRequest>PriorityRequestGeneratorStatus::getActiveRequestTable(PriorityRequestGenerator PRG)
{
    ActiveRequestTable = PRG.getActiveRequestTable();
    return ActiveRequestTable;
}

std::string PriorityRequestGeneratorStatus::priorityRequestGeneratorStatus2Json(PriorityRequestGenerator priorityRequestGenerator, BasicVehicle basicVehicle, MapManager mapManager)
{
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    std::string jsonString;

    getAavailableMapList(mapManager);
    getActiveRequestTable(priorityRequestGenerator);

    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["secMark_Second"] = basicVehicle.getSecMark_Second();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["vehicleID"] = basicVehicle.getTemporaryID() ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["vehicleType"] = priorityRequestGenerator.getVehicleType() ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["latitude_DecimalDegree"] = basicVehicle.getLatitude_DecimalDegree();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["longitude_DecimalDegree"] = basicVehicle.getLongitude_DecimalDegree();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["elevation_Meter"] = basicVehicle.getElevation_Meter();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["heading_Degree"] = basicVehicle.getHeading_Degree() ;
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["speed_MeterPerSecond"] = basicVehicle.getSpeed_MeterPerSecond();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["laneID"] = priorityRequestGenerator.getLaneID();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["signalGroup"] = priorityRequestGenerator.getVehicleCurrentSignalGroup();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["prioritystatus"]["OnMap"] =  priorityRequestGenerator.getVehicleMapStatus();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["prioritystatus"]["requestSent"] = priorityRequestGenerator.getVehicleRequestSentStatus();
    for (unsigned int i = 0; i < availableMapList.size(); i++)
    {
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"][i]["availableMaps"]["map_intersectionID"] = availableMapList[i].mapIntersectionID;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"][i]["availableMaps"]["map_descriptiveName"] = availableMapList[i].availableMapFileName;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"][i]["availableMaps"]["map_active"] =  availableMapList[i].activeMapStatus;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"][i]["availableMaps"]["map_age"] = availableMapList[i].mapAge;
    }
    for (unsigned int i = 0; i < ActiveRequestTable.size(); i++)
    {
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["vehicleID"] = ActiveRequestTable[i].vehicleID;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["requestID"] = ActiveRequestTable[i].requestID;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["msgCount"] = ActiveRequestTable[i].msgCount;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["basicVehicleRole"] = ActiveRequestTable[i].basicVehicleRole;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["inBoundLaneID"] = ActiveRequestTable[i].vehicleLaneID;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["inBoundApproachID"] = ActiveRequestTable[i].vehicleApproachID;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["ETA"] = ActiveRequestTable[i].vehicleETA;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["ETA_Duration"] = ETA_DURATION_SECOND;
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["priorityRequestStatus"] = ActiveRequestTable[i].prsStatus;
    }

    jsonString = fastWriter.write(jsonObject);
    return jsonString;
}

PriorityRequestGeneratorStatus::~PriorityRequestGeneratorStatus()
{
}