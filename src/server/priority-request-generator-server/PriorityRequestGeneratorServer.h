/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorrityRequestGeneratorServer.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the header file for PriorrityRequestGeneratorServer.cpp
*/
#pragma once
#include <iostream>
#include <vector>
#include "ServerList.h"
#include "BasicVehicle.h"
#include "SignalStatus.h"
#include "SignalRequest.h"
#include "json/json.h"
#include "msgEnum.h"

using std::cout;
using std::endl; 
using std::vector;
using std::string;
using std::ifstream;

class PriorityRequestGeneratorServer
{
private:
    vector<ServerList> PRGServerList;
    int timedOutVehicleID{};
    string srmSendingJsonString{};
    string prgStatusSendingJsonString{};
    bool sendSRM{false};
    ServerList vehicleinfo;

public:
    PriorityRequestGeneratorServer();
    ~PriorityRequestGeneratorServer();

    void managingPRGServerList(BasicVehicle basicVehicle);
    void processBSM(BasicVehicle basicVehicle);
    void processMap(string jsonString, MapManager mapManager);
    void processSSM(string jsonString);
    void deleteTimedOutVehicleInformationFromPRGServerList();
    void setTimedOutVehicleID(int vehicleId);
    void printPRGServerList();
    bool checkAddVehicleIDToPRGServerList(BasicVehicle basicVehicle);
    bool checkUpdateVehicleIDInPRGServerList(BasicVehicle basicVehicle);
    bool checkDeleteTimedOutVehicleIDFromList();
    bool checkSrmSendingFlag();
    int getMessageType(string jsonString);
    int getTimedOutVehicleID();
    double getCurrentTimeInSeconds();
    double haversineDistance(double lat1, double lon1, double lat2, double lon2);
    string getSRMJsonString();
    string getPrgStatusJsonString();
};


