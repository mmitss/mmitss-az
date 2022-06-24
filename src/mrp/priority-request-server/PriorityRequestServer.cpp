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
  3. The scripts either add, update or delete the request from the ART based on the vehicle role
  4. If the request comes from an emergency vehicle, the application will create split phase request and append it into the ART.
  5. This script use mapengine library to obtain the requested signal group.
  6. This application sends the requests list to the solver in a JSON formatted message.
  7. This application update the ETA of the available requests in the ART and create a JSON formatted SSM message.
  8. This application also delete the timed-out priority request from the ART.
*/

#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <algorithm>
#include "PriorityRequestServer.h"
#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"
#include "geoUtils.h"
#include <time.h>

using namespace MsgEnum;

PriorityRequestServer::PriorityRequestServer()
{
	bool singleFrame{false};

	readconfigFile();

	LocAware *tempPlocAwareLib = new LocAware(mapPayloadFileName, singleFrame);
	plocAwareLib = tempPlocAwareLib;
}

/*
	- Method for identifying the message type
*/
int PriorityRequestServer::getMessageType(string jsonString)
{
	int messageType{};
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
		{
			displayConsoleData("Message type is unknown");
			loggingData("Message type is unknown");
		}
	}

	return messageType;
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
		displayConsoleData("Discard the SRM since intersectionId doesn't match");
		loggingData("Discard the SRM since intersectionId doesn't match");
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
	int vehicleID = signalRequest.getTemporaryVehicleID();

	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p)
																		{ return p.vehicleID == vehicleID; });

	if (ActiveRequestTable.size() >= Maximum_Number_Of_Priority_Request)
		addRequest = false;

	else if (ActiveRequestTable.empty() && (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest)))
		addRequest = true;

	else if (!ActiveRequestTable.empty() && (findVehicleIDOnTable == ActiveRequestTable.end()) &&
			 (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest)))
		addRequest = true;

	// else if (!ActiveRequestTable.empty() && (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest)))
	// 	addRequest = true;

	return addRequest;
}

/*
	- If Active Request Table is not empty or vehicle ID is already in the Active Request Table then-
		if msg count or vehicle Signal group or vehicle ETA changed then update Active request Table
*/
bool PriorityRequestServer::updateActiveRequestTable(SignalRequest signalRequest)
{
	bool updateRequest{false};
	int vehicleID = signalRequest.getTemporaryVehicleID();
	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p)
																		{ return p.vehicleID == vehicleID; });

	if (ActiveRequestTable.empty())
		updateRequest = false;

	else if (!ActiveRequestTable.empty() && findVehicleIDOnTable == ActiveRequestTable.end())
		updateRequest = false;

	else if (!ActiveRequestTable.empty() && (findVehicleIDOnTable != ActiveRequestTable.end()) &&
			 (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::requestUpdate)))
		updateRequest = true;

	// Following Logic is for the SRM received from DanLaw unit since they doesn't send update request.
	else if (!ActiveRequestTable.empty() && (findVehicleIDOnTable != ActiveRequestTable.end()) &&
			 (signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest)) &&
			 (getPosixTimestamp() - findVehicleIDOnTable->msgReceivedTime) >= ART_UPDATE_FREQUENCY)
	{
		if (findVehicleIDOnTable->signalGroup != vehicleRequestedSignalGroup)
			updateRequest = true;

		else if (fabs(findVehicleIDOnTable->vehicleSpeed - signalRequest.getSpeed_MeterPerSecond()) >= ALLOWED_SPEED_DEVIATION)
			updateRequest = true;

		else if (fabs(findVehicleIDOnTable->vehicleETA - vehicleETA) >= ALLOWED_ETA_DIFFERENCE)
			updateRequest = true;

		else if (getPosixTimestamp() - findVehicleIDOnTable->artForwardTime >= SRM_TIME_GAP_VALUE)
			updateRequest = true;
	}

	return updateRequest;
}

