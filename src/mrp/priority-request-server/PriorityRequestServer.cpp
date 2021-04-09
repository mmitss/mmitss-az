/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorityRequestServer.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This is the initial revision developed for receiving srm data from the vehicle (transceiver) and mainitaining the priority requests. 
  2. This application matches the intersection ID of the receive SRM to determine whether to accept the SRM or not
  3. The scripts either add, update or delete the request from the ART based on the vehicle type
  4. If the request comes from an emergency vehicle, the application will create split phase request and append it into the ART.
  5. This script use mapengine library to obtain the requested signal group.
  6. This application sends the requests list to the solver in a JSON formatted message.
  7. This application update the ETA of the available requests in the ART and create a JSON formatted SSM message.
  8. This application also delete the timed-out priority request from the ART.
*/
#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <algorithm>
#include "json/json.h"
#include "PriorityRequestServer.h"
#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"
#include "msgEnum.h"
#include "locAware.h"
#include "geoUtils.h"
#include "Timestamp.h"

using namespace MsgEnum;

PriorityRequestServer::PriorityRequestServer()
{
	string logging{};
	std::ofstream outputfile;
	Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
	delete reader;

	//Delete old map file
	intersectionName = jsonObject["IntersectionName"].asString();
	string deleteFileName = "/nojournal/bin/" + intersectionName + ".map.payload";
	remove(deleteFileName.c_str());

	//Write the map palyload in a file
	string mapPayload = (jsonObject["MapPayload"]).asString();

	const char *path = "/nojournal/bin";
	std::stringstream ss;
	ss << path;
	string s;
	ss >> s;

	outputfile.open(s + "/" + intersectionName + ".map.payload");

	outputfile << "payload"
			   << " "
			   << intersectionName << " " << mapPayload << endl;
	outputfile.close();

	//Set the intersection ID
	intersectionID = (jsonObject["IntersectionID"]).asInt();
	//set the regional ID
	regionalID = (jsonObject["RegionalID"]).asInt();
	// set the request timed out value for clearing the old request
	requestTimedOutValue = (jsonObject["SRMTimedOutTime"]).asDouble();
	// set the time interval for logging the system performance data
	timeInterval = (jsonObject["SystemPerformanceTimeInterval"]).asDouble();
	//Check the logging requirement
	logging = (jsonObject["Logging"]).asString();
	fileName = "/nojournal/bin/log/PRSLog-" + intersectionName + ".txt";
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	if (logging == "True")
	{
		loggingStatus = true;
		outputfile.open(fileName);
		outputfile << "PRS Logfile opened for " << intersectionName << " intersection at time : " << currentTime << endl;
		outputfile.close();
	}
	else
		loggingStatus = false;

	msgSentTime = static_cast<int>(currentTime);
}

/*
	- Method for identifying the message type
*/
int PriorityRequestServer::getMessageType(string jsonString)
{
	int messageType{};
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
	bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;
	if (parsingSuccessful)
	{
		if ((jsonObject["MsgType"]).asString() == "SRM")
			messageType = MsgEnum::DSRCmsgID_srm;

		else if ((jsonObject["MsgType"]).asString() == "CoordinationRequest")
			messageType = static_cast<int>(msgType::coordinationRequest);

		else
			cout << "[" << currentTime << "] Message type is unknown" << endl;
	}

	return messageType;
}

/*
	Get the Intersection ID from the configuration file.
*/
int PriorityRequestServer::getIntersectionID()
{
	return intersectionID;
}

/*
	Get the Regional ID from the configuration file.
*/
int PriorityRequestServer::getRegionalID()
{
	return regionalID;
}

/*
	If Intersection ID and Regional ID match then accept the srm
*/
bool PriorityRequestServer::acceptSignalRequest(SignalRequest signalRequest)
{
	bool matchIntersection{false};
	msgReceived++;

	if (intersectionID == signalRequest.getIntersectionID() && regionalID == signalRequest.getRegionalID())
		matchIntersection = true;

	else
	{
		matchIntersection = false;
		msgRejected++;
	}

	return matchIntersection;
}

