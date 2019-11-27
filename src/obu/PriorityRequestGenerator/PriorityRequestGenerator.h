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
    std::vector<ActiveRequest>ActiveRequestTable;
    std::vector<Map::ActiveMap> activeMapList;
    bool bgetActiveMap{false}; //This variables will be used by while checking if vehicle needs to send srm or not. If there is active map the value of this variable will true
    bool bRequestSendStatus{false};  //Required for HMI json
    std::string mapFileDirectory; 
    std::string mapFileName;
    int messageType{};
    int temporaryVehicleID{};
    int vehicleLaneID{};
    int vehicleAprroachID{};
    int intersectionID{};
    int regionalID{};
    int vehicleIntersectionStatus{};
    int msgCount{};
    int vehicleType{};
    int basicVehicleRole{};
    int priorityRequestType{};
    int counter_VehicleInMap{};
    double time2go{};
    double tempVehicleSpeed{}; //tempVehicleSpeed store the vehicle speed of last send out srm. use it to check the speed change. will be set vehicle minimum speed when out of the intersection
    double tempSRMTimeStamp{}; //temporary store the time when last SRM has been sent
    
    
public:
    PriorityRequestGenerator();
    ~PriorityRequestGenerator();

    std::vector<ActiveRequest> creatingSignalRequestTable(SignalStatus signalStatus);
    std::string createSRMJsonObject(BasicVehicle basicVehicle, SignalRequest signalRequest, MapManager mapManager);

    bool addToActiveRequestTable(SignalStatus signalStatus);
    bool shouldSendOutRequest(BasicVehicle basicVehicle);

    void setIntersectionID(int vehicleNearByIntersectionId);
    void setRegionalID(int vehicleNearByRegionalId);
    void setLaneID(int laneId);
    void setApproachID(int approachID);
    bool setTime2Go(double distance2go, double vehicleSpeed);
    void setVehicleIntersectionStatus(int vehIntersectionStatus);
    int getMessageType(std::string jsonString);
    std::vector<Map::ActiveMap> getActiveMapList(MapManager mapManager);
    void getVehicleInformationFromMAP(MapManager mapManager, BasicVehicle basicVehicle);    
    int getIntersectionID();
    int getRegionalID();
    int getVehicleID(BasicVehicle basicVehicle);
    int getLaneID();
    int getApproachID();
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
    std::vector<ActiveRequest>getActiveRequestTable();
    void printART();
    

};