/*
	If Active Request Table is not empty or vehicle ID is already in the Active Request Table then-
		if priority request type is priorityCancellation then delete the srm from the Active Request Table
*/
bool PriorityRequestServer::deleteRequestfromActiveRequestTable(SignalRequest signalRequest)
{
	bool deleteRequest{false};
	int vehicleID = signalRequest.getTemporaryVehicleID();

	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p)
																			 { return p.vehicleID == vehicleID; });

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
	double currentTime = getPosixTimestamp();

	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if ((currentTime - ActiveRequestTable[i].msgReceivedTime) >= requestTimedOutValue)
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
	double currentTime = getPosixTimestamp();

	if (ActiveRequestTable.empty())
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

void PriorityRequestServer::calculateETA(int ETA_Minute, int ETA_Second)
{
	vehicleETA = ((ETA_Minute - currentMinuteOfYear) * SECOND_MINTUTE_CONVERSION) + ((ETA_Second - currentMsOfMinute) / SECOND_MILISECOND_CONVERSION);

	if (vehicleETA < Minimum_ETA)
		vehicleETA = Minimum_ETA;
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
	int vehicleID{};
	double currentTime = getPosixTimestamp();
	ActiveRequest activeRequest;
	activeRequest.reset();

	displayConsoleData("Received Priority Request from MsgDecoder");
	loggingData("Received Priority Request from MsgDecoder");

	if (acceptSignalRequest(signalRequest))
	{
		setMinuteOfYear();
		setMsOfMinute();
		calculateETA(signalRequest.getETA_Minute(), signalRequest.getETA_Second());
		setRequestedSignalGroup(signalRequest);

		if (addToActiveRequestTable(signalRequest))
		{
			setETAUpdateTime();
			setPRSUpdateCount();
			setVehicleType(signalRequest);

			activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
			activeRequest.requestID = signalRequest.getRequestID();
			activeRequest.msgCount = signalRequest.getMsgCount();
			activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
			activeRequest.vehicleType = vehicleType;
			activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
			activeRequest.minuteOfYear = currentMsOfMinute;
			activeRequest.secondOfMinute = static_cast<int>(currentMsOfMinute / SECOND_MILISECOND_CONVERSION);
			activeRequest.signalGroup = vehicleRequestedSignalGroup;
			activeRequest.vehicleETAMinute = signalRequest.getETA_Minute();
			activeRequest.vehicleETASecond = signalRequest.getETA_Second();
			activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
			activeRequest.vehicleETA = vehicleETA;
			activeRequest.vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
			activeRequest.vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
			activeRequest.vehicleElevation = signalRequest.getElevation_Meter();
			activeRequest.vehicleHeading = signalRequest.getHeading_Degree();
			activeRequest.vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
			activeRequest.msgReceivedTime = currentTime;
			activeRequest.etaUpdateTime = currentTime;
			ActiveRequestTable.push_back(activeRequest);

			// Add split phase request in the ART
			if (findEVInRequest(signalRequest))
			{
				activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
				activeRequest.requestID = signalRequest.getRequestID();
				activeRequest.msgCount = signalRequest.getMsgCount();
				activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
				activeRequest.vehicleType = vehicleType;
				activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
				activeRequest.minuteOfYear = currentMinuteOfYear;
				activeRequest.secondOfMinute = static_cast<int>(currentMsOfMinute / SECOND_MILISECOND_CONVERSION);
				activeRequest.signalGroup = getSplitPhase(vehicleRequestedSignalGroup);
				activeRequest.vehicleETAMinute = signalRequest.getETA_Minute();
				activeRequest.vehicleETASecond = signalRequest.getETA_Second();
				activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
				activeRequest.vehicleETA = vehicleETA;
				activeRequest.vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
				activeRequest.vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
				activeRequest.vehicleElevation = signalRequest.getElevation_Meter();
				activeRequest.vehicleHeading = signalRequest.getHeading_Degree();
				activeRequest.vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
				activeRequest.msgReceivedTime = currentTime;
				activeRequest.etaUpdateTime = currentTime;
				ActiveRequestTable.push_back(activeRequest);
			}
			sendPriorityRequestList = true;
		}

		else if (updateActiveRequestTable(signalRequest))
		{
			setPRSUpdateCount();
			vehicleID = signalRequest.getTemporaryVehicleID();
			// For EV prioriry requests
			if (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::fire))
			{
				for (int i = 0; i < 2; i++)
				{
					std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							 [&](ActiveRequest const &p)
																							 { return p.vehicleID == vehicleID; });

					ActiveRequestTable.erase(findVehicleIDOnTable);
				}
				setVehicleType(signalRequest);

				activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
				activeRequest.requestID = signalRequest.getRequestID();
				activeRequest.msgCount = signalRequest.getMsgCount();
				activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
				activeRequest.vehicleType = vehicleType;
				activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
				activeRequest.minuteOfYear = currentMinuteOfYear;
				activeRequest.secondOfMinute = static_cast<int>(currentMsOfMinute / SECOND_MILISECOND_CONVERSION);
				activeRequest.signalGroup = vehicleRequestedSignalGroup;
				activeRequest.vehicleETAMinute = signalRequest.getETA_Minute();
				activeRequest.vehicleETASecond = signalRequest.getETA_Second();
				activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
				activeRequest.vehicleETA = vehicleETA;
				activeRequest.vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
				activeRequest.vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
				activeRequest.vehicleElevation = signalRequest.getElevation_Meter();
				activeRequest.vehicleHeading = signalRequest.getHeading_Degree();
				activeRequest.vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
				activeRequest.msgReceivedTime = currentTime;
				activeRequest.etaUpdateTime = currentTime;
				ActiveRequestTable.push_back(activeRequest);

				if (findEVInRequest(signalRequest))
				{
					activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
					activeRequest.requestID = signalRequest.getRequestID();
					activeRequest.msgCount = signalRequest.getMsgCount();
					activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
					activeRequest.vehicleType = vehicleType;
					activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
					activeRequest.minuteOfYear = currentMinuteOfYear;
					activeRequest.secondOfMinute = static_cast<int>(currentMsOfMinute / SECOND_MILISECOND_CONVERSION);
					activeRequest.signalGroup = getSplitPhase(vehicleRequestedSignalGroup);
					activeRequest.vehicleETAMinute = signalRequest.getETA_Minute();
					activeRequest.vehicleETASecond = signalRequest.getETA_Second();
					activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
					activeRequest.vehicleETA = vehicleETA;
					activeRequest.vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
					activeRequest.vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
					activeRequest.vehicleElevation = signalRequest.getElevation_Meter();
					activeRequest.vehicleHeading = signalRequest.getHeading_Degree();
					activeRequest.vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
					activeRequest.msgReceivedTime = currentTime;
					activeRequest.etaUpdateTime = currentTime;
					ActiveRequestTable.push_back(activeRequest);
				}
			}
			// For Transit and Truck priority requests
			else
			{
				std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																						 [&](ActiveRequest const &p)
																						 { return p.vehicleID == vehicleID; });

				findVehicleIDOnTable->vehicleID = signalRequest.getTemporaryVehicleID();
				findVehicleIDOnTable->requestID = signalRequest.getRequestID();
				findVehicleIDOnTable->msgCount = signalRequest.getMsgCount();
				findVehicleIDOnTable->basicVehicleRole = signalRequest.getBasicVehicleRole();
				findVehicleIDOnTable->vehicleLaneID = signalRequest.getInBoundLaneID();
				findVehicleIDOnTable->minuteOfYear = currentMinuteOfYear;
				findVehicleIDOnTable->secondOfMinute = static_cast<int>(currentMsOfMinute / SECOND_MILISECOND_CONVERSION);
				findVehicleIDOnTable->signalGroup = vehicleRequestedSignalGroup;
				findVehicleIDOnTable->vehicleETAMinute = signalRequest.getETA_Minute();
				findVehicleIDOnTable->vehicleETASecond = signalRequest.getETA_Second();
				findVehicleIDOnTable->vehicleETADuration = signalRequest.getETA_Duration();
				findVehicleIDOnTable->vehicleETA = vehicleETA;
				findVehicleIDOnTable->vehicleLatitude = signalRequest.getLatitude_DecimalDegree();
				findVehicleIDOnTable->vehicleLongitude = signalRequest.getLongitude_DecimalDegree();
				findVehicleIDOnTable->vehicleElevation = signalRequest.getElevation_Meter();
				findVehicleIDOnTable->vehicleHeading = signalRequest.getHeading_Degree();
				findVehicleIDOnTable->vehicleSpeed = signalRequest.getSpeed_MeterPerSecond();
				findVehicleIDOnTable->msgReceivedTime = currentTime;
				findVehicleIDOnTable->etaUpdateTime = currentTime;
			}
			sendPriorityRequestList = true;
		}

		else if (deleteRequestfromActiveRequestTable(signalRequest))
		{
			vehicleID = signalRequest.getTemporaryVehicleID();

			// If the delete request is for EV we need to delete both EV request (through and left turn phase)
			if (findEVInRequest(signalRequest))
			{
				for (int i = 0; i < 2; i++)
				{
					std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							 [&](ActiveRequest const &p)
																							 { return p.vehicleID == vehicleID; });

					if (findVehicleIDOnTable != ActiveRequestTable.end())
						ActiveRequestTable.erase(findVehicleIDOnTable);
				}

				if (!ActiveRequestTable.empty())
				{
					std::vector<ActiveRequest>::iterator findVehicleRoleOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							   [&](ActiveRequest const &p)
																							   { return p.basicVehicleRole == static_cast<int>(MsgEnum::basicRole::fire); });
					if (findVehicleRoleOnTable == ActiveRequestTable.end())
						sentClearRequestForEV = true; // It will allow to cancel the omit command.
				}
			}

			else
			{
				std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																						 [&](ActiveRequest const &p)
																						 { return p.vehicleID == vehicleID; });
				if (findVehicleIDOnTable != ActiveRequestTable.end())
					ActiveRequestTable.erase(findVehicleIDOnTable);
			}
			sendPriorityRequestList = true;
		}

		updateETAInActiveRequestTable();
		setPriorityRequestStatus();
		setSrmMessageStatus(signalRequest);
		sendSSM = true;
	}

	else
	{
		sendSSM = false;
		sendPriorityRequestList = false;
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

	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p)
																		{ return p.vehicleID == vehicleID; });

	// For EV we have to delete two request
	if ((findVehicleIDOnTable->basicVehicleRole == static_cast<int>(MsgEnum::basicRole::fire)) &&
		(findVehicleIDOnTable != ActiveRequestTable.end()))
	{
		ActiveRequestTable.erase(findVehicleIDOnTable);

		// Deleting the request related to split phase.
		vector<ActiveRequest>::iterator findSplitPhaseEV = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p)
																		{ return p.vehicleID == vehicleID; });
		ActiveRequestTable.erase(findSplitPhaseEV);

		if (!ActiveRequestTable.empty())
		{
			std::vector<ActiveRequest>::iterator findVehicleRoleOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																					   [&](ActiveRequest const &p)
																					   { return p.basicVehicleRole == static_cast<int>(MsgEnum::basicRole::fire); });
			if (findVehicleRoleOnTable == ActiveRequestTable.end())
				sentClearRequestForEV = true;
		}
	}
	// For Coordination Request
	else if ((findVehicleIDOnTable->basicVehicleRole == static_cast<int>(MsgEnum::basicRole::roadsideSource)) && (findVehicleIDOnTable != ActiveRequestTable.end()))
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

		// Deleting the associated coordination request on the same cycle.
		vector<ActiveRequest>::iterator findAssociatedCoordinationRequest = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																						 [&](ActiveRequest const &p)
																						 { return p.vehicleID == associatedVehicleID; });
		ActiveRequestTable.erase(findAssociatedCoordinationRequest);
	}

	// For Transit and truck PriorityRequest
	else if (findVehicleIDOnTable != ActiveRequestTable.end())
		ActiveRequestTable.erase(findVehicleIDOnTable);

	displayConsoleData("Deleted Timed-Out Request");
	loggingData("Deleted Timed-Out Request");
}

