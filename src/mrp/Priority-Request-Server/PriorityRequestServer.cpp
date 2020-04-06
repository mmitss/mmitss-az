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
  1. This is the initial revision developed for receiving srm data from the vehicle (transceiver) and mainiting a priority request. 
  2. This script use mapengine library to obtain vehicle status based on active map. 
  3. This script has an API to calculate ETA. 
  4. This script has an API to generate srm json string which is compatible to asn1 j2735 standard.
  5. The generated srm json string will send to Transceiver over a UDP socket.
  6. This script has an API to create Active Request Table on the vehicle side based on the received ssm.
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

using namespace MsgEnum;

const double TIME_GAP_BETWEEN_RECEIVING_SIGNALREQUEST = 10;
const int SEQUENCE_NUMBER_MINLIMIT = 1;
const int SEQUENCE_NUMBER_MAXLIMIT = 127;
const int HOURSINADAY = 24;
const int MINUTESINAHOUR = 60;
const int SECONDSINAMINUTE = 60;
const int SECONDTOMILISECOND = 1000;
const int TIME_GAP_BETWEEN_ETA_Update = 1;

PriorityRequestServer::PriorityRequestServer()
{
}

/*
	- Method for identifying the message type.
*/
int PriorityRequestServer::getMessageType(std::string jsonString)
{
	int messageType{};
	Json::Value jsonObject;
	Json::Reader reader;
	reader.parse(jsonString.c_str(), jsonObject);

	if ((jsonObject["MsgType"]).asString() == "SRM")
	{
		messageType = MsgEnum::DSRCmsgID_srm;
	}

	else
		std::cout << "Message type is unknown" << std::endl;
	// else if ((jsonObject["MsgType"]).asString() == "SPAT")
	// {
	// 	messageType = MsgEnum::DSRCmsgID_spat;
	// }
	return messageType;
}

/*
	Get the Intersection ID from the configuration file.
*/
int PriorityRequestServer::getIntersectionID()
{
	Json::Value jsonObject;
	Json::Reader reader;
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject);

	intersectionID = (jsonObject["IntersectionID"]).asInt();

	return intersectionID;
}

/*
	Get the Regional ID from the configuration file.
*/
int PriorityRequestServer::getRegionalID()
{
	Json::Value jsonObject;
	Json::Reader reader;
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject);

	regionalID = (jsonObject["RegionalID"]).asInt();

	return regionalID;
}

/*
	Get the SRM timed out value from the configuration file.
*/
double PriorityRequestServer::getRequestTimedOutValue()
{

	Json::Value jsonObject;
	Json::Reader reader;
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject);
	requestTimedOutValue = (jsonObject["SRMTimedOutTime"]).asDouble();
	return requestTimedOutValue;
}

/*
	If Intersection ID and Regional ID match then accept the srm
*/
bool PriorityRequestServer::acceptSignalRequest(SignalRequest signalRequest)
{
	bool matchIntersection = false;

	if (intersectionID == signalRequest.getIntersectionID() && regionalID == signalRequest.getRegionalID())
		matchIntersection = true;

	else
		matchIntersection = false;

	return matchIntersection;
}
/*
	If Active Request Table is empty or vehicle ID is not found in the Active Request Table and request type is priority request then add received srm in the Active Request table
*/
bool PriorityRequestServer::addToActiveRequestTable(SignalRequest signalRequest)
{
	bool addRequest = false;
	int vehid = signalRequest.getTemporaryVehicleID();

	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

	if (ActiveRequestTable.empty() && signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest))
		addRequest = true;

	else if (ActiveRequestTable.empty() && findVehicleIDOnTable == ActiveRequestTable.end() && signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest))
		addRequest = true;

	// else
	// 	retval = false;

	return addRequest;
}

