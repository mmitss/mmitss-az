/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorrityRequestServer.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the header file for PriorrityRequestServer.cpp
*/

#pragma once

#include <chrono>
#include "SignalRequest.h"
#include "SignalStatus.h"
#include "ActiveRequest.h"

using std::cout;
using std::endl;
using std::ifstream;
using std::string;
using std::vector;

#define coordinationVehicleType 20
#define Minimum_ETA 1.0
#define ETA_Delete_Time 1.0
#define TIME_GAP_BETWEEN_ETA_Update 1
#define SEQUENCE_NUMBER_MINLIMIT 1
#define SEQUENCE_NUMBER_MAXLIMIT 127
#define HOURSINADAY 24
#define MINUTESINAHOUR 60
#define SECONDSINAMINUTE 60
#define SECONDTOMILISECOND 1000

enum msgType
{
    coordinationRequest = 1,
};

class PriorityRequestServer
{
private:
    vector<ActiveRequest> ActiveRequestTable;
    int minuteOfYear{};
    int msOfMinute{};
    int regionalID;
    int intersectionID{};
    int sequenceNumber{};
    int updateCount{};
    int vehicleType{};
    int priorityRequestStatus{};
    int requestTimedOutVehicleID{};
    int tempLastTimeETAUpdated{};
    int msgReceived{};
    int msgServed{};
    int msgRejected{};
    int msgSentTime{};
    double expectedTimeOfArrivalToStopBar{0.0};
    double requestTimedOutValue{0.0};
    double timeInterval{0.0};
    bool bLogging{};
    bool emergencyVehicleStatus{false};
    bool sentClearRequest{};
    string intersectionName{};

public:
    PriorityRequestServer();
    ~PriorityRequestServer();

    string createSSMJsonString(SignalStatus signalStatus);
    string createJsonStringForPrioritySolver();
    string createJsonStringForSystemPerformanceDataLog();
    void manageSignalRequestTable(SignalRequest signalRequest);
    void manageCoordinationRequest(string jsonString);
    void deleteTimedOutRequestfromActiveRequestTable();
    void updateETAInActiveRequestTable();
    void deleteCoordinationRequestFromList();
    void printActiveRequestTable();
    void setRequestTimedOutVehicleID(int timedOutVehicleID);
    void setPriorityRequestStatus();
    void setPRSUpdateCount();
    void setVehicleType(SignalRequest signalRequest);
    void setSrmMessageStatus(SignalRequest signalRequest);
    void loggingData(string jsonString);
    int getMessageType(string jsonString);
    int getIntersectionID();
    int getRegionalID();
    int getRequestTimedOutVehicleID();
    int getMinuteOfYear();
    int getMsOfMinute();
    int getPRSSequenceNumber();
    int getPRSUpdateCount();
    int getSignalGroup(SignalRequest signalRequest);
    int getSplitPhase(int signalGroup);
    bool acceptSignalRequest(SignalRequest signalRequest);
    bool addToActiveRequestTable(SignalRequest signalRequest);
    bool updateActiveRequestTable(SignalRequest signalRequest);
    bool deleteRequestfromActiveRequestTable(SignalRequest signalRequest);
    bool shouldDeleteTimedOutRequestfromActiveRequestTable();
    bool updateETA();
    bool sendClearRequest();
    bool findEVInList();
    bool findEVInRequest(SignalRequest signalRequest);
    bool sendSystemPerformanceDataLog();
};