/*
	Method for creating Signal Status Message from the Active Request Table.
*/
string PriorityRequestServer::createSSMJsonString(SignalStatus signalStatus)
{
	string ssmJsonString{};
	signalStatus.reset();

	signalStatus.setMinuteOfYear(currentMinuteOfYear);
	signalStatus.setMsOfMinute(currentMsOfMinute);
	signalStatus.setSequenceNumber(getPRSSequenceNumber());
	signalStatus.setUpdateCount(getPRSUpdateCount());
	signalStatus.setRegionalID(regionalID);
	signalStatus.setIntersectionID(intersectionID);
	ssmJsonString = signalStatus.signalStatus2Json(ActiveRequestTable);

	displayConsoleData("SSM will send to MsgEncoder");
	loggingData("SSM will send to MsgEncoder");
	loggingData(ssmJsonString);

	return ssmJsonString;
}

/*
	Method for creating json string from the Active Request Table for the Priority Solver.
*/
string PriorityRequestServer::createJsonStringForPrioritySolver()
{
	string solverJsonString{};
	int noOfRequest{};
	Json::Value jsonObject;
	Json::StreamWriterBuilder builder;
	builder["commentStyle"] = "None";
	builder["indentation"] = "";

	noOfRequest = static_cast<int>(ActiveRequestTable.size());

	if (sentClearRequestForEV || ActiveRequestTable.empty())
	{
		jsonObject["MsgType"] = "ClearRequest";
		sentClearRequest = true;
		sentClearRequestForEV = false;
		displayConsoleData("Clear Request Message will send to PRSolver");
		loggingData("Clear Request Message will send to PRSolver");
	}

	else if (noOfRequest > 0)
	{
		jsonObject["MsgType"] = "PriorityRequest";
		jsonObject["PriorityRequestList"]["noOfRequest"] = noOfRequest;
		jsonObject["PriorityRequestList"]["regionalID"] = regionalID;
		jsonObject["PriorityRequestList"]["intersectionID"] = intersectionID;
		for (int i = 0; i < noOfRequest; i++)
		{
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleID"] = ActiveRequestTable[i].vehicleID;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleType"] = ActiveRequestTable[i].vehicleType;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["basicVehicleRole"] = ActiveRequestTable[i].basicVehicleRole;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"] = ActiveRequestTable[i].signalGroup;

			if (ActiveRequestTable[i].vehicleETA <= 0)
				jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] = 1.0;

			else
				jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] = ActiveRequestTable[i].vehicleETA;

			jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"] = ActiveRequestTable[i].vehicleETADuration / SECOND_MILISECOND_CONVERSION;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["speed_MeterPerSecond"] = ActiveRequestTable[i].vehicleSpeed;

			ActiveRequestTable[i].artForwardTime = getPosixTimestamp();
		}

		sentClearRequest = false;
		displayConsoleData("Priority Request Message will send to PRSolver");
		loggingData("Priority Request Message will send to PRSolver");
	}

	solverJsonString = Json::writeString(builder, jsonObject);
	loggingData(solverJsonString);

	sendSSM = false;
	sendPriorityRequestList = false;

	return solverJsonString;
}