/*
	- If Active Request Table is not empty or vehicle ID is already in the Active Request Table then if-
		-msg count or vehicle Signal group or vehicle ETA changed then update Active request Table
*/
bool PriorityRequestServer::updateActiveRequestTable(SignalRequest signalRequest)
{
	bool updateValue = false;
	int vehid = signalRequest.getTemporaryVehicleID();
	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

	if (ActiveRequestTable.empty())

		updateValue = false;

	else if (!ActiveRequestTable.empty() && findVehicleIDOnTable == ActiveRequestTable.end())
		updateValue = false;

	else if (!ActiveRequestTable.empty() && findVehicleIDOnTable != ActiveRequestTable.end() && signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::requestUpdate)) //(findVehicleIDOnTable->vehicleLaneID != signalRequest.getInBoundLaneID() || findVehicleIDOnTable->vehicleETA != (signalRequest.getETA_Minute() * SECONDSINAMINUTE + signalRequest.getETA_Second()) || findVehicleIDOnTable->msgCount != signalRequest.getMsgCount()))
		updateValue = true;

	return updateValue;
}

/*
	If Active Request Table is not empty or vehicle ID is already in the Active Request Table then if-
		-priority request type is priorityCancellation then delete the srm from the Active Request Table
*/
bool PriorityRequestServer::deleteRequestfromActiveRequestTable(SignalRequest signalRequest)
{
	bool deleteRequest = false;
	int vehid = signalRequest.getTemporaryVehicleID();
	// int requestType = signalRequest.getPriorityRequestType();
	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

	if (ActiveRequestTable.empty())
		deleteRequest = false;

	else if (!ActiveRequestTable.empty() && findVehicleIDOnTable == ActiveRequestTable.end())
		deleteRequest = false;

	else if (!ActiveRequestTable.empty() && findVehicleIDOnTable != ActiveRequestTable.end() && signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityCancellation)) //priorityCancellation=3
		deleteRequest = true;

	return deleteRequest;
}

/*
	If there is no signal request is received from a vehicle for more than predifined time(10sec), PRS needs to delete that request.
*/
bool PriorityRequestServer::shouldDeleteTimedOutRequestfromActiveRequestTable()
{
	bool deleteSignalRequest = false;

	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if ((getMsOfMinute() / SECONDTOMILISECOND > ActiveRequestTable[i].secondOfMinute) && (getMsOfMinute() / SECONDTOMILISECOND - ActiveRequestTable[i].secondOfMinute) >= requestTimedOutValue)
			{
				deleteSignalRequest = true;
				setRequestTimedOutVehicleID(ActiveRequestTable[i].vehicleID);
				break;
			}
			else if ((getMsOfMinute() / SECONDTOMILISECOND < ActiveRequestTable[i].secondOfMinute) && (getMsOfMinute() / SECONDTOMILISECOND + SECONDSINAMINUTE - ActiveRequestTable[i].secondOfMinute) >= requestTimedOutValue)
			{
				deleteSignalRequest = true;
				setRequestTimedOutVehicleID(ActiveRequestTable[i].vehicleID);
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
	bool bEVInList = false;

	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].basicVehicleRole == static_cast<int>(MsgEnum::basicRole::fire))
			{
				bEVInList = true;
				break;
			}
			else
				bEVInList = false;
		}
	}

	else
		bEVInList = false;

	return bEVInList;
}

/*
	Method to check if received SRM is from EV or not
*/
bool PriorityRequestServer::findEVInRequest(SignalRequest signalRequest)
{
	bool bEVRequest = false;

	if (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::fire))
		bEVRequest = true;

	return bEVRequest;
}

