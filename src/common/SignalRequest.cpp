/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  SignalRequest.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script will get the values of all the attributes to create SRM JSON Object and convert the SRM JSON Object into Signal Request object
*/

#include <cstdint>
#include <iostream>
#include <fstream>
#include "SignalRequest.h"
#include "json/json.h"
#include "Timestamp.h"

const int SEQUENCE_NUMBER_MINLIMIT = 0;
const int SEQUENCE_NUMBER_MAXLIMIT = 127;

const double VEHICLE_SPEED_METERPERSECOND_MINLIMIT = 0.0;
const double VEHICLE_SPEED_METERPERSECOND_MAXLIMIT = 163.82;

const double VEHICLE_HEADING_MINLIMIT = 0.0;
const double VEHICLE_HEADING_MAXLIMIT = 359.9875;

//Constructor
SignalRequest::SignalRequest()
{
}

//Setters
void SignalRequest::setMinuteOfYear(int vehminuteOfYear)
{
    minuteOfYear = vehminuteOfYear;
}

void SignalRequest::setMsOfMinute(int vehMsOfMinute)
{
    msOfMinute = vehMsOfMinute;
}

bool SignalRequest::setMsgCount(int sequenceNumber)
{

    bool sequenceNumberStatus{true};

    if (sequenceNumber >= SEQUENCE_NUMBER_MINLIMIT && sequenceNumber <= SEQUENCE_NUMBER_MAXLIMIT)
        msgCount = sequenceNumber;
    else
        sequenceNumberStatus = false;

    return sequenceNumberStatus;
}

void SignalRequest::setRegionalID(int vehRegionalID)
{
    regionalID = vehRegionalID;
}

void SignalRequest::setIntersectionID(int vehIntersectionID)
{
    intersectionID = vehIntersectionID;
}

void SignalRequest::setRequestID(int vehRequestID)
{
    requestID = vehRequestID;
}

void SignalRequest::setPriorityRequestType(int vehPriorityRequestType)
{
    priorityRequestType = vehPriorityRequestType;
}

void SignalRequest::setInBoundLaneIntersectionAccessPoint(int vehLaneID, int vehApproachID)
{
    inBoundLane.setLaneID(vehLaneID);
    inBoundLane.setApproachID(vehApproachID);
}

void SignalRequest::setETA(int vehExpectedTimeOfArrival_Minute, int vehExpectedTimeOfArrival_Second, int vehDuration)
{
    expectedTimeOfArrival.setETA_Minute(vehExpectedTimeOfArrival_Minute);
    expectedTimeOfArrival.setETA_Second(vehExpectedTimeOfArrival_Second);
    expectedTimeOfArrival.setETA_Duration(vehDuration);
}

void SignalRequest::setTemporaryVechileID(int temporaryVehicleID)
{
    vehicleID = temporaryVehicleID;
}

void SignalRequest::setBasicVehicleRole(int vehBasicVehicleRole)
{
    basicVehicleRole = vehBasicVehicleRole;
}

void SignalRequest::setPosition(double vehLatitude_DecimalDegree, double vehLongitude_DecimalDegree, double vehElevation_Meter)
{

    position.setLatitude_decimalDegree(vehLatitude_DecimalDegree);
    position.setLongitude_decimalDegree(vehLongitude_DecimalDegree);
    position.setElevation_meter(vehElevation_Meter);
}

bool SignalRequest::setHeading_Degree(double vehHeading)
{
    bool vehicleHeadingStatus{true};

    if (vehHeading >= VEHICLE_HEADING_MINLIMIT && vehHeading <= VEHICLE_HEADING_MAXLIMIT)
        heading_Degree = vehHeading;
    else
        vehicleHeadingStatus = false;

    return vehicleHeadingStatus;
}

bool SignalRequest::setSpeed_MeterPerSecond(double vehSpeed_MeterPerSecond)
{
    bool vehicleSpeedStatus{true};

    if (vehSpeed_MeterPerSecond >= VEHICLE_SPEED_METERPERSECOND_MINLIMIT && vehSpeed_MeterPerSecond < VEHICLE_SPEED_METERPERSECOND_MAXLIMIT)
        speed_MeterPerSecond = vehSpeed_MeterPerSecond;
    else
        vehicleSpeedStatus = false;

    return vehicleSpeedStatus;
}

void SignalRequest::setVehicleType(int vehType)
{
    vehicleType = vehType;
}

//Getters
int SignalRequest::getMsgCount()
{
    return msgCount;
}

int SignalRequest::getMinuteOfYear()
{
    return minuteOfYear;
}

int SignalRequest::getMsOfMinute()
{
    return msOfMinute;
}

int SignalRequest::getRegionalID()
{
    return regionalID;
}

int SignalRequest::getIntersectionID()
{
    return intersectionID;
}

int SignalRequest::getRequestID()
{
    return requestID;
}
int SignalRequest::getPriorityRequestType()
{
    return priorityRequestType;
}

IntersectionAccessPoint SignalRequest::getInBoundLane()
{
    return inBoundLane;
}

int SignalRequest::getInBoundLaneID()
{
    return inBoundLane.getLaneID();
}

int SignalRequest::getInBoundApproachID()
{
    return inBoundLane.getApproachID();
}

ETA SignalRequest::getETA()
{
    return expectedTimeOfArrival;
}

int SignalRequest::getETA_Minute()
{
    return expectedTimeOfArrival.getETA_Minute();
}

int SignalRequest::getETA_Second()
{
    return expectedTimeOfArrival.getETA_Second();
}

int SignalRequest::getETA_Duration()
{
    return expectedTimeOfArrival.getETA_Duration();
}

int SignalRequest::getTemporaryVehicleID()
{
    return vehicleID;
}

int SignalRequest::getBasicVehicleRole()
{
    return basicVehicleRole;
}

Position3D SignalRequest::getPosition()
{
    return position;
}

double SignalRequest::getLatitude_DecimalDegree()
{
    return position.getLatitude_DecimalDegree();
}

double SignalRequest::getLongitude_DecimalDegree()
{
    return position.getLongitude_DecimalDegree();
}

double SignalRequest::getElevation_Meter()
{
    return position.getElevation_Meter();
}

double SignalRequest::getHeading_Degree()
{
    return heading_Degree;
}

double SignalRequest::getSpeed_MeterPerSecond()
{
    return speed_MeterPerSecond;
}

int SignalRequest::getVehicleType()
{
    return vehicleType;
}

//Following method is for creating json string from the mmitss signal request object
std::string SignalRequest::signalRequest2Json()
{
    Json::Value jsonObject;
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";
    std::string jsonString{};

    jsonObject["Timestamp_verbose"] = getVerboseTimestamp();
    jsonObject["Timestamp_posix"] = getPosixTimestamp();
    jsonObject["MsgType"] = "SRM";
    jsonObject["SignalRequest"]["msgCount"] = msgCount;
    jsonObject["SignalRequest"]["minuteOfYear"] = minuteOfYear;
    jsonObject["SignalRequest"]["msOfMinute"] = msOfMinute;
    jsonObject["SignalRequest"]["regionalID"] = regionalID;
    jsonObject["SignalRequest"]["intersectionID"] = intersectionID;
    jsonObject["SignalRequest"]["priorityRequestType"] = priorityRequestType;
    jsonObject["SignalRequest"]["inBoundLane"]["LaneID"] = inBoundLane.getLaneID();
    jsonObject["SignalRequest"]["inBoundLane"]["ApproachID"] = inBoundLane.getApproachID();
    jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Minute"] = expectedTimeOfArrival.getETA_Minute();
    jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Second"] = expectedTimeOfArrival.getETA_Second();
    jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Duration"] = expectedTimeOfArrival.getETA_Duration();
    jsonObject["SignalRequest"]["vehicleID"] = vehicleID;
    jsonObject["SignalRequest"]["basicVehicleRole"] = basicVehicleRole;
    jsonObject["SignalRequest"]["position"]["latitude_DecimalDegree"] = position.getLatitude_DecimalDegree();
    jsonObject["SignalRequest"]["position"]["longitude_DecimalDegree"] = position.getLongitude_DecimalDegree();
    jsonObject["SignalRequest"]["position"]["elevation_Meter"] = position.getElevation_Meter();
    jsonObject["SignalRequest"]["heading_Degree"] = heading_Degree;
    jsonObject["SignalRequest"]["speed_MeterPerSecond"] = speed_MeterPerSecond;
    // jsonObject["SignalRequest"]["vehicleType"] = vehicleType;

    jsonString = Json::writeString(builder, jsonObject);

    return jsonString;
}

//Following method is for converting json string into mmitss signal request object.
void SignalRequest::json2SignalRequest(std::string jsonString)
{
    Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	std::string errors{};

    reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;

    minuteOfYear = (jsonObject["SignalRequest"]["minuteOfYear"]).asInt();
    msOfMinute = (jsonObject["SignalRequest"]["msOfMinute"]).asInt();
    msgCount = (jsonObject["SignalRequest"]["msgCount"]).asInt();
    regionalID = (jsonObject["SignalRequest"]["regionalID"]).asInt();
    intersectionID = (jsonObject["SignalRequest"]["intersectionID"]).asInt();
    requestID = (jsonObject["SignalRequest"]["requestID"]).asInt();
    priorityRequestType = (jsonObject["SignalRequest"]["priorityRequestType"]).asInt();
    basicVehicleRole = (jsonObject["SignalRequest"]["basicVehicleRole"]).asInt();
    inBoundLane.setLaneID((jsonObject["SignalRequest"]["inBoundLane"]["LaneID"]).asInt());
    inBoundLane.setApproachID((jsonObject["SignalRequest"]["inBoundLane"]["ApproachID"]).asInt());
    expectedTimeOfArrival.setETA_Minute((jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Minute"]).asInt());
    expectedTimeOfArrival.setETA_Second((jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Second"]).asInt());
    expectedTimeOfArrival.setETA_Duration((jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Duration"]).asInt());
    vehicleID = (jsonObject["SignalRequest"]["vehicleID"]).asInt();
    position.setLatitude_decimalDegree((jsonObject["SignalRequest"]["position"]["latitude_DecimalDegree"]).asDouble());
    position.setLongitude_decimalDegree((jsonObject["SignalRequest"]["position"]["longitude_DecimalDegree"]).asDouble());
    position.setElevation_meter((jsonObject["SignalRequest"]["position"]["elevation_Meter"]).asDouble());
    heading_Degree = (jsonObject["SignalRequest"]["heading_Degree"]).asDouble();
    speed_MeterPerSecond = (jsonObject["SignalRequest"]["speed_MeterPerSecond"]).asDouble();
    // vehicleType = (jsonObject["SignalRequest"]["vehicleType"]).asInt();
}

//Destructor:
SignalRequest::~SignalRequest()
{
}