/*
	If Active Request Table is empty or vehicle ID is not found in the Active Request Table and request type is priority request then add received srm in the Active Request table
*/
bool PriorityRequestServer::addToActiveRequestTable(SignalRequest signalRequest)
{
	bool addRequest{false};
	int vehid = signalRequest.getTemporaryVehicleID();

	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p) { return p.vehicleID == vehid; });

	if (ActiveRequestTable.size() >= Maximum_Number_Of_Priority_Request)
		addRequest = false;

	else if (ActiveRequestTable.empty() && (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest)))
		addRequest = true;

	else if (!ActiveRequestTable.empty() && (findVehicleIDOnTable == ActiveRequestTable.end()) &&
			 (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest)))
		addRequest = true;

	return addRequest;
}

/*
	- If Active Request Table is not empty or vehicle ID is already in the Active Request Table then-
		if msg count or vehicle Signal group or vehicle ETA changed then update Active request Table
*/
bool PriorityRequestServer::updateActiveRequestTable(SignalRequest signalRequest)
{
	bool updateValue{false};
	int vehid = signalRequest.getTemporaryVehicleID();
	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p) { return p.vehicleID == vehid; });

	if (ActiveRequestTable.empty())
		updateValue = false;

	else if (!ActiveRequestTable.empty() && findVehicleIDOnTable == ActiveRequestTable.end())
		updateValue = false;

	else if (!ActiveRequestTable.empty() && (findVehicleIDOnTable != ActiveRequestTable.end()) &&
			 (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::requestUpdate)))
		updateValue = true;

	return updateValue;
}

/*
	If Active Request Table is not empty or vehicle ID is already in the Active Request Table then-
		if priority request type is priorityCancellation then delete the srm from the Active Request Table
*/
bool PriorityRequestServer::deleteRequestfromActiveRequestTable(SignalRequest signalRequest)
{
	bool deleteRequest{false};
	int vehid = signalRequest.getTemporaryVehicleID();

	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

	if (ActiveRequestTable.empty())
		deleteRequest = false;

	else if (!ActiveRequestTable.empty() && findVehicleIDOnTable == ActiveRequestTable.end())
		deleteRequest = false;

	else if (!ActiveRequestTable.empty() && (findVehicleIDOnTable != ActiveRequestTable.end()) &&
			 (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityCancellation)))
		deleteRequest = true;

	return deleteRequest;
}

/*
	If there is no signal request is received from a vehicle for more than predifined time(10sec), PRS needs to delete that request.
*/
bool PriorityRequestServer::checkTimedOutRequestDeletingRequirement()
{
	bool deleteSignalRequest{false};
	int getSecondOfMinute = getMsOfMinute() / SECOND_FROM_MILISECOND;

	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if ((getSecondOfMinute > ActiveRequestTable[i].secondOfMinute) && (getSecondOfMinute - ActiveRequestTable[i].secondOfMinute) >= requestTimedOutValue)
			{
				deleteSignalRequest = true;
				setRequestTimedOutVehicleID(ActiveRequestTable[i].vehicleID);
				deleteTimedOutRequestfromActiveRequestTable();
				break;
			}
			else if ((getSecondOfMinute < ActiveRequestTable[i].secondOfMinute) && (getSecondOfMinute + SECONDS_IN_A_MINUTE - ActiveRequestTable[i].secondOfMinute) >= requestTimedOutValue)
			{
				deleteSignalRequest = true;
				setRequestTimedOutVehicleID(ActiveRequestTable[i].vehicleID);
				deleteTimedOutRequestfromActiveRequestTable();
				break;
			}
		}
	}

	return deleteSignalRequest;
}

/*
	-Setters for the timed out request. Set the vehicile ID which is not sending update request for a predefined amount of time
*/
void PriorityRequestServer::setRequestTimedOutVehicleID(int timedOutVehicleID)
{
	requestTimedOutVehicleID = timedOutVehicleID;
}

/*
	-If Active Request Table is empty, set the etaUpdateTime as current time while adding new priority requests
*/
void PriorityRequestServer::setETAUpdateTime()
{
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

	if(ActiveRequestTable.empty())
		etaUpdateTime = currentTime;
}

/*
	-Method for the obtaining the timed out vehicle ID
*/
int PriorityRequestServer::getRequestTimedOutVehicleID()
{
	return requestTimedOutVehicleID;
}

