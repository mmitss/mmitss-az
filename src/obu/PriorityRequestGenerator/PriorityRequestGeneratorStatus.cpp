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
#include <fstream>

const double ETA_DURATION_SECOND = 2;

PriorityRequestGeneratorStatus::PriorityRequestGeneratorStatus()
{
}

std::vector<Map::AvailableMap> PriorityRequestGeneratorStatus::getAavailableMapList(MapManager mapManager)
{
    availableMapList = mapManager.getAavailableMapList();
    return availableMapList;
}

std::vector<ActiveRequest> PriorityRequestGeneratorStatus::getActiveRequestTable(PriorityRequestGenerator PRG)
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
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["vehicleID"] = basicVehicle.getTemporaryID();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["vehicleType"] = priorityRequestGenerator.getVehicleType();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["latitude_DecimalDegree"] = basicVehicle.getLatitude_DecimalDegree();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["longitude_DecimalDegree"] = basicVehicle.getLongitude_DecimalDegree();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["position"]["elevation_Meter"] = basicVehicle.getElevation_Meter();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["heading_Degree"] = basicVehicle.getHeading_Degree();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["speed_MeterPerSecond"] = basicVehicle.getSpeed_MeterPerSecond();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["laneID"] = priorityRequestGenerator.getLaneID();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["signalGroup"] = priorityRequestGenerator.getVehicleCurrentSignalGroup();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["priorityStatus"]["OnMAP"] = priorityRequestGenerator.getVehicleMapStatus();
    jsonObject["PriorityRequestGeneratorStatus"]["hostVehicle"]["priorityStatus"]["requestSent"] = priorityRequestGenerator.getVehicleRequestSentStatus();

    if (availableMapList.empty())
    {
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["availableMaps"]= {};
    }
    
    else
    {
        for (unsigned int i = 0; i < availableMapList.size(); i++)
        {
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["availableMaps"][i]["IntersectionID"] = availableMapList[i].mapIntersectionID;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["availableMaps"][i]["DescriptiveName"] = availableMapList[i].availableMapFileName;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["availableMaps"][i]["active"] = availableMapList[i].activeMapStatus;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["availableMaps"][i]["age"] = availableMapList[i].mapAge;
        }
    }

    if (ActiveRequestTable.empty())
    {
        jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"] = {};
    }
    
    else
    {
        for (unsigned int i = 0; i < ActiveRequestTable.size(); i++)
        {
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["vehicleID"] = ActiveRequestTable[i].vehicleID;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["requestID"] = ActiveRequestTable[i].requestID;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["msgCount"] = ActiveRequestTable[i].msgCount;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["basicVehicleRole"] = ActiveRequestTable[i].basicVehicleRole;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["inBoundLane"] = ActiveRequestTable[i].vehicleLaneID;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["inBoundApproach"] = ActiveRequestTable[i].vehicleApproachID;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["vehicleETA"] = ActiveRequestTable[i].vehicleETA;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["duration"] = ETA_DURATION_SECOND;
            jsonObject["PriorityRequestGeneratorStatus"]["infrastructure"]["activeRequestTable"][i]["priorityRequestStatus"] = ActiveRequestTable[i].prsStatus;
        }
    }

    Json::StyledStreamWriter styledStreamWriter;
    std::ofstream outputter("output.json");
    styledStreamWriter.write(outputter, jsonObject);
    jsonString = fastWriter.write(jsonObject);
    return jsonString;
}

PriorityRequestGeneratorStatus::~PriorityRequestGeneratorStatus()
{
}