/*
	- Checking whether PRS need to send SSM or not.
		- If there is no request in the list, no need to send SSM
*/
bool PriorityRequestServer::checkSsmSendingRequirement()
{
	return sendSSM;
}

/*
	- Getter for priority request list sending requirement
*/
bool PriorityRequestServer::getPriorityRequestListSendingRequirement()
{
	return sendPriorityRequestList;
}

/*
	- Checking whether PRS need to update the ETA in the ART.
		- If there is no request in the list, no need to update the ETA
		- If vehicle ETA is zero, no need to update.
*/
bool PriorityRequestServer::updateETA()
{
	bool etaUpdateRequirement{false};
	double currentTime = getPosixTimestamp();

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

	if (ActiveRequestTable.empty() && (sentClearRequest == false))
		clearRequestStatus = true;

	return clearRequestStatus;
}

/*
	- Method to update ETA in Active Request Table if vehile ETA is not zero.
*/
void PriorityRequestServer::updateETAInActiveRequestTable()
{
	double currentTime = getPosixTimestamp();
	int relativeETAInMiliSecond{};

	setMinuteOfYear();
	setMsOfMinute();

	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			ActiveRequestTable[i].vehicleETA = ActiveRequestTable[i].vehicleETA - (currentTime - ActiveRequestTable[i].etaUpdateTime);

			if (ActiveRequestTable[i].basicVehicleRole == static_cast<int>(MsgEnum::basicRole::roadsideSource) && ActiveRequestTable[i].vehicleETA <= 0.0)
				ActiveRequestTable[i].vehicleETADuration = static_cast<int>(ActiveRequestTable[i].vehicleETADuration - (currentTime - ActiveRequestTable[i].etaUpdateTime) * SECOND_MILISECOND_CONVERSION);

			if (ActiveRequestTable[i].vehicleETA <= Minimum_ETA)
				ActiveRequestTable[i].vehicleETA = Minimum_ETA;

			relativeETAInMiliSecond = static_cast<int>(ActiveRequestTable[i].vehicleETA * SECOND_MILISECOND_CONVERSION + currentMsOfMinute);

			ActiveRequestTable[i].vehicleETAMinute = currentMinuteOfYear + (relativeETAInMiliSecond / static_cast<int>(SECOND_MINTUTE_CONVERSION * SECOND_MILISECOND_CONVERSION));
			ActiveRequestTable[i].vehicleETASecond = relativeETAInMiliSecond % static_cast<int>(SECOND_MINTUTE_CONVERSION * SECOND_MILISECOND_CONVERSION);

			ActiveRequestTable[i].etaUpdateTime = currentTime;
		}

		etaUpdateTime = currentTime;
	}
}

/*
	Method to print Active Request Table
*/
void PriorityRequestServer::printActiveRequestTable()
{
	double timeStamp = getPosixTimestamp();

	if (!ActiveRequestTable.empty() && consoleOutput)
	{
		cout << "[" << fixed << showpoint << setprecision(4) << timeStamp << "] Active Request Table is following: " << endl;
		cout << "VehicleID"
			 << "	"
			 << "VehicleRole"
			 << "	"
			 << "ETA"
			 << "	"
			 << "ETADuration"
			 << "	"
			 << "SignalGroup" << endl;

		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
			cout << "   " << ActiveRequestTable[i].vehicleID << "       	" << ActiveRequestTable[i].basicVehicleRole << "     	   	" << ActiveRequestTable[i].vehicleETA << "      " << ActiveRequestTable[i].vehicleETADuration << "     	    " << ActiveRequestTable[i].signalGroup << endl;
	}

	else
	{
		displayConsoleData("Active Request Table is empty");
		loggingData("Active Request Table is empty");
	}
}

/*
	Method for obtaining signal group based on vehicle laneID and approachID using MapEngine Library.
*/
void PriorityRequestServer::setRequestedSignalGroup(SignalRequest signalRequest)
{
	int approachID{};

	approachID = plocAwareLib->getApproachIdByLaneId(static_cast<uint16_t>(regionalID), static_cast<uint16_t>(intersectionID), static_cast<uint8_t>(signalRequest.getInBoundLaneID()));
	vehicleRequestedSignalGroup = unsigned(plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalID), static_cast<uint16_t>(intersectionID), static_cast<uint8_t>(approachID),
																			  static_cast<uint8_t>(signalRequest.getInBoundLaneID())));
}

