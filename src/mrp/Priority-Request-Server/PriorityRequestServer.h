#pragma once

#include "SignalRequest.h"
#include "SignalStatus.h"
#include "ActiveRequest.h"

class PriorityRequestServer
{
private:
    std::vector<ActiveRequest> ActiveRequestTable;
    std::vector<int> phaseGroup;
    int messageType{};
    int minuteOfYear{};
    int msOfMinute{};
    int regionalID;
    int intersectionID{};
    int sequenceNumber{};
    int updateCount{};
    int priorityRequestStatus{};
    double expectedTimeOfArrivalToStopBar{0.0};
    int requestTimedOutVehicleID{};
    int tempLastTimeETAUpdated{};

public:
    PriorityRequestServer();
    ~PriorityRequestServer();

    int getMessageType(std::string jsonString);
    int getIntersectionID();
    int getRegionalID();
    bool aceeptSignalRequest(SignalRequest signalRequest);
    bool addToActiveRequesttable(SignalRequest signalRequest);
    bool updateActiveRequestTable(SignalRequest signalRequest);
    bool deleteRequestfromActiveRequestTable(SignalRequest signalRequest);
    bool shouldDeleteTimedOutRequestfromActiveRequestTable();
    bool findEVInList();
    // void findSplitPhase();
    // void modifyPriorityRequestList();
    void setRequestTimedOutVehicleID(int timedOutVehicleID);
    int getRequestTimedOutVehicleID();
    std::vector<ActiveRequest> creatingSignalRequestTable(SignalRequest signalRequest);
    void deleteTimedOutRequestfromActiveRequestTable();
    void updateETAInActiveRequestTable();
    void printvector();
    void setPriorityRequestStatus(); //Check with Dr. Head
    //int getPriorityRequestStatus();
    int getMinuteOfYear();
    int getMsOfMinute();
    int getPRSSequenceNumber();
    int getPRSUpdateCount(SignalRequest signalRequest);
    int getSignalGroup(SignalRequest signalRequest);
    void writeMAPPayloadInFile();
    void deleteMapPayloadFile();
    void getPhaseGroup(std::string jsonString);
    std::string createSSMJsonString(SignalRequest signalRequest, SignalStatus signalStatus);
};