/*
    - Obtain Split Phase information if EV is in List
        - Copy the EV request object.
		- Change the signal group with split phase
		- Append that object in the ART
*/
void PriorityRequestServer::findSplitPhase()
{
	std::vector<int>::iterator it;
	std::vector<ActiveRequest> temporarySplitPriorityRequestList;
	int temporarySplitPhase{};

	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	{
		if (ActiveRequestTable[i].basicVehicleRole == static_cast<int>(MsgEnum::basicRole::fire))
		{
			switch (ActiveRequestTable[i].signalGroup)
			{
			case 1:
				temporarySplitPhase = 6;
				// it = std::find(P21.begin(), P21.end(), temporarySplitPhase);
				// if (it != P21.end())
				// {
				//     temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				//     temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				// }
				temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				break;

			case 2:
				temporarySplitPhase = 5;
				// it = std::find(P21.begin(), P21.end(), temporarySplitPhase);
				// if (it != P21.end())
				// {
				//     temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				//     temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				// }
				temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				break;

			case 3:
				temporarySplitPhase = 7;
				// it = std::find(P22.begin(), P22.end(), temporarySplitPhase);
				// if (it != P22.end())
				// {
				// 	temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				// 	temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				// }
				temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				break;

			case 4:
				temporarySplitPhase = 8;
				// it = std::find(P22.begin(), P22.end(), temporarySplitPhase);
				// if (it != P22.end())
				// {
				// 	temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				// 	temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				// }
				temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				break;

			case 5:
				temporarySplitPhase = 2;
				// it = std::find(P11.begin(), P11.end(), temporarySplitPhase);
				// if (it != P11.end())
				// {
				// 	temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				// 	temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				// }
				temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				break;

			case 6:
				temporarySplitPhase = 1;
				// it = std::find(P11.begin(), P11.end(), temporarySplitPhase);
				// if (it != P11.end())
				// {
				// 	temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				// 	temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				// }
				temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				break;

			case 7:
				temporarySplitPhase = 3;
				// it = std::find(P12.begin(), P12.end(), temporarySplitPhase);
				// if (it != P12.end())
				// {
				// 	temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				// 	temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				// }
				temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				break;

			case 8:
				temporarySplitPhase = 4;
				// it = std::find(P12.begin(), P12.end(), temporarySplitPhase);
				// if (it != P12.end())
				// {
				// 	temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				// 	temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				// }
				temporarySplitPriorityRequestList.push_back(ActiveRequestTable[i]);
				temporarySplitPriorityRequestList[i].signalGroup = temporarySplitPhase;
				break;

			default:
				break;
			}
		}
	}
	ActiveRequestTable.insert(ActiveRequestTable.end(), temporarySplitPriorityRequestList.begin(), temporarySplitPriorityRequestList.end());
}