/*
	Method to check if EV is in the list to set priority request status
*/
bool PriorityRequestServer::findEVInList()
{
	bool emergencyVehicleStatusInList{false};

	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].basicVehicleRole == static_cast<int>(MsgEnum::basicRole::fire))
			{
				emergencyVehicleStatusInList = true;
				break;
			}
			else
				emergencyVehicleStatusInList = false;
		}
	}

	else
		emergencyVehicleStatusInList = false;

	return emergencyVehicleStatusInList;
}

/*
	Method to check if received SRM is from EV or not
*/
bool PriorityRequestServer::findEVInRequest(SignalRequest signalRequest)
{
	bool emergencyVehicleRequest{false};

	if (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::fire))
		emergencyVehicleRequest = true;

	return emergencyVehicleRequest;
}

/*
    - Obtain Split Phase information if EV is in List
*/
int PriorityRequestServer::getSplitPhase(int signalGroup)
{
	vector<int>::iterator it;
	int temporarySplitPhase{};

	switch (signalGroup)
	{
	case 1:
		temporarySplitPhase = 6;
		break;

	case 2:
		temporarySplitPhase = 5;
		break;

	case 3:
		temporarySplitPhase = 8;
		break;

	case 4:
		temporarySplitPhase = 7;
		break;

	case 5:
		temporarySplitPhase = 2;
		break;

	case 6:
		temporarySplitPhase = 1;
		break;

	case 7:
		temporarySplitPhase = 4;
		break;

	case 8:
		temporarySplitPhase = 3;
		break;

	default:
		break;
	}

	return temporarySplitPhase;
}

