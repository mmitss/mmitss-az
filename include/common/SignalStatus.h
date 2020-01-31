/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  SignalStatus.h
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the header file for SignalStatus.cpp
*/

#pragma once

#include <string>
#include <vector>
#include "ExpectedTimeOfArrival.h"
#include "IntersectionAccessPoint.h"
#include "ActiveRequest.h"

class SignalStatus
{
private:
	int noOfRequest{};
	int minuteOfYear{};
	int msOfMinute{};
	int sequenceNumber{};
	int updateCount{};
	int regionalID{};
	int intersectionID{};
	std::vector<int>vehicleID{};
	std::vector<int>requestID{};
	std::vector<int>msgCount{};
	std::vector<int>inBoundLaneID{};
	std::vector<int>inBoundApproachID{};
	std::vector<int>basicVehicleRole{};
	std::vector<int>expectedTimeOfArrival_Minute{};
	std::vector<double>expectedTimeOfArrival_Second{};
	std::vector<double>expectedTimeOfArrival_Duration{};
	std::vector<int>priorityRequestStatus{};

public:
	//Constructor & Destructor
	SignalStatus();
	~SignalStatus();

	//Setters
	void setNoOfRequest(int artsize);
	void setMinuteOfYear(int vehMinuteOfYear);
	void setMsOfMinute(int vehMsOfMinute);
	void setSequenceNumber(int prsSequenceNumber);
	void setUpdateCount(int prsUpdateCount);
	void setRegionalID(int vehRegionalID);
	void setIntersectionID(int vehIntersectionID);
	void setTemporaryVechileID(std::vector<ActiveRequest> ActiveRequestTable);
	void setRequestID(std::vector<ActiveRequest> ActiveRequestTable);
	void setMsgCount(std::vector<ActiveRequest> ActiveRequestTable);
	void setBasicVehicleRole(std::vector<ActiveRequest> ActiveRequestTable);
	void setInBoundLaneIntersectionAccessPoint(std::vector<ActiveRequest> ActiveRequestTable);
	void setETA(std::vector<ActiveRequest> ActiveRequestTable);
	void setPriorityRequestStatus(std::vector<ActiveRequest> ActiveRequestTable);

	//Getters
	int getNoOfRequest();
	int getMinuteOfYear();
	int getMsOfMinute();
	int getPRSSequenceNumber();
	int getPRSUpdateCount();
	int getRegionalID();
	int getIntersectionID();
	std::vector<int> getTemporaryVehicleID();
	std::vector<int> getRequestID();
	std::vector<int> getMsgCount();
	std::vector<int> getBasicVehicleRole();
	std::vector<int> getInBoundLaneID();
	std::vector<int> getInBoundApproachID();
	std::vector<int> getETA_Minute();
	std::vector<double> getETA_Second();
	std::vector<double> getETA_Duration();
	std::vector<int> getPriorityRequestStatus();

	void reset(void);

	//JSON
	std::string signalStatus2Json(std::vector<ActiveRequest> ActiveRequestTable);
	void json2SignalStatus(std::string jsonString);



};