/*
	- Method to create, update and delete Active Request Table
	- If the the request type is update:
		- If the update request is from transit or truck, find the position of the corresponding vehicle and update the information.
		- If the update request is from EV, remove the vehicle request(both requested phase and split phase request) from the list.
			- Then append the new received update request in the list 
			- Then append the request regarding the split phase
*/
void PriorityRequestServer::managingSignalRequestTable(SignalRequest signalRequest)
{
	ActiveRequest activeRequest;
	activeRequest.reset();
	int vehid{};
	// std::cout << "Add To ActiveRequesttable(True/False): " << addToActiveRequestTable(signalRequest) << std::endl;
	if (acceptSignalRequest(signalRequest) == true)
	{
		if (addToActiveRequestTable(signalRequest) == true)
		{
			setPRSUpdateCount();
			setVehicleType(signalRequest);
			activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
			activeRequest.requestID = signalRequest.getRequestID();
			activeRequest.msgCount = signalRequest.getMsgCount();
			activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
			activeRequest.vehicleType = vehicleType;
			activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
			activeRequest.vehicleETA = signalRequest.getETA_Minute() * SECONDSINAMINUTE + signalRequest.getETA_Second();
			activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
			activeRequest.minuteOfYear = getMinuteOfYear();
			activeRequest.secondOfMinute = getMsOfMinute() / SECONDTOMILISECOND;
			activeRequest.signalGroup = getSignalGroup(signalRequest);
			ActiveRequestTable.push_back(activeRequest);
			if (findEVInRequest(signalRequest) == true)
			{
				findSplitPhase();
			}
			updateETAInActiveRequestTable();
		}

		else if (updateActiveRequestTable(signalRequest) == true)
		{
			setPRSUpdateCount();
			vehid = signalRequest.getTemporaryVehicleID();
			if (signalRequest.getBasicVehicleRole() == static_cast<int>(MsgEnum::basicRole::fire)) //For EV
			{
				for (int i = 0; i < 2; i++)
				{
					std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

					ActiveRequestTable.erase(findVehicleIDOnTable);
				}

				activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
				activeRequest.requestID = signalRequest.getRequestID();
				activeRequest.msgCount = signalRequest.getMsgCount();
				activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
				activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
				activeRequest.vehicleETA = signalRequest.getETA_Minute() * SECONDSINAMINUTE + signalRequest.getETA_Second();
				activeRequest.vehicleETADuration = signalRequest.getETA_Duration();
				//activeRequest.prsStatus = getPriorityRequestStatus();
				activeRequest.minuteOfYear = getMinuteOfYear();
				activeRequest.secondOfMinute = getMsOfMinute() / SECONDTOMILISECOND;
				activeRequest.signalGroup = getSignalGroup(signalRequest);
				ActiveRequestTable.push_back(activeRequest);
				//setPriorityRequestStatus(signalRequest);
				if (findEVInRequest(signalRequest) == true)
				{
					findSplitPhase();
				}
			}

			else //For Transit and Truck
			{
				std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																						 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

				findVehicleIDOnTable->vehicleID = signalRequest.getTemporaryVehicleID();
				findVehicleIDOnTable->requestID = signalRequest.getRequestID();
				findVehicleIDOnTable->msgCount = signalRequest.getMsgCount();
				findVehicleIDOnTable->basicVehicleRole = signalRequest.getBasicVehicleRole();
				findVehicleIDOnTable->vehicleLaneID = signalRequest.getInBoundLaneID();
				findVehicleIDOnTable->vehicleETA = signalRequest.getETA_Minute() * SECONDSINAMINUTE + signalRequest.getETA_Second();
				findVehicleIDOnTable->vehicleETADuration = signalRequest.getETA_Duration();
				//findVehicleIDOnTable->prsStatus = signalRequest.getPriorityRequestType();
				findVehicleIDOnTable->minuteOfYear = getMinuteOfYear();
				findVehicleIDOnTable->secondOfMinute = getMsOfMinute() / SECONDTOMILISECOND;
				findVehicleIDOnTable->signalGroup = getSignalGroup(signalRequest);
			}
			updateETAInActiveRequestTable();
		}
		else if (deleteRequestfromActiveRequestTable(signalRequest) == true)
		{
			vehid = signalRequest.getTemporaryVehicleID();

			//If the delete request is for EV we need to delete both EV request (through and left turn phase)
			if (findEVInRequest(signalRequest) == true)
			{
				for (int i = 0; i < 2; i++)
				{
					std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																							 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

					if (findVehicleIDOnTable != ActiveRequestTable.end())
					{
						ActiveRequestTable.erase(findVehicleIDOnTable);
					}
				}
			}

			else
			{
				std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																						 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });
				if (findVehicleIDOnTable != ActiveRequestTable.end())
				{
					ActiveRequestTable.erase(findVehicleIDOnTable);
				}
			}
			updateETAInActiveRequestTable();
		}
		// else
		// 	std::cout << "Unknown priority request type" << std::endl;
		setPriorityRequestStatus();
	}
}
/*
	Method to delete vehicle info from Active Request Table if Infrustracture doesn't receive and SRM for predefined time
*/
void PriorityRequestServer::deleteTimedOutRequestfromActiveRequestTable()
{
	int vehid{};

	vehid = getRequestTimedOutVehicleID();

	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

	//For EV we have to delete two request
	if (findVehicleIDOnTable->vehicleType == static_cast<int>(MsgEnum::basicRole::fire) && findVehicleIDOnTable != ActiveRequestTable.end())
	{
		ActiveRequestTable.erase(findVehicleIDOnTable);

		//Deleting the request related to split phase.
		std::vector<ActiveRequest>::iterator findSplitPhaseEV = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });
		ActiveRequestTable.erase(findSplitPhaseEV);
	}

	else if (findVehicleIDOnTable != ActiveRequestTable.end()) //For Transit and truck
	{
		ActiveRequestTable.erase(findVehicleIDOnTable);
	}
}

/*
	Method for creating Signal Status Message from the Active Request Table.
*/
std::string PriorityRequestServer::createSSMJsonString(SignalStatus signalStatus)
{
	std::string ssmJsonString{};

	signalStatus.setMinuteOfYear(getMinuteOfYear());
	signalStatus.setMsOfMinute(getMsOfMinute());
	signalStatus.setSequenceNumber(getPRSSequenceNumber());
	signalStatus.setUpdateCount(getPRSUpdateCount());
	signalStatus.setRegionalID(getRegionalID());
	signalStatus.setIntersectionID(getIntersectionID());
	ssmJsonString = signalStatus.signalStatus2Json(ActiveRequestTable);

	return ssmJsonString;
}

