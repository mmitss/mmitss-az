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
#include <string>
#include "BasicVehicle.h"
#include "Position3D.h"
#include "Map.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ofstream;
using std::stringstream;

class MapManager
{
private:
  vector<Map::ActiveMap> activeMapList;
  string mapPayload{};
  string intersectionMapName{};
  string timedOutMapPayLoad{};
  int intersectinID{};
  int regionalID{};
  double mapReferenceLatitude{};
  double mapReferenceLongitude{};
  double mapReferenceElevation{};

public:
  MapManager();
  ~MapManager();
  vector<Map::AvailableMap> availableMapList;
  void json2MapPayload(string jsonString);
  void setTimedOutMapPayLoad(string timedOutPayLoad);
  string getTimedOutMapPayLoad();
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
  vector<Map::ActiveMap> getActiveMapList();
  vector<Map::AvailableMap> getAvailableMapList();
  void updateMapAge();
};
