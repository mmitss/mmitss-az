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

class PriorityRequestServer
{
private:
    std::vector<ActiveRequest> ActiveRequestTable;
    // int messageType{};
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
    std::string intersectionName{};

public:
    PriorityRequestServer();
    ~PriorityRequestServer();

    std::string createSSMJsonString(SignalStatus signalStatus);
    std::string createJsonStringForPrioritySolver();
    std::string createJsonStringForSystemPerformanceDataLog();
    void managingSignalRequestTable(SignalRequest signalRequest);
    void deleteTimedOutRequestfromActiveRequestTable();
    void updateETAInActiveRequestTable();
    void writeMAPPayloadInFile();
    void deleteMapPayloadFile();
    void printvector();
    void setRequestTimedOutVehicleID(int timedOutVehicleID);
    void setPriorityRequestStatus();
    void setPRSUpdateCount();
    void setVehicleType(SignalRequest signalRequest);
    void setSrmMessageStatus(SignalRequest signalRequest);
    void loggingData(std::string jsonString);
    int getMessageType(std::string jsonString);
    int getIntersectionID();
    int getRegionalID();
    int getRequestTimedOutVehicleID();
    int getMinuteOfYear();
    int getMsOfMinute();
    int getPRSSequenceNumber();
    int getPRSUpdateCount();
    int getSignalGroup(SignalRequest signalRequest);
    int getSplitPhase(int signalGroup);
    double getRequestTimedOutValue();
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