/*
	Method for creating json string from the Active Request Table for the Priority Solver.
*/
std::string PriorityRequestServer::createJsonStringForPrioritySolver()
{
	std::string solverJsonString{};
	int noOfRequest{};
	Json::Value jsonObject;
	Json::FastWriter fastWriter;
	Json::StyledStreamWriter styledStreamWriter;
	std::ofstream outputter("schedule.json");

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
			if(ActiveRequestTable[i].vehicleETA <= 0)
				jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] = 1.0;
			else
				jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] = ActiveRequestTable[i].vehicleETA;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"] = ActiveRequestTable[i].vehicleETADuration;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"] = ActiveRequestTable[i].signalGroup;
			jsonObject["PriorityRequestList"]["requestorInfo"][i]["priorityRequestStatus"] = ActiveRequestTable[i].prsStatus;
		}
	}

	else
	{	
		jsonObject["MsgType"] = "ClearRequest";
		std::cout << "Sent Clear Request to Solver " << std::endl;
	}
	solverJsonString = fastWriter.write(jsonObject);
	styledStreamWriter.write(outputter, jsonObject);

	return solverJsonString;
}

/*
	- Checking whether PRS need to update the ETA in the ART.
		- If there is no request in the list no need to update the ETA
		- If vehicle ETA is zero no need to update.
*/
bool PriorityRequestServer::updateETA()
{
	bool bUpdateETA = false;
	// std::ofstream outputfile;
	// outputfile.open("timelog.txt", std::ios_base::app);
	double timeDifference = abs(getMsOfMinute() / SECONDTOMILISECOND);

	if (!ActiveRequestTable.empty())
	{
		// outputfile << "current time: " << timeDifference << std::endl;
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (timeDifference - ActiveRequestTable[i].secondOfMinute >= TIME_GAP_BETWEEN_ETA_Update && ActiveRequestTable[i].vehicleETA != 0)
			{
				// outputfile << ActiveRequestTable[i].secondOfMinute << std::endl;
				bUpdateETA = true;
				break;
			}
		}
	}
	// outputfile << "Returned Value " << bUpdateETA << std::endl;
	// outputfile.close();

	// else
	// 	bUpdateETA = false;

	return bUpdateETA;
}

/*
	- Method for checking whether clear request has to be sent to solver
*/
bool PriorityRequestServer::sendClearRequest()
{
	bool bSendClearRequest = false;

	if (ActiveRequestTable.size() == 0)
		bSendClearRequest = true;
	else
		bSendClearRequest = false;

	return bSendClearRequest;
}

/*
	- Method to update ETA in Active Request Table if vehile ETA is not zero.
*/
void PriorityRequestServer::updateETAInActiveRequestTable()
{
	if (!ActiveRequestTable.empty())
	{

		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].secondOfMinute < (getMsOfMinute() / SECONDTOMILISECOND) && ((getMsOfMinute() / SECONDTOMILISECOND) - ActiveRequestTable[i].secondOfMinute) >= TIME_GAP_BETWEEN_ETA_Update && ActiveRequestTable[i].vehicleETA != 0)
			{
				ActiveRequestTable[i].vehicleETA = ActiveRequestTable[i].vehicleETA - ((getMsOfMinute() / SECONDTOMILISECOND) - ActiveRequestTable[i].secondOfMinute);
				// ActiveRequestTable[i].secondOfMinute = getMsOfMinute() / SECONDTOMILISECOND;
				// ActiveRequestTable[i].minuteOfYear = getMinuteOfYear();
			}

			else if (ActiveRequestTable[i].secondOfMinute > getMsOfMinute() * SECONDTOMILISECOND && (ActiveRequestTable[i].secondOfMinute - getMsOfMinute() / SECONDTOMILISECOND) >= TIME_GAP_BETWEEN_ETA_Update && ActiveRequestTable[i].vehicleETA != 0)
			{
				ActiveRequestTable[i].vehicleETA = ActiveRequestTable[i].vehicleETA - (getMsOfMinute() / SECONDTOMILISECOND + SECONDSINAMINUTE - ActiveRequestTable[i].secondOfMinute);
				// ActiveRequestTable[i].secondOfMinute = getMsOfMinute() / SECONDTOMILISECOND;
				// ActiveRequestTable[i].minuteOfYear = getMinuteOfYear();
			}
		}
	}
}

