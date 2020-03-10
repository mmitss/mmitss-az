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

#include "SignalRequest.h"
#include "SignalStatus.h"
#include "ActiveRequest.h"

class PriorityRequestServer
{
private:
    std::vector<ActiveRequest> ActiveRequestTable;
    int messageType{};
    int minuteOfYear{};
    int msOfMinute{};
    int regionalID;
    int intersectionID{};
    int sequenceNumber{};
    int updateCount{};
    int vehicleType{};
    int priorityRequestStatus{};
    double expectedTimeOfArrivalToStopBar{0.0};
    double requestTimedOutValue{0.0};
    int requestTimedOutVehicleID{};
    int tempLastTimeETAUpdated{};

public:
    PriorityRequestServer();
    ~PriorityRequestServer();

    std::string createSSMJsonString(SignalStatus signalStatus);
    std::string createJsonStringForPrioritySolver();
    void managingSignalRequestTable(SignalRequest signalRequest);
    void findSplitPhase();
    void deleteTimedOutRequestfromActiveRequestTable();
    void updateETAInActiveRequestTable();
    void writeMAPPayloadInFile();
    void deleteMapPayloadFile();
    void printvector();
    void setRequestTimedOutVehicleID(int timedOutVehicleID);
    void setPriorityRequestStatus();
    void setPRSUpdateCount();
    void setVehicleType(SignalRequest signalRequest);
    int getMessageType(std::string jsonString);
    int getIntersectionID();
    int getRegionalID();
    int getRequestTimedOutVehicleID();
    int getMinuteOfYear();
    int getMsOfMinute();
    int getPRSSequenceNumber();
    int getPRSUpdateCount();
    int getSignalGroup(SignalRequest signalRequest);
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
};