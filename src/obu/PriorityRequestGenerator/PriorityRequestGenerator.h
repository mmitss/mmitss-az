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
  1. This script is the header file for PriorrityRequestGenerator.cpp
*/

#pragma once
#include <list>
#include <string>
#include "BasicVehicle.h"
#include "SignalStatus.h"
#include "SignalRequest.h"
#include "ActiveRequest.h"
#include "MapManager.h"

class PriorityRequestGenerator
{
private:
  std::vector<ActiveRequest> ActiveRequestTable;
  std::vector<Map::ActiveMap> activeMapList;

  bool bgetActiveMap{false};      //This variables will be used by while checking if vehicle needs to send srm or not. If there is active map the value of this variable will true
  bool bRequestSendStatus{false}; //Required for HMI json
  std::string mapFileDirectory{};
  std::string mapFileName{};
  int messageType{};
  int temporaryVehicleID{};
  int vehicleLaneID{};
  int vehicleAprroachID{};
  int intersectionID{};
  int regionalID{};
  int signalGroup{};
  int vehicleIntersectionStatus{};
  int msgCount{};
  int vehicleType{};
  int basicVehicleRole{};
  int priorityRequestType{};
  int counter_VehicleInMap{};
  double time2go{};
  double tempVehicleSpeed{};    //tempVehicleSpeed store the vehicle speed of last send out srm. Use it to check if vehicle speed is changed or not.
  double tempSRMTimeStamp{};    //temporary store the time when last SRM has been sent
  int tempVehicleSignalGroup{}; //tempVehicleSignalGroup store the vehicle signalGroup of last send out srm. Use it to check if signalGroup is changed or not.

public:
  PriorityRequestGenerator();
  ~PriorityRequestGenerator();
  std::vector<Map::AvailableMap> availableMapList;
  std::vector<ActiveRequest> creatingSignalRequestTable(SignalStatus signalStatus);
  std::string createSRMJsonObject(BasicVehicle basicVehicle, SignalRequest signalRequest, MapManager mapManager);

  bool addToActiveRequestTable(SignalStatus signalStatus);
  bool shouldSendOutRequest(BasicVehicle basicVehicle);

  void setIntersectionID(int vehicleNearByIntersectionId);
  void setRegionalID(int vehicleNearByRegionalId);
  void setLaneID(int laneId);
  void setApproachID(int approachID);
  void setSignalGroup(int phaseNo);
  bool setTime2Go(double distance2go, double vehicleSpeed);
  void setVehicleIntersectionStatus(int vehIntersectionStatus);
  int getMessageType(std::string jsonString);
  std::vector<Map::ActiveMap> getActiveMapList(MapManager mapManager);
  // std::vector<Map::AvailableMap> getAvailableMapList(MapManager mapManager);
  void getVehicleInformationFromMAP(MapManager mapManager, BasicVehicle basicVehicle);
  int getIntersectionID();
  int getRegionalID();
  int getVehicleID(BasicVehicle basicVehicle);
  int getLaneID();
  int getApproachID();
  int getSignalGroup();
  double getTime2Go();
  int getVehicleIntersectionStatus();
  int getVehicleType();
  int getBasicVehicleRole();
  int getPriorityRequestType(BasicVehicle basicVehicle, MapManager mapManager);
  int getMinuteOfYear();
  int getMsOfMinute();
  int getMsgCount();
  int getVehicleCurrentSignalGroup();
  std::string getVehicleMapStatus();
  std::string getVehicleRequestSentStatus();
  std::vector<ActiveRequest> getActiveRequestTable();
  // std::vector<Map::ActiveMap> getActiveMapListFORHMI();
  void printART();
  std::vector<Map::AvailableMap> changeMapStatusInAvailableMapList(MapManager mapManager);
};