/*
	- Method to manage (create, update and delete) requests in the Active Request Table
	- If the request is received from EV, split phase request is added, or updated, or deleted in the ART.
	- If the the request type is update:
		- If the update request is received from transit or truck, find the position of the corresponding vehicle and update the information.
		- If the update request is received from EV, remove the vehicles priority requests both for the requested phase and the split phase request from the list.
			- The newly received update request in the list 
			- Then append the request regarding the split phase
	- ETA of the other (already added) priority requests in the ART will be updated.
*/
void PriorityRequestServer::manageSignalRequestTable(SignalRequest signalRequest)
{
	ActiveRequest activeRequest;
	activeRequest.reset();
	int vehid{};
	int temporarySignalGroup{};

	if (acceptSignalRequest(signalRequest))
	{
		if (addToActiveRequestTable(signalRequest))
		{
			setETAUpdateTime();
			setPRSUpdateCount();
			setVehicleType(signalRequest);
			temporarySignalGroup = getSignalGroup(signalRequest);
			activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
			activeRequest.requestID = signalRequest.getRequestID();
			activeRequest.msgCount = signalRequest.getMsgCount();
			activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
			activeRequest.vehicleType = vehicleType;
			activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
			activeRequest.minuteOfYear = getMinuteOfYear();
			activeRequest.secondOfMinute = getMsOfMinute() / SECOND_FROM_MILISECOND;
			activeRequest.signalGroup = temporarySignalGroup;
			activeRequest.vehicleETA = signalRequest.getETA_Minute() * SECONDS_IN_A_MINUTE + signalRequest.getETA_Second();
			activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
			activeRequest.vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
			activeRequest.vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
			activeRequest.vehicleElevation = signalRequest.getElevation_Meter();
			activeRequest.vehicleHeading = signalRequest.getHeading_Degree();
			activeRequest.vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
			ActiveRequestTable.push_back(activeRequest);

			//Add split phase request in the ART
			if (findEVInRequest(signalRequest))
			{
				activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
				activeRequest.requestID = signalRequest.getRequestID();
				activeRequest.msgCount = signalRequest.getMsgCount();
				activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
				activeRequest.vehicleType = vehicleType;
				activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
				activeRequest.minuteOfYear = getMinuteOfYear();
				activeRequest.secondOfMinute = getMsOfMinute() / SECOND_FROM_MILISECOND;
				activeRequest.signalGroup = getSplitPhase(temporarySignalGroup);
				activeRequest.vehicleETA = signalRequest.getETA_Minute() * SECONDS_IN_A_MINUTE + signalRequest.getETA_Second();
				activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
				activeRequest.vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
				activeRequest.vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
				activeRequest.vehicleElevation = signalRequest.getElevation_Meter();
				activeRequest.vehicleHeading = signalRequest.getHeading_Degree();
				activeRequest.vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
				ActiveRequestTable.push_back(activeRequest);
			}
			updateETAInActiveRequestTable();
		}

		else if (updateActiveRequestTable(signalRequest))
		{
			setPRSUpdateCount();
			vehid = signalRequest.getTemporaryVehicleID();
			//For EV prioriry requests
			if (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::fire))
			{
				for (int i = 0; i < 2; i++)
				{
					std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

					ActiveRequestTable.erase(findVehicleIDOnTable);
				}
				setVehicleType(signalRequest);
				temporarySignalGroup = getSignalGroup(signalRequest);

				activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
				activeRequest.requestID = signalRequest.getRequestID();
				activeRequest.msgCount = signalRequest.getMsgCount();
				activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
				activeRequest.vehicleType = vehicleType;
				activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
				activeRequest.minuteOfYear = getMinuteOfYear();
				activeRequest.secondOfMinute = getMsOfMinute() / SECOND_FROM_MILISECOND;
				activeRequest.signalGroup = temporarySignalGroup;
				activeRequest.vehicleETA = signalRequest.getETA_Minute() * SECONDS_IN_A_MINUTE + signalRequest.getETA_Second();
				activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
				activeRequest.vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
				activeRequest.vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
				activeRequest.vehicleElevation = signalRequest.getElevation_Meter();
				activeRequest.vehicleHeading = signalRequest.getHeading_Degree();
				activeRequest.vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
				ActiveRequestTable.push_back(activeRequest);
				if (findEVInRequest(signalRequest))
				{
					activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
					activeRequest.requestID = signalRequest.getRequestID();
					activeRequest.msgCount = signalRequest.getMsgCount();
					activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
					activeRequest.vehicleType = vehicleType;
					activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
					activeRequest.minuteOfYear = getMinuteOfYear();
					activeRequest.secondOfMinute = getMsOfMinute() / SECOND_FROM_MILISECOND;
					activeRequest.signalGroup = getSplitPhase(temporarySignalGroup);
					activeRequest.vehicleETA = signalRequest.getETA_Minute() * SECONDS_IN_A_MINUTE + signalRequest.getETA_Second();
					activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
					activeRequest.vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
					activeRequest.vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
					activeRequest.vehicleElevation = signalRequest.getElevation_Meter();
					activeRequest.vehicleHeading = signalRequest.getHeading_Degree();
					activeRequest.vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
					ActiveRequestTable.push_back(activeRequest);
				}
			}
			//For Transit and Truck priority requests
			else
			{
				std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																						 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

				findVehicleIDOnTable->vehicleID = signalRequest.getTemporaryVehicleID();
				findVehicleIDOnTable->requestID = signalRequest.getRequestID();
				findVehicleIDOnTable->msgCount = signalRequest.getMsgCount();
				findVehicleIDOnTable->basicVehicleRole = signalRequest.getBasicVehicleRole();
				findVehicleIDOnTable->vehicleLaneID = signalRequest.getInBoundLaneID();
				findVehicleIDOnTable->minuteOfYear = getMinuteOfYear();
				findVehicleIDOnTable->secondOfMinute = getMsOfMinute() / SECOND_FROM_MILISECOND;
				findVehicleIDOnTable->signalGroup = getSignalGroup(signalRequest);
				findVehicleIDOnTable->vehicleETA = signalRequest.getETA_Minute() * SECONDS_IN_A_MINUTE + signalRequest.getETA_Second();
				findVehicleIDOnTable->vehicleETADuration = signalRequest.getETA_Duration();
				findVehicleIDOnTable->vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
				findVehicleIDOnTable->vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
				findVehicleIDOnTable->vehicleElevation = signalRequest.getElevation_Meter();
				findVehicleIDOnTable->vehicleHeading = signalRequest.getHeading_Degree();
				findVehicleIDOnTable->vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
			}
			updateETAInActiveRequestTable();
		}
		else if (deleteRequestfromActiveRequestTable(signalRequest))
		{
			vehid = signalRequest.getTemporaryVehicleID();

			//If the delete request is for EV we need to delete both EV request (through and left turn phase)
			if (findEVInRequest(signalRequest))
			{
				for (int i = 0; i < 2; i++)
				{
					std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

					if (findVehicleIDOnTable != ActiveRequestTable.end())
						ActiveRequestTable.erase(findVehicleIDOnTable);
				}
			}

			else
			{
				std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																						 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });
				if (findVehicleIDOnTable != ActiveRequestTable.end())
					ActiveRequestTable.erase(findVehicleIDOnTable);
			}
			updateETAInActiveRequestTable();
		}

		setPriorityRequestStatus();
		setSrmMessageStatus(signalRequest);
	}
}
/*
	Method to delete vehicle info from Active Request Table if Infrustracture doesn't receive and SRM for predefined time
*/
void PriorityRequestServer::deleteTimedOutRequestfromActiveRequestTable()
{
	int vehicleID{};
	int associatedVehicleID{};
	vehicleID = getRequestTimedOutVehicleID();
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p) { return p.vehicleID == vehicleID; });

	//For EV we have to delete two request
	if ((findVehicleIDOnTable->vehicleType == static_cast<int>(MsgEnum::vehicleType::special)) &&
		(findVehicleIDOnTable != ActiveRequestTable.end()))
	{
		ActiveRequestTable.erase(findVehicleIDOnTable);

		//Deleting the request related to split phase.
		vector<ActiveRequest>::iterator findSplitPhaseEV = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p) { return p.vehicleID == vehicleID; });
		ActiveRequestTable.erase(findSplitPhaseEV);
	}
	//For Coordination Request
	else if ((findVehicleIDOnTable->vehicleType == coordinationVehicleType) && (findVehicleIDOnTable != ActiveRequestTable.end()))
	{
		ActiveRequestTable.erase(findVehicleIDOnTable);

		if (vehicleID == 1)
			associatedVehicleID = 2;

		else if (vehicleID == 2)
			associatedVehicleID = 1;

		else if (vehicleID == 3)
			associatedVehicleID = 4;

		else if (vehicleID == 4)
			associatedVehicleID = 3;

		//Deleting the associated coordination request on the same cycle.
		vector<ActiveRequest>::iterator findAssociatedCoordinationRequest = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																						 [&](ActiveRequest const &p) { return p.vehicleID == associatedVehicleID; });
		ActiveRequestTable.erase(findAssociatedCoordinationRequest);
	}

	//For Transit and truck PriorityRequest
	else if (findVehicleIDOnTable != ActiveRequestTable.end())
		ActiveRequestTable.erase(findVehicleIDOnTable);
	cout << "[" << currentTime << "] Deleted Timed-Out Request" << endl;
}

/*
	Method for creating Signal Status Message from the Active Request Table.
*/
string PriorityRequestServer::createSSMJsonString(SignalStatus signalStatus)
{
	string ssmJsonString{};
	signalStatus.reset();
	signalStatus.setMinuteOfYear(getMinuteOfYear());
	signalStatus.setMsOfMinute(getMsOfMinute());
	signalStatus.setSequenceNumber(getPRSSequenceNumber());
	signalStatus.setUpdateCount(getPRSUpdateCount());
	signalStatus.setRegionalID(regionalID);
	signalStatus.setIntersectionID(intersectionID);
	ssmJsonString = signalStatus.signalStatus2Json(ActiveRequestTable);
	loggingData(ssmJsonString, "sent");

	return ssmJsonString;
}

/*
	Method for creating json string from the Active Request Table for the Priority Solver.
*/
string PriorityRequestServer::createJsonStringForPrioritySolver()
{
	string solverJsonString{};
	int noOfRequest{};
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	Json::Value jsonObject;
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "";

	noOfRequest = static_cast<int>(ActiveRequestTable.size());
	if (noOfRequest > 0)
	{
		jsonObject["MsgType"] = "PriorityRequest";
		jsonObject["PriorityRequestList"]["noOfRequest"] = noOfRequest;
		jsonObject["PriorityRequestList"]["minuteOfYear"] = getMinuteOfYear();
		jsonObject["PriorityRequestList"]["msOfMinute"] = getMsOfMinute();
		jsonObject["PriorityRequestList"]["regionalID"] = regionalID;
		jsonObject["PriorityRequestList"]["intersectionID"] = intersectionID;
		for (int i = 0; i < noOfRequest; i++)
		{
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleID"] = ActiveRequestTable[i].vehicleID;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleType"] = ActiveRequestTable[i].vehicleType;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["basicVehicleRole"] = ActiveRequestTable[i].basicVehicleRole;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["inBoundLaneID"] = ActiveRequestTable[i].vehicleLaneID;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"] = ActiveRequestTable[i].signalGroup;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["priorityRequestStatus"] = ActiveRequestTable[i].prsStatus;

			if (ActiveRequestTable[i].vehicleETA <= 0)
				jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] = 1.0;
			else
				jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] = ActiveRequestTable[i].vehicleETA;

			jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"] = ActiveRequestTable[i].vehicleETADuration;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["latitude_DecimalDegree"] = ActiveRequestTable[i].vehicleLatitude;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["longitude_DecimalDegree"] = ActiveRequestTable[i].vehicleLongitude;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["elevation_Meter"] = ActiveRequestTable[i].vehicleElevation;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["heading_Degree"] = ActiveRequestTable[i].vehicleHeading;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["speed_MeterPerSecond"] = ActiveRequestTable[i].vehicleSpeed;
		}
		sentClearRequest = false;
	}

	else
	{
		jsonObject["MsgType"] = "ClearRequest";
		cout << "[" << currentTime << "] Sent Clear Request to Solver " << endl;
		sentClearRequest = true;
	}

	solverJsonString = Json::writeString(builder, jsonObject);
	loggingData(solverJsonString, "sent");

	return solverJsonString;
}

/*
	- Checking whether PRS need to update the ETA in the ART.
		- If there is no request in the list no need to update the ETA
		- If vehicle ETA is zero no need to update.
*/
bool PriorityRequestServer::updateETA()
{
	bool etaUpdateRequirement{false};
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

	if (!ActiveRequestTable.empty() && (currentTime - etaUpdateTime >= TIME_GAP_BETWEEN_ETA_Update))
	{
		etaUpdateRequirement = true;
		updateETAInActiveRequestTable();
	}

	return etaUpdateRequirement;
}

/*
	- Method for checking whether clear request has to be sent to solver
*/
bool PriorityRequestServer::sendClearRequest()
{
	bool clearRequestStatus{false};

	if ((ActiveRequestTable.size() == 0) && (sentClearRequest == false))
		clearRequestStatus = true;

	return clearRequestStatus;
}

/*
	- Method to update ETA in Active Request Table if vehile ETA is not zero.
*/
void PriorityRequestServer::updateETAInActiveRequestTable()
{
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	double timeDifference = currentTime - etaUpdateTime;

	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			ActiveRequestTable[i].vehicleETA = 	ActiveRequestTable[i].vehicleETA - timeDifference;
			
			if (ActiveRequestTable[i].vehicleETA <= 0)
				ActiveRequestTable[i].vehicleETA = 0.0;
		}

		etaUpdateTime = currentTime;
	}
}

/*
	Method to print Active Request Table
*/
void PriorityRequestServer::printActiveRequestTable()
{
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	if (!ActiveRequestTable.empty())
	{
		cout << "[" << currentTime << "] Active Request Table is following: " << endl;
		cout << "VehicleID"
			 << " "
			 << "VehicleType"
			 << " "
			 << "ETA"
			 << " "
			 << "ETADuration"
			 << " "
			 << "SignalGroup\t" << endl;
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
			cout << "   " << ActiveRequestTable[i].vehicleID << "       " << ActiveRequestTable[i].vehicleType << "        " << ActiveRequestTable[i].vehicleETA << "       " << ActiveRequestTable[i].vehicleETADuration << "         " << ActiveRequestTable[i].signalGroup << endl;
	}

	else
		cout << "[" << currentTime << "] Active Request Table is empty" << endl;
}