/*
	- Method for defining priority request status.
		- If there is EV in the priority request list for EV priority status will be granted and for rest of the vehicle priority status will be rejected.
		- If there is no EV in the priority request list for Transit and Truck priority status will be granted.
*/
void PriorityRequestServer::setPriorityRequestStatus() // work on this wih traffic controller or priority solver or whom. check page 176 of j2735 pdf
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
	- Method for setting current minute of a year based on GMT(UTC) time
*/
void PriorityRequestServer::setMinuteOfYear()
{
	time_t curr_time;
	curr_time = time(NULL);
	tm *tm_gmt = gmtime(&curr_time);

	int dayOfYear = tm_gmt->tm_yday;
	int currentHour = tm_gmt->tm_hour;
	int currentMinute = tm_gmt->tm_min;

	currentMinuteOfYear = dayOfYear * HOUR_DAY_CONVERSION * MINTUTE_HOUR_CONVERSION + currentHour * MINTUTE_HOUR_CONVERSION + currentMinute;
}

/*
	- Method for setting current millisecond of a minute based on GMT(UTC) time
*/
void PriorityRequestServer::setMsOfMinute()
{
	time_t curr_time;
	curr_time = time(NULL);
	tm *tm_gmt = gmtime(&curr_time);

	int currentSecond = tm_gmt->tm_sec;

	currentMsOfMinute = currentSecond * static_cast<int>(SECOND_MILISECOND_CONVERSION);
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
	- The following method delete the mapPayload file which has *.map.payload extension.
	- The method writes mapPayload in .map.payload formatted file based on the configuration file.
	- It also checks the logging requirement in the config file
*/
void PriorityRequestServer::readconfigFile()
{
	string pathDirectory{};
	string mapPayload{};
	double timeStamp = getPosixTimestamp();
	ofstream mapPayloadOutputfile;
	Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
	ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
	delete reader;

	// Get intersection ID, regional ID, request timed out value for clearing the old request, time interval for logging the system performance data, logging requirement, mapPayload and intersection name
	intersectionID = jsonObject["IntersectionID"].asInt();
	regionalID = jsonObject["RegionalID"].asInt();
	requestTimedOutValue = jsonObject["SRMTimedOutTime"].asDouble();
	timeInterval = jsonObject["SystemPerformanceTimeInterval"].asDouble();
	logging = jsonObject["Logging"].asBool();
	consoleOutput = jsonObject["ConsoleOutput"].asBool();
	mapPayload = jsonObject["MapPayload"].asString();
	intersectionName = jsonObject["IntersectionName"].asString();

	mapPayloadFileName = "/nojournal/bin/" + intersectionName + ".map.payload";

	// Delete old map file
	remove(mapPayloadFileName.c_str());

	// Write the map palyload in a file
	mapPayloadOutputfile.open(mapPayloadFileName);
	mapPayloadOutputfile << "payload"
						 << " " << intersectionName << " " << mapPayload << endl;
	mapPayloadOutputfile.close();

	// Create Log File, if requires
	time_t now = time(0);
	struct tm tstruct;
	char logFileOpenningTime[80];
	tstruct = *localtime(&now);
	strftime(logFileOpenningTime, sizeof(logFileOpenningTime), "%m%d%Y_%H%M%S", &tstruct);

	string logFileName = "/nojournal/bin/log/" + intersectionName + "_prsLog_" + logFileOpenningTime + ".log";

	if (logging)
	{
		logFile.open(logFileName);
		logFile << "[" << fixed << showpoint << setprecision(4) << timeStamp << "] Open PRS log file for " << intersectionName << " intersection" << endl;
	}

	msgSentTime = timeStamp;
}

/*
	- Method for logging data in a file
*/
void PriorityRequestServer::loggingData(string logString)
{
	double timeStamp = getPosixTimestamp();

	if (logging)
	{
		logFile << "\n[" << fixed << showpoint << setprecision(4) << timeStamp << "] ";
		logFile << logString << endl;
	}
}

/*
	- Method for displaying console output
*/
void PriorityRequestServer::displayConsoleData(string consoleString)
{
	double timestamp = getPosixTimestamp();

	if (consoleOutput)
	{
		cout << "\n[" << fixed << showpoint << setprecision(4) << timestamp << "] ";
		cout << consoleString << endl;
	}
}

/*
	Method to check for sending system peformance data to Data-Collector.
*/
bool PriorityRequestServer::sendSystemPerformanceDataLog()
{
	bool sendData{false};
	double currentTime = getPosixTimestamp();

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
	displayConsoleData("System Performance Data Log will send to data collector");
	loggingData("System Performance Data Log will send to data collector");
	msgSentTime = getPosixTimestamp();
	msgReceived = 0;
	msgServed = 0;
	msgRejected = 0;

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
	double currentTime = getPosixTimestamp();
	Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};

	ActiveRequest activeRequest;
	activeRequest.reset();

	displayConsoleData("Received Coordination Request from Signal Coordination Request Generator");
	loggingData("Received Coordination Request from Signal Coordination Request Generator");

	setMinuteOfYear();
	setMsOfMinute();
	setETAUpdateTime();

	reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;
	int noOfCoordinationRequest = jsonObject["noOfCoordinationRequest"].asInt();

	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	{
		if (ActiveRequestTable[i].basicVehicleRole == static_cast<int>(MsgEnum::basicRole::roadsideSource))
		{
			vector<ActiveRequest>::iterator findCoordinationVehicleRoleOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							  [&](ActiveRequest const &p)
																							  { return p.basicVehicleRole == static_cast<int>(MsgEnum::basicRole::roadsideSource); });

			ActiveRequestTable.erase(findCoordinationVehicleRoleOnTable);
			i--;
		}
	}

	if (noOfCoordinationRequest > 0)
	{
		for (int i = 0; i < noOfCoordinationRequest; i++)
		{
			activeRequest.minuteOfYear = jsonObject["minuteOfYear"].asInt();
			activeRequest.secondOfMinute = static_cast<int>(jsonObject["msOfMinute"].asInt() / SECOND_MILISECOND_CONVERSION);
			activeRequest.msgCount = jsonObject["msgCount"].asInt();
			activeRequest.basicVehicleRole = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["basicVehicleRole"].asInt();
			activeRequest.signalGroup = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["requestedPhase"].asInt();
			activeRequest.vehicleID = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["vehicleID"].asInt();
			activeRequest.vehicleType = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["vehicleType"].asInt();
			activeRequest.vehicleETA = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["ETA"].asDouble();
			activeRequest.vehicleETAMinute = currentMinuteOfYear + static_cast<int>(jsonObject["CoordinationRequestList"]["requestorInfo"][i]["ETA"].asDouble() / SECOND_MINTUTE_CONVERSION);
			activeRequest.vehicleETASecond = static_cast<int>((fmod(jsonObject["CoordinationRequestList"]["requestorInfo"][i]["ETA"].asDouble(), SECOND_MINTUTE_CONVERSION)) * currentMsOfMinute);
			activeRequest.vehicleETADuration = jsonObject["CoordinationRequestList"]["requestorInfo"][i]["CoordinationSplit"].asInt();
			activeRequest.vehicleLaneID = coordinationLaneID;
			activeRequest.msgReceivedTime = currentTime;
			activeRequest.etaUpdateTime = currentTime;
			ActiveRequestTable.push_back(activeRequest);
		}

		setPriorityRequestStatus();
		updateETAInActiveRequestTable();
		sendSSM = true;
		sendPriorityRequestList = true;
	}

	else
	{
		sendSSM = false;
		sendPriorityRequestList = false;
	}
}

PriorityRequestServer::~PriorityRequestServer()
{
	logFile.close();
	delete plocAwareLib;
}