/*
	Method to print Active Request Table
*/
void PriorityRequestServer::printvector()
{
	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			std::cout << ActiveRequestTable[i].vehicleID << " " << ActiveRequestTable[i].vehicleLaneID << " " << ActiveRequestTable[i].vehicleETA << std::endl;
		}
	}

	else
		std::cout << "Active Request Table is empty" << std::endl;
}

/*
	- Method for defining priority request status.
		- If there is EV in the priority request list for EV priority status will be granted and for rest of the vehicle priority status will be rejected.
		- If there is no EV in the priority request list for Transit and Truck priority status will be granted.
*/
void PriorityRequestServer::setPriorityRequestStatus() //work on this wih traffic controller or priority solver or whom. check page 176 of j2735 pdf
{
	if (findEVInList() == true)
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::fire)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::granted);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::rejected);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::rejected);

			else
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::unavailable);
		}
	}

	else if (findEVInList() == false)
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::granted);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::granted);
			else
				ActiveRequestTable[i].prsStatus = static_cast<int>(MsgEnum::requestStatus::unavailable); // In standard it is unknown
		}
	}
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

	// if (addToActiveRequestTable(signalRequest) == true || updateActiveRequestTable(signalRequest) == true)
	// {
	// 	if (updateCount < SEQUENCE_NUMBER_MAXLIMIT)
	// 		updateCount++;

	// 	else
	// 		updateCount = SEQUENCE_NUMBER_MINLIMIT;
	// }
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

	minuteOfYear = (dayOfYear - 1) * HOURSINADAY * MINUTESINAHOUR + currentHour * MINUTESINAHOUR + currentMinute;

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
	msOfMinute = currentSecond * SECONDTOMILISECOND;

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
	int intersectionId{};
	int regionalId{};
	int approachID{};
	std::string intersectionName{};
	std::string fmap{};
	bool singleFrame = false;
	Json::Value jsonObject;
	Json::Reader reader;
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject);

	intersectionId = getIntersectionID();
	regionalId = getRegionalID();
	intersectionName = (jsonObject["IntersectionName"]).asString();
	fmap = "/nojournal/bin/" + intersectionName + ".map.payload";

	LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
	approachID = plocAwareLib->getApproachIdByLaneId(static_cast<uint16_t>(regionalId), static_cast<uint16_t>(intersectionId), static_cast<uint8_t>(signalRequest.getInBoundLaneID()));
	phaseNo = unsigned(plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalId), static_cast<uint16_t>(intersectionId), static_cast<uint8_t>(approachID),
														  static_cast<uint8_t>(signalRequest.getInBoundLaneID())));

	delete plocAwareLib;
	return phaseNo;
}

/*
	Method for writing mapPayload in .map.payload file.
*/
void PriorityRequestServer::writeMAPPayloadInFile()
{
	Json::Value jsonObject;
	Json::Reader reader;
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject);

	std::string intersectionName = (jsonObject["IntersectionName"]).asString();
	std::string mapPayload = (jsonObject["MapPayload"]).asString();

	const char *path = "/nojournal/bin";
	std::stringstream ss;
	ss << path;
	std::string s;
	ss >> s;
	std::ofstream outputfile;

	outputfile.open(s + "/" + intersectionName + ".map.payload");

	outputfile << "payload"
			   << " "
			   << intersectionName << " " << mapPayload << std::endl;
	outputfile.close();
}

/*
	Method for deleting mapPayload file which is in *.map.payload file.
*/
void PriorityRequestServer::deleteMapPayloadFile()
{
	Json::Value jsonObject;
	Json::Reader reader;
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject);

	std::string intersectionName = (jsonObject["IntersectionName"]).asString();

	std::string deleteFileName = "/nojournal/bin/" + intersectionName + ".map.payload";
	remove(deleteFileName.c_str());
}

PriorityRequestServer::~PriorityRequestServer()
{
}