/*
	- Method for defining priority request status.
		- If there is EV in the priority request list for EV priority status will be granted and for rest of the vehicle priority status will be rejected.
		- If there is no EV in the priority request list for Transit and Truck priority status will be granted.
*/
void PriorityRequestServer::setPriorityRequestStatus() //work on this wih traffic controller or priority solver or whom. check page 176 of j2735 pdf
{
	emergencyVehicleStatus = findEVInList();

	if (emergencyVehicleStatus == true)
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::fire)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::granted);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::rejected);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::rejected);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::roadsideSource)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::rejected);

			else
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::unavailable);
		}
	}

	else if (emergencyVehicleStatus == false)
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::granted);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::granted);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::roadsideSource)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::granted);

			else
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::unavailable); // In standard it is unknown
		}
	}
}

/*
	- Method for storing the SRM Message status for system performance data log
*/
void PriorityRequestServer::setSrmMessageStatus(SignalRequest signalRequest)
{
	if ((emergencyVehicleStatus == true) && (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::fire)))
		msgServed++;

	else if ((emergencyVehicleStatus == true) && (signalRequest.getBasicVehicleRole() != static_cast<int>(MsgEnum::basicRole::fire)))
		msgRejected++;

	else if ((emergencyVehicleStatus == false) && (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::transit)))
		msgServed++;

	else if ((emergencyVehicleStatus == false) && (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::truck)))
		msgServed++;
}

/*
	Method for updating the updateCount from the infrastructure side.
*/
void PriorityRequestServer::setPRSUpdateCount()
{
	if (updateCount < SEQUENCE_NUMBER_MAXLIMIT)
		updateCount++;

	else
		updateCount = SEQUENCE_NUMBER_MINLIMIT;
}

/*
	- Method for setting vehicle type based on the basic vehicle role
*/
void PriorityRequestServer::setVehicleType(SignalRequest signalRequest)
{
	if (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::fire))
		vehicleType = static_cast<int>(MsgEnum::vehicleType::special);

	else if (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::transit))
		vehicleType = static_cast<int>(MsgEnum::vehicleType::bus);

	else if (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::truck))
		vehicleType = static_cast<int>(MsgEnum::vehicleType::axleCnt4);

	else
		vehicleType = static_cast<int>(MsgEnum::vehicleType::unavailable);
}

/*
	Method for obtaining minute of a year based on current time.
*/
int PriorityRequestServer::getMinuteOfYear()
{
	time_t t = time(NULL);
	tm *timePtr = gmtime(&t);

	int dayOfYear = timePtr->tm_yday;
	int currentHour = timePtr->tm_hour;
	int currentMinute = timePtr->tm_min;

	minuteOfYear = (dayOfYear - 1) * HOURS_IN_A_DAY * MINUTES_IN_A_HOUR + currentHour * MINUTES_IN_A_HOUR + currentMinute;

	return minuteOfYear;
}

/*
	Method for obtaining millisecond of a minute based on current time.
*/
int PriorityRequestServer::getMsOfMinute()
{
	time_t t = time(NULL);
	tm *timePtr = gmtime(&t);

	int currentSecond = timePtr->tm_sec;
	msOfMinute = currentSecond * SECOND_FROM_MILISECOND;

	return msOfMinute;
}

/*
	Method for updating the messageCount from the infrastructure side.
*/
int PriorityRequestServer::getPRSSequenceNumber()
{
	if (sequenceNumber < SEQUENCE_NUMBER_MAXLIMIT)
		sequenceNumber++;

	else
		sequenceNumber = SEQUENCE_NUMBER_MINLIMIT;

	return sequenceNumber;
}

/*
	Method for obtaining the updateCount.
*/
int PriorityRequestServer::getPRSUpdateCount()
{
	return updateCount;
}

/*
	Method for obtaining signal group based on vehicle laneID and approachID using MapEngine Library.
*/
int PriorityRequestServer::getSignalGroup(SignalRequest signalRequest)
{
	int phaseNo{};
	int approachID{};
	string fmap{};
	bool singleFrame = false;

	fmap = "/nojournal/bin/" + intersectionName + ".map.payload";

	LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
	approachID = plocAwareLib->getApproachIdByLaneId(static_cast<uint16_t>(regionalID), static_cast<uint16_t>(intersectionID), static_cast<uint8_t>(signalRequest.getInBoundLaneID()));
	phaseNo = unsigned(plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalID), static_cast<uint16_t>(intersectionID), static_cast<uint8_t>(approachID),
														  static_cast<uint8_t>(signalRequest.getInBoundLaneID())));

	delete plocAwareLib;
	return phaseNo;
}

