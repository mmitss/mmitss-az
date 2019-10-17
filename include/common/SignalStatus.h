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
	int *vehicleID = new int[noOfRequest];
	int *requestID = new int[noOfRequest];
	int *msgCount = new int[noOfRequest];
	int *inBoundLaneID = new int[noOfRequest];
	int *inBoundApproachID = new int[noOfRequest];
	int *basicVehicleRole = new int[noOfRequest];
	int *expectedTimeOfArrival_Minute = new int[noOfRequest];
	double *expectedTimeOfArrival_Second = new double[noOfRequest];
	double *expectedTimeOfArrival_Duration = new double[noOfRequest];
	int *priorityRequestStatus = new int[noOfRequest];

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
	int * getTemporaryVehicleID();
	int * getRequestID();
	int * getMsgCount();
	int * getBasicVehicleRole();
	int * getInBoundLaneID();
	int * getInBoundApproachID();
	int * getETA_Minute();
	double * getETA_Second();
	double * getETA_Duration();
	int * getPriorityRequestStatus();


	//JSON
	std::string signalStatus2Json(std::vector<ActiveRequest> ActiveRequestTable);
	void json2SignalStatus(std::string jsonString);
	
};