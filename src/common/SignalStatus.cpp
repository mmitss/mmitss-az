/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  SignalStatus.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script will get the values of all the attributes to create SSM JSON Object and convert the SSM JSON Object into Signal Status object
*/

#include <iostream>
#include <iomanip>
#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"
#include "json/json.h"
#include "SignalStatus.h"
#include "Timestamp.h"

const int ETA_CONVERTION = 60;

//Constructor
SignalStatus::SignalStatus()
{
}

//Setters:
void SignalStatus::setNoOfRequest(int artsize)
{
    // std::vector<ActiveRequest> ActiveRequestTable;
     
    // artsize = ActiveRequestTable.size();
    noOfRequest = artsize;
}

void SignalStatus::setMinuteOfYear(int vehMinuteOfYear)
{
    minuteOfYear = vehMinuteOfYear;
}

void SignalStatus::setMsOfMinute(int vehMsOfMinute)
{
    msOfMinute = vehMsOfMinute;
}

void SignalStatus::setSequenceNumber(int prsSequenceNumber)
{
    sequenceNumber = prsSequenceNumber;
}

void SignalStatus::setUpdateCount(int prsUpdateCount)
{
    updateCount = prsUpdateCount;
}

void SignalStatus::setRegionalID(int vehRegionalID)
{
    regionalID = vehRegionalID;
}

void SignalStatus::setIntersectionID(int vehIntersectionID)
{
    intersectionID = vehIntersectionID;
}

void SignalStatus::setTemporaryVechileID(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (size_t i = 0; i < ActiveRequestTable.size(); i++)
    {
        vehicleID.push_back(ActiveRequestTable[i].vehicleID);
    }
}

void SignalStatus::setRequestID(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (size_t i = 0; i < ActiveRequestTable.size(); i++)
    {
        requestID.push_back(ActiveRequestTable[i].requestID);
    }
}

void SignalStatus::setMsgCount(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (size_t i = 0; i < ActiveRequestTable.size(); i++)
    {
        msgCount.push_back(ActiveRequestTable[i].msgCount);
    }
}

void SignalStatus::setBasicVehicleRole(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (size_t i = 0; i < ActiveRequestTable.size(); i++)
    {
        basicVehicleRole.push_back(ActiveRequestTable[i].basicVehicleRole);
    }
}

void SignalStatus::setInBoundLaneIntersectionAccessPoint(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (size_t i = 0; i < ActiveRequestTable.size(); i++)
    {
        inBoundLaneID.push_back(ActiveRequestTable[i].vehicleLaneID);
        inBoundApproachID.push_back(ActiveRequestTable[i].vehicleApproachID);
    }
}

void SignalStatus::setETA(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (size_t i = 0; i < ActiveRequestTable.size(); i++)
    {
        //expectedTimeOfArrival_Minute.push_back(static_cast<int>(ActiveRequestTable[i].vehicleETA / ETA_CONVERTION));
        //expectedTimeOfArrival_Second.push_back(fmod(ActiveRequestTable[i].vehicleETA, ETA_CONVERTION));
        expectedTimeOfArrival_Minute.push_back(ActiveRequestTable[i].vehicleETAMinute);
        expectedTimeOfArrival_Second.push_back(ActiveRequestTable[i].vehicleETAMinute);
        expectedTimeOfArrival_Duration.push_back(ActiveRequestTable[i].vehicleETADuration);
    }
}

void SignalStatus::setPriorityRequestStatus(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (size_t i = 0; i < ActiveRequestTable.size(); i++)
    {
        priorityRequestStatus.push_back(ActiveRequestTable[i].prsStatus);
    }
}

// Getters
int SignalStatus::getNoOfRequest()
{
    return noOfRequest;
}

int SignalStatus::getMinuteOfYear()
{
    return minuteOfYear;
}

int SignalStatus::getMsOfMinute()
{
    return msOfMinute;
}

int SignalStatus::getPRSSequenceNumber()
{
    return sequenceNumber;
}

int SignalStatus::getPRSUpdateCount()
{
    return updateCount;
}

int SignalStatus::getRegionalID()
{
    return regionalID;
}

int SignalStatus::getIntersectionID()
{
    return intersectionID;
}

std::vector<int> SignalStatus::getTemporaryVehicleID()
{
    return vehicleID;
}

std::vector<int> SignalStatus::getRequestID()
{
    return requestID;
}

std::vector<int> SignalStatus::getMsgCount()
{
    return msgCount;
}

std::vector<int> SignalStatus::getBasicVehicleRole()
{
    return basicVehicleRole;
}

std::vector<int> SignalStatus::getInBoundLaneID()
{
    return inBoundLaneID;
}

std::vector<int> SignalStatus::getInBoundApproachID()
{
    return inBoundApproachID;
}

std::vector<int> SignalStatus::getETA_Minute()
{
    return expectedTimeOfArrival_Minute;
}

std::vector<int> SignalStatus::getETA_Second()
{
    return expectedTimeOfArrival_Second;
}

std::vector<int> SignalStatus::getETA_Duration()
{
    return expectedTimeOfArrival_Duration;
}

std::vector<int> SignalStatus::getPriorityRequestStatus()
{
    return priorityRequestStatus;
}

