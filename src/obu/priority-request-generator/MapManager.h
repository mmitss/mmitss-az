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
  std::vector<Map::ActiveMap> activeMapList;
  std::string mapPayload{};
  std::string intersectionMapName{};
  std::string timedOutMapPayLoad{};
  int intersectinID{};
  int regionalID{};
  double mapReferenceLatitude{};
  double mapReferenceLongitude{};
  double mapReferenceElevation{};

public:
  MapManager();
  ~MapManager();
  std::vector<Map::AvailableMap> availableMapList;
  void json2MapPayload(std::string jsonString);
  void setTimedOutMapPayLoad(std::string timedOutPayLoad);
  std::string getTimedOutMapPayLoad();
  void writeMAPPayloadInFile();
  bool addToMapInList();
  bool updateMapPayLoadList();
  bool deleteMapPayLoadFromList();
  int getMapPayloadReceivedTime();
  int getMapPayloadReceivedSecondOfMinute();
  void getReferencePoint();          //Required for PRGServer
  double getMapReferenceLatitude();  //Required for PRGServer
  double getMapReferenceLongitude(); //Required for PRGServer
  double getMapReferenceElevation(); //Required for PRGServer
  void maintainAvailableMapList();
  void deleteMap();
  void printAvailableMapList();
  void createActiveMapList(BasicVehicle basicVehicle);
  void deleteActiveMapfromList();
  std::vector<Map::ActiveMap> getActiveMapList();
  std::vector<Map::AvailableMap> getAvailableMapList();
  void updateMapAge();
};
