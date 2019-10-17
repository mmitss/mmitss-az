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
  1. This script is the header file for SignalRequest.cpp
*/

#pragma once
#include <string>
#include "ExpectedTimeOfArrival.h"
#include "IntersectionAccessPoint.h"
#include <Position3D.h>

class SignalRequest
{
private:
	int minuteOfYear{};
	int msOfMinute{};
	int msgCount{};
	int regionalID{};
	int intersectionID{};
	int requestID{};
	int priorityRequestType{};
	IntersectionAccessPoint inBoundLane;
	ETA expectedTimeOfArrival;
	int vehicleID{};
	int basicVehicleRole{};
	Position3D position;
	double heading_Degree{0.0};
	double speed_MeterPerSecond{0.0};
	int vehicleType{};

public:
	//Constructor
	SignalRequest();

	//Setters
	void setMinuteOfYear(int vehMinuteOfYear);
	void setMsOfMinute(int vehMsOfMinute);
	bool setMsgCount(int sequenceNumber);
	void setRegionalID(int vehRegionalID);
	void setIntersectionID(int vehIntersectionID);
	void setRequestID(int vehRequestID);
	void setPriorityRequestType(int vehPriorityRequestType);
	void setInBoundLaneIntersectionAccessPoint(int vehLaneID, int vehApproachID);
	void setETA(int vehExpectedTimeOfArrival_Minute, double vehExpectedTimeOfArrival_Second, double vehDuration);
	void setTemporaryVechileID(int temporaryVehicleID);
	void setBasicVehicleRole(int vehBasicVehicleRole);
	void setPosition(double vehLatitude_DecimalDegree, double vehLongitude_DecimalDegree, double vehElevation_Meter);
	bool setHeading_Degree(double vehHeading);
	bool setSpeed_MeterPerSecond(double vehSpeed);
	void setVehicleType(int vehType);

	//Getters
	int getMinuteOfYear();
	int getMsOfMinute();
	int getMsgCount();
	int getRegionalID();
	int getIntersectionID();
	int getRequestID();
	int getPriorityRequestType();
	IntersectionAccessPoint getInBoundLane();
	int getInBoundLaneID();
	int getInBoundApproachID();
	ETA getETA();
	int getETA_Minute();
	double getETA_Second();
	double getETA_Duration();
	int getTemporaryVehicleID();
	int getBasicVehicleRole();
	Position3D getPosition();
	double getLatitude_DecimalDegree();
	double getLongitude_DecimalDegree();
	double getElevation_Meter();
	double getHeading_Degree();
	double getSpeed_MeterPerSecond();
	int getVehicleType();

	//JSON
	std::string signalRequest2Json();
	void json2SignalRequest(std::string jsonString);

	//Destructor
	~SignalRequest();
};