void SignalStatus::reset()
{
    noOfRequest = 0;
    minuteOfYear = 0;
    msOfMinute = 0;
    sequenceNumber = 0;
    updateCount = 0;
    regionalID = 0;
    intersectionID = 0;
    vehicleID.clear();
    requestID.clear();
    msgCount.clear();
    inBoundLaneID.clear();
    inBoundApproachID.clear();
    basicVehicleRole.clear();
    expectedTimeOfArrival_Minute.clear();
    expectedTimeOfArrival_Second.clear();
    expectedTimeOfArrival_Duration.clear();
    priorityRequestStatus.clear();
}

std::string SignalStatus::signalStatus2Json(std::vector<ActiveRequest> ActiveRequestTable)
{
    Json::Value jsonObject;
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";
    std::string jsonString{};

    jsonObject["Timestamp_verbose"] = getVerboseTimestamp();
    jsonObject["Timestamp_posix"] = getPosixTimestamp();
    jsonObject["MsgType"] = "SSM";
    jsonObject["noOfRequest"] = ActiveRequestTable.size();
    jsonObject["SignalStatus"]["minuteOfYear"] = minuteOfYear;
    jsonObject["SignalStatus"]["msOfMinute"] = msOfMinute;
    jsonObject["SignalStatus"]["sequenceNumber"] = sequenceNumber;
    jsonObject["SignalStatus"]["updateCount"] = updateCount;
    jsonObject["SignalStatus"]["regionalID"] = regionalID;
    jsonObject["SignalStatus"]["intersectionID"] = intersectionID;
    for (unsigned int i = 0; i < ActiveRequestTable.size(); i++)
    {
        jsonObject["SignalStatus"]["requestorInfo"][i]["vehicleID"] = ActiveRequestTable[i].vehicleID;
        jsonObject["SignalStatus"]["requestorInfo"][i]["requestID"] = ActiveRequestTable[i].requestID;
        jsonObject["SignalStatus"]["requestorInfo"][i]["msgCount"] = ActiveRequestTable[i].msgCount;
        jsonObject["SignalStatus"]["requestorInfo"][i]["basicVehicleRole"] = ActiveRequestTable[i].basicVehicleRole;
        jsonObject["SignalStatus"]["requestorInfo"][i]["inBoundLaneID"] = ActiveRequestTable[i].vehicleLaneID;
        jsonObject["SignalStatus"]["requestorInfo"][i]["inBoundApproachID"] = ActiveRequestTable[i].vehicleApproachID;
        jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Minute"] = ActiveRequestTable[i].vehicleETAMinute;
        jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Second"] = ActiveRequestTable[i].vehicleETASecond;
        jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Duration"] = ActiveRequestTable[i].vehicleETADuration;
        jsonObject["SignalStatus"]["requestorInfo"][i]["priorityRequestStatus"] = ActiveRequestTable[i].prsStatus;
    }

    jsonString = Json::writeString(builder, jsonObject);
    return jsonString;
}

void SignalStatus::json2SignalStatus(std::string jsonString)
{
    Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	std::string errors{};

    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;

    const Json::Value values = jsonObject["SignalStatus"]["requestorInfo"];

    noOfRequest = (jsonObject["noOfRequest"]).asInt();
    minuteOfYear = (jsonObject["SignalStatus"]["minuteOfYear"]).asInt();
    msOfMinute = (jsonObject["SignalStatus"]["msOfMinute"]).asInt();
    sequenceNumber = (jsonObject["SignalStatus"]["sequenceNumber"]).asInt();
    updateCount = (jsonObject["SignalStatus"]["updateCount"]).asInt();
    regionalID = (jsonObject["SignalStatus"]["regionalID"]).asInt();
    intersectionID = (jsonObject["SignalStatus"]["intersectionID"]).asInt();
    for (int i = 0; i < noOfRequest; i++)
    {
        for (size_t j = 0; j < values[i].getMemberNames().size(); j++)
        {
            if (values[i].getMemberNames()[j] == "vehicleID")
                vehicleID.push_back(values[i][values[i].getMemberNames()[j]].asInt());

            if (values[i].getMemberNames()[j] == "requestID")
                requestID.push_back(values[i][values[i].getMemberNames()[j]].asInt());

            if (values[i].getMemberNames()[j] == "msgCount")
                msgCount.push_back(values[i][values[i].getMemberNames()[j]].asInt());

            if (values[i].getMemberNames()[j] == "basicVehicleRole")
                basicVehicleRole.push_back(values[i][values[i].getMemberNames()[j]].asInt());

            if (values[i].getMemberNames()[j] == "inBoundLaneID")
                inBoundLaneID.push_back(values[i][values[i].getMemberNames()[j]].asInt());
            
            if (values[i].getMemberNames()[j] == "inBoundApproachID")
                inBoundApproachID.push_back(values[i][values[i].getMemberNames()[j]].asInt());

            if (values[i].getMemberNames()[j] == "ETA_Minute")
                expectedTimeOfArrival_Minute.push_back(values[i][values[i].getMemberNames()[j]].asInt());

            if (values[i].getMemberNames()[j] == "ETA_Second")
                expectedTimeOfArrival_Second.push_back(values[i][values[i].getMemberNames()[j]].asInt());

            if (values[i].getMemberNames()[j] == "ETA_Duration")
                expectedTimeOfArrival_Duration.push_back(values[i][values[i].getMemberNames()[j]].asInt());

            if (values[i].getMemberNames()[j] == "priorityRequestStatus")
                priorityRequestStatus.push_back(values[i][values[i].getMemberNames()[j]].asInt());
        }
    }
}



SignalStatus::~SignalStatus()
{
}
