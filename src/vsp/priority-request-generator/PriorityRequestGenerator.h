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
#include <iostream>
#include <iomanip>
#include <list>
#include <string>
#include <vector>
#include <chrono>
#include "BasicVehicle.h"
#include "SignalStatus.h"
#include "SignalRequest.h"
#include "ActiveRequest.h"
#include "MapManager.h"
#include "BusStopInformation.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;

#define EmergencyVehicle 2
#define Transit 6
#define Truck 9
#define Vehicle_Length 6.096
#define SrmTimeGapValue 2.0
#define DISTANCEUNITCONVERSION 100
#define SECONDSINAMINUTE 60.0
#define HOURSINADAY 24
#define	MINUTESINAHOUR 60
#define	SECONDTOMILISECOND 1000
#define Degree_Conversion 10000000.0
#define	maxMsgCount 127
#define	minMsgCount 1
#define minimumETA 3.0
#define minimumETA_Duration 4.0
#define minimumVehicleSpeed 4.0
#define vehicleSpeedDeviationLimit 4.0
#define allowed_ETA_Difference 6.0
#define Bus_Stop_Distance_Allowance 6.0
#define kmToMeter 1000

enum msgType
{
  lightSirenStatus = 1,
};

class PriorityRequestGenerator
{
private:
  vector<ActiveRequest> ActiveRequestTable;
  vector<Map::ActiveMap> activeMapList;
  vector<BusStopInformation> busStopList;

  bool activeMapStatus{false};      //This variables will be used by while checking if vehicle needs to send srm or not. If there is active map the value of this variable will true
  bool requestSendStatus {false}; //Required for HMI json
  bool loggingStatus{false};
  bool lightSirenStatus{false};
  bool busStopPassedStatus{false};
  string mapFileDirectory{};
  string mapFileName{};
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
  double vehicleDistanceFromStopBar{};
  double vehicleETA{};
  double vehicleSpeed{};
  double SrmSendingTime{};    //temporary store the time when last SRM has been sent
  double requestTimedOutValue{};
  double busStopLattitude{};
  double busStopLongitude{};
  double mapReferenceLattitude{};
  double mapReferenceLongitue{};
  // double startPointBusStopLattitude{};
  // double startPointBusStopLongitude{};
  // double endPointBusStopLattitude{};
  // double endPointBusStopLongitude{};
  // double busStopLength{};


public:
  PriorityRequestGenerator();
  ~PriorityRequestGenerator();
  std::vector<Map::AvailableMap> availableMapList;
  std::vector<ActiveRequest> creatingSignalRequestTable(SignalStatus signalStatus);
  std::string createSRMJsonObject(BasicVehicle basicVehicle, SignalRequest signalRequest, MapManager mapManager);
  bool addToActiveRequestTable(SignalStatus signalStatus);
  bool shouldSendOutRequest();
  bool checkRequestSendingRequirement(); //This overloading function will be used for Truck
  bool checkRequestSendingRequirement(vector<BusStopInformation> bus_Stop_List); //This overloading function will be used for Transit
  bool checkRequestSendingRequirement(bool light_Siren_Status); //This overloading function will be used for EmergencyVehicle
  bool findNearestBusStopLocation();
  bool checkPassedNearestBusStop();
  bool getLoggingStatus();
  void loggingData(std::string jsonString);
  void setIntersectionID(int vehicleNearByIntersectionId);
  void setRegionalID(int vehicleNearByRegionalId);
  void setVehicleID(BasicVehicle basicVehicle);
  void setVehicleSpeed(BasicVehicle basicVehicle);
  void setLaneID(int laneId);
  void setApproachID(int approachID);
  void setSignalGroup(int phaseNo);
  void setTime2Go(double distance2go, double vehicle_Speed);
  void setVehicleIntersectionStatus(int vehIntersectionStatus);
  void setVehicleType();
  void setSimulationVehicleType(std::string vehType); //For PRGServer
  void setPriorityRequestType(int priority_Request_Type);
  void setLightSirenStatus(std::string jsonString);
  void setMsgCount(int msg_count);
  int getMessageType(std::string jsonString);
  std::vector<Map::ActiveMap> getActiveMapList(MapManager mapManager);
  void getVehicleInformationFromMAP(MapManager mapManager, BasicVehicle basicVehicle);
  int getIntersectionID();
  int getRegionalID();
  int getVehicleID();
  double getVehicleSpeed();
  int getLaneID();
  int getApproachID();
  int getSignalGroup();
  double getTime2Go();
  int getVehicleIntersectionStatus();
  int getVehicleType();
  int getBasicVehicleRole();
  int getPriorityRequestType();
  // int getPriorityRequestType(BasicVehicle basicVehicle, MapManager mapManager);
  int getMinuteOfYear();
  int getMsOfMinute();
  int getMsgCount();
  int getActiveMapStatus();
  double getRequestTimedOutValue();
  double getHaversineDistance(double lat1, double lon1, double lat2, double lon2);
  std::string getVehicleMapStatus();
  std::string getVehicleRequestSentStatus();
  std::vector<ActiveRequest> getActiveRequestTable();
  void printART();
  std::vector<Map::AvailableMap> manageMapStatusInAvailableMapList(MapManager mapManager);
  void getBusStopInformation();
  void clearActiveMapInformation(MapManager mapManager);
};