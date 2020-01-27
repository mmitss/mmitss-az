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

const int ETA_CONVERTION = 60;
const int ETA_DURATION = 2000;

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
    for (int i = 0; i < getNoOfRequest(); i++)
    {
        vehicleID[i] = ActiveRequestTable[i].vehicleID;
    }
}

void SignalStatus::setRequestID(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (int i = 0; i < getNoOfRequest(); i++)
    {
        requestID[i] = ActiveRequestTable[i].requestID;
    }
}

void SignalStatus::setMsgCount(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (int i = 0; i < getNoOfRequest(); i++)
    {
        msgCount[i] = ActiveRequestTable[i].msgCount;
    }
}

void SignalStatus::setBasicVehicleRole(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (int i = 0; i < getNoOfRequest(); i++)
    {
        basicVehicleRole[i] = ActiveRequestTable[i].basicVehicleRole;
    }
}

void SignalStatus::setInBoundLaneIntersectionAccessPoint(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (int i = 0; i < getNoOfRequest(); i++)
    {
        inBoundLaneID[i] = ActiveRequestTable[i].vehicleLaneID;
        inBoundApproachID[i] = ActiveRequestTable[i].vehicleApproachID;
    }
}

void SignalStatus::setETA(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (int i = 0; i < getNoOfRequest(); i++)
    {
        expectedTimeOfArrival_Minute[i] = int(ActiveRequestTable[i].vehicleETA / ETA_CONVERTION);
        expectedTimeOfArrival_Second[i] = fmod(ActiveRequestTable[i].vehicleETA, ETA_CONVERTION);
        expectedTimeOfArrival_Duration[i] = ETA_DURATION;
    }
}

void SignalStatus::setPriorityRequestStatus(std::vector<ActiveRequest> ActiveRequestTable)
{
    for (int i = 0; i < getNoOfRequest(); i++)
    {
        priorityRequestStatus[i] = ActiveRequestTable[i].prsStatus;
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

int *SignalStatus::getTemporaryVehicleID()
{
    return vehicleID;
}

int *SignalStatus::getRequestID()
{
    return requestID;
}

int *SignalStatus::getMsgCount()
{
    return msgCount;
}

int *SignalStatus::getBasicVehicleRole()
{
    return basicVehicleRole;
}

int *SignalStatus::getInBoundLaneID()
{
    return inBoundLaneID;
}

int *SignalStatus::getInBoundApproachID()
{
    return inBoundApproachID;
}

int *SignalStatus::getETA_Minute()
{
    return expectedTimeOfArrival_Minute;
}

double *SignalStatus::getETA_Second()
{
    return expectedTimeOfArrival_Second;
}

double *SignalStatus::getETA_Duration()
{
    return expectedTimeOfArrival_Duration;
}

int *SignalStatus::getPriorityRequestStatus()
{
    return priorityRequestStatus;
}

std::string SignalStatus::signalStatus2Json(std::vector<ActiveRequest> ActiveRequestTable)
{
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    std::string jsonString;

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
        jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Minute"] = int(ActiveRequestTable[i].vehicleETA / ETA_CONVERTION);
        jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Second"] = fmod(ActiveRequestTable[i].vehicleETA, ETA_CONVERTION);
        jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Duration"] = ETA_DURATION;
        jsonObject["SignalStatus"]["requestorInfo"][i]["priorityRequestStatus"] = ActiveRequestTable[i].prsStatus;
    }

    jsonString = fastWriter.write(jsonObject);
    return jsonString;
}

void SignalStatus::json2SignalStatus(std::string jsonString)
{
    Json::Value jsonObject;
    Json::Reader reader;

    reader.parse(jsonString.c_str(), jsonObject);
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
                vehicleID[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "requestID")
                requestID[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "msgCount")
                msgCount[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "basicVehicleRole")
                basicVehicleRole[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "inBoundLaneID")
                inBoundLaneID[i] = values[i][values[i].getMemberNames()[j]].asInt();
            
            if (values[i].getMemberNames()[j] == "inBoundApproachID")
                inBoundApproachID[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "ETA_Minute")
                expectedTimeOfArrival_Minute[i] = values[i][values[i].getMemberNames()[j]].asInt();

            if (values[i].getMemberNames()[j] == "ETA_Second")
                expectedTimeOfArrival_Second[i] = values[i][values[i].getMemberNames()[j]].asDouble();
            if (values[i].getMemberNames()[j] == "ETA_Duration")
                expectedTimeOfArrival_Duration[i] = values[i][values[i].getMemberNames()[j]].asDouble();

            if (values[i].getMemberNames()[j] == "priorityRequestStatus")
                priorityRequestStatus[i] = values[i][values[i].getMemberNames()[j]].asInt();
        }
    }
}

SignalStatus::~SignalStatus()
{
	delete vehicleID;
	delete requestID;
	delete msgCount;
	delete inBoundLaneID;
	delete inBoundApproachID;
	delete basicVehicleRole;
	delete expectedTimeOfArrival_Minute;
	delete expectedTimeOfArrival_Second;
	delete expectedTimeOfArrival_Duration;
	delete priorityRequestStatus;
}
