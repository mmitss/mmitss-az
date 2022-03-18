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
#include <fstream>
#include <iomanip>
#include <string>
#include "BasicVehicle.h"
#include "SignalStatus.h"
#include "SignalRequest.h"
#include "ActiveRequest.h"
#include "MapManager.h"
#include "BusStopInformation.h"
#include "msgEnum.h"
#include "json/json.h"
#include "Timestamp.h"

using std::cout;
using std::endl;
using std::fixed;
using std::setprecision;
using std::showpoint;
using std::string;
using std::vector;
using std::ofstream;
using std::ifstream;

#define EmergencyVehicle 2
#define Transit 6
#define Truck 9
#define SRM_TIME_GAP_VALUE 2.0
#define DISTANCEUNITCONVERSION 100
#define SECOND_MINTUTE_CONVERSION 60.0
#define HOUR_DAY_CONVERSION 24
#define MINTUTE_HOUR_CONVERSION 60
#define SECOND_MILISECOND_CONVERSION 1000.0
#define Degree_Conversion 10000000.0
#define maxMsgCount 127
#define minMsgCount 1
#define minimumETA 2.0
#define minimumETA_Duration 4
#define vehicleStartUpLossTime 2.0
#define minimumVehicleSpeed 4.0
#define ALLOWED_SPEED_DEVIATION 4.0
#define ALLOWED_ETA_DIFFERENCE 6.0


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

    bool activeMapStatus{false};   //This variables will be used by while checking if vehicle needs to send srm or not. If there is active map the value of this variable will true
    bool requestSendStatus{false}; //Required for HMI json
    bool logging{false};
    bool consoleOutput{false};
    bool lightSirenStatus{false};
    bool busStopPassedStatus{false};
    string mapFileDirectory{};
    string mapFileName{};
    string logFileName{};
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
    int tempVehicleSignalGroup{}; //tempVehicleSignalGroup store the vehicle signalGroup of last send out srm. Use it to check if signalGroup is changed or not.
    double tempVehicleSpeed{};    //tempVehicleSpeed store the vehicle speed of last send out srm. Use it to check if vehicle speed is changed or not.
    double vehicleDistanceFromStopBar{};
    double vehicleETA{};
    double vehicleSpeed{};
    double srmSendingTime{}; //temporary store the time when last SRM has been sent
    double requestTimedOutValue{};
    double busStopLattitude{};
    double busStopLongitude{};
    double busStopElevation{};
    double busStopHeading{};
    double mapReferenceLattitude{};
    double mapReferenceLongitue{};


public:
    PriorityRequestGenerator();
    ~PriorityRequestGenerator();
    vector<Map::AvailableMap> availableMapList;
    vector<ActiveRequest> creatingSignalRequestTable(SignalStatus signalStatus);
    vector<ActiveRequest> getActiveRequestTable();
    vector<Map::AvailableMap> manageMapStatusInAvailableMapList(MapManager mapManager);
    vector<Map::ActiveMap> getActiveMapList(MapManager mapManager);
    string createSRMJsonString(BasicVehicle basicVehicle, SignalRequest signalRequest, MapManager mapManager);
    string getVehicleMapStatus();
    string getVehicleRequestSentStatus();
    void getVehicleInformationFromMAP(MapManager mapManager, BasicVehicle basicVehicle);
    void readConfigFile();
    void loggingData(string logString);
    void displayConsoleData(string consoleString);
    void setIntersectionID(int vehicleNearByIntersectionId);
    void setRegionalID(int vehicleNearByRegionalId);
    void setVehicleID(BasicVehicle basicVehicle);
    void setVehicleSpeed(BasicVehicle basicVehicle);
    void setLaneID(int laneId);
    void setApproachID(int approachID);
    void setSignalGroup(int phaseNo);
    void setETA(double distance2go, double vehicle_Speed);
    void setVehicleIntersectionStatus(int vehIntersectionStatus);
    void setVehicleType();
    void setSimulationVehicleType(string vehType); //For PRGServer
    void setBasicVehicleRole(int vehicle_Type);
    void setPriorityRequestType(int priority_Request_Type);
    void setLightSirenStatus(string jsonString);
    void setMsgCount(int msg_count);
    void printActiveRequestTable();
    void getBusStopInformation();
    void clearActiveMapInformation(MapManager mapManager);
    int getMessageType(string jsonString);   
    int getIntersectionID();
    int getRegionalID();
    int getVehicleID();
    int getLaneID();
    int getApproachID();
    int getSignalGroup();
    int getVehicleIntersectionStatus();
    int getVehicleType();
    int getBasicVehicleRole();
    int getPriorityRequestType();
    int getMinuteOfYear();
    int getMsOfMinute();
    int getMsgCount();
    int getActiveMapStatus();
    double getVehicleSpeed();
    double getVehicleDistanceFromStopBar();
    double getETA();
    double getRequestTimedOutValue();
    bool addToActiveRequestTable(SignalStatus signalStatus);
    bool checkPriorityRequestSendingRequirementStatus();
    bool checkRequestSendingRequirement();                                         //This overloading function will be used for Truck
    bool checkRequestSendingRequirement(vector<BusStopInformation> bus_Stop_List); //This overloading function will be used for Transit
    bool checkRequestSendingRequirement(bool light_Siren_Status);                  //This overloading function will be used for EmergencyVehicle
    bool findNearestBusStopLocation();
    bool checkPassedNearestBusStop();
};