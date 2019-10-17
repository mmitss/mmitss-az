/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  MapManager.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is header file for mapmanager.cpp
*/

#pragma once
#include <vector>
#include <list>
#include <string>
#include "BasicVehicle.h"
#include "Position3D.h"
#include "Map.h"

class MapManager
{
private:
    std::vector<Map::AvailableMap> availableMapList;
    std::vector<Map::ActiveMap> activeMapList;
    std::string mapPayload{};
    int numberOfMapFile{};
    std::string timedOutMapPayLoad{};

public:
    MapManager();
    ~MapManager();

    void json2MapPayload(std::string jsonString);
    void setTimedOutMapPayLoad(std::string timedOutPayLoad);
    std::string getTimedOutMapPayLoad();
    void writeMAPPayloadInFile();
    bool addToMapInList();
    bool updateMapPayLoadList();
    bool deleteMapPayLoadFromList();
    int getMapPayloadReceivedTime();
    void maintainAvailableMapList();
    void deleteMap();
    void printAvailableMapList();
    void createActiveMapList(BasicVehicle basicVehicle);
    std::vector<Map::ActiveMap>getActiveMapList();
};