/*
	Method for logging sending or receied JSON sting.
*/
void PriorityRequestServer::loggingData(string jsonString, string communicationType)
{
	std::ofstream outputfile;
	std::ifstream infile;

	if (loggingStatus == true)
	{
		outputfile.open(fileName, std::ios_base::app);
		auto currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		outputfile << "\nThe following message is " << communicationType << " at time : " << currentTime << endl;
		outputfile << jsonString << endl;
		outputfile.close();
	}
}

/*
	Method to check for sending system peformance data to Data-Collector.
*/
bool PriorityRequestServer::sendSystemPerformanceDataLog()
{
	bool sendData{false};
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

	if (currentTime - msgSentTime >= timeInterval)
		sendData = true;

	return sendData;
}

/*
	Method to create a JSON string to send system peformance data to Data-Collector.
*/
string PriorityRequestServer::createJsonStringForSystemPerformanceDataLog()
{
	string systemPerformanceDataLogJsonString{};
	double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	Json::Value jsonObject;
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "";

	jsonObject["MsgType"] = "MsgCount";
	jsonObject["MsgInformation"]["MsgSource"] = intersectionName;
	jsonObject["MsgInformation"]["MsgCountType"] = "SRM";
	jsonObject["MsgInformation"]["MsgCount"] = msgReceived;
	jsonObject["MsgInformation"]["MsgServed"] = msgServed;
	jsonObject["MsgInformation"]["MsgRejected"] = msgRejected;
	jsonObject["MsgInformation"]["TimeInterval"] = timeInterval;
	jsonObject["MsgInformation"]["Timestamp_posix"] = getPosixTimestamp();
	jsonObject["MsgInformation"]["Timestamp_verbose"] = getVerboseTimestamp();

	systemPerformanceDataLogJsonString = Json::writeString(builder, jsonObject);

	msgSentTime = static_cast<int>(currentTime);
	msgReceived = 0;
	msgServed = 0;
	msgRejected = 0;
	cout << "[" << currentTime << "] System Performance Data Log will sent to data collector" << endl;

	return systemPerformanceDataLogJsonString;
}

/*
	- Following method is responsible for managing the Coordination request 
	- The method will check whether a particular coordination request is already available in the list or not (based on the vehicle ID)
	- If the request is already in the the list, the method will delete the old request.
	- The request will be added in the list afterwards.
	- The method will call setPriorityRequestStatus() function, to set the priority request status
*/
void PriorityRequestServer::manageCoordinationRequest(string jsonString)
{
	Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
	ActiveRequest activeRequest;
	activeRequest.reset();
	setETAUpdateTime();

	reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;
	int noOfCoordinationRequest = jsonObject["noOfCoordinationRequest"].asInt();
	
	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	{
		if (ActiveRequestTable[i].vehicleType == coordinationVehicleType)
		{
			vector<ActiveRequest>::iterator findCoordinationVehicleTypeOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							  [&](ActiveRequest const &p) { return p.vehicleType == coordinationVehicleType; });

			ActiveRequestTable.erase(findCoordinationVehicleTypeOnTable);
			i--;
		}
	}

	for (int i = 0; i < noOfCoordinationRequest; i++)
	{
		activeRequest.minuteOfYear = getMinuteOfYear();
		activeRequest.secondOfMinute = getMsOfMinute() / SECOND_FROM_MILISECOND;
		activeRequest.basicVehicleRole = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["basicVehicleRole"].asInt();
		activeRequest.signalGroup = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["requestedPhase"].asInt();
		activeRequest.vehicleID = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["vehicleID"].asInt();
		activeRequest.vehicleType = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["vehicleType"].asInt();
		activeRequest.vehicleETA = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["ETA"].asDouble();
		activeRequest.vehicleETADuration = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["CoordinationSplit"].asDouble();
		activeRequest.vehicleLaneID = coordinationLaneID;
		ActiveRequestTable.push_back(activeRequest);
	}

	setPriorityRequestStatus();
	updateETAInActiveRequestTable();
}

PriorityRequestServer::~PriorityRequestServer()
{
}