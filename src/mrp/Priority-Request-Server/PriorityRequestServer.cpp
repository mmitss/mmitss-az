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

// using namespace GeoUtils;
using namespace MsgEnum;

const int TIME_GAP_BETWEEN_RECEIVING_SIGNALREQUEST = 5;
const int SEQUENCE_NUMBER_MINLIMIT = 1;
const int SEQUENCE_NUMBER_MAXLIMIT = 127;
const int HOURSINADAY = 24;
const int MINUTESINAHOUR = 60;
const int SECONDSINAMINUTE = 60;
const int SECONDTOMILISECOND = 1000;
const int TIME_GAP_BETWEEN_ETA_Update =1;

PriorityRequestServer::PriorityRequestServer()
{
	std::vector<int> phaseGroup(8,0);
}

int PriorityRequestServer::getMessageType(std::string jsonString)
{
	Json::Value jsonObject;
	Json::Reader reader;
	reader.parse(jsonString.c_str(), jsonObject);

	if ((jsonObject["MsgType"]).asString() == "SRM")
	{
		messageType = MsgEnum::DSRCmsgID_srm;
	}

	else if ((jsonObject["MsgType"]).asString() == "SPAT")
	{
		messageType = MsgEnum::DSRCmsgID_spat;
	}
	return messageType;
}

/*
	Get the Intersection ID from the IntersectionConfiguration file.
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
	Get the Regional ID from the IntersectionConfiguration file.
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
	If Intersection ID and Regional ID match then accept the srm
*/
bool PriorityRequestServer::aceeptSignalRequest(SignalRequest signalRequest)
{
	bool matchIntersection = false;

	if (getIntersectionID() == signalRequest.getIntersectionID() && getRegionalID() == signalRequest.getRegionalID())
		matchIntersection = true;

	else
		matchIntersection = false;

	return matchIntersection;
}
/*
	If Active Request Table is empty or vehicle ID is not found in the Active Request Table add received srm in the Active Request table
*/
bool PriorityRequestServer::addToActiveRequesttable(SignalRequest signalRequest)
{
	bool addRequest = false;
	int vehid = signalRequest.getTemporaryVehicleID();

	std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

	if (ActiveRequestTable.empty() && signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest))
		addRequest = true;

	else if (findVehicleIDOnTable == ActiveRequestTable.end() && signalRequest.getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityRequest))
		addRequest = true;

	// else
	// 	retval = false;

	return addRequest;
}

/*
	If Active Request Table is not empty or vehicle ID is already in the Active Request Table then if-
		-msg count or vehicle lane id or vehicle ETA changed then update Active request Table
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

void PriorityRequestServer::setRequestTimedOutVehicleID(int timedOutVehicleID)
{
	requestTimedOutVehicleID = timedOutVehicleID;
}

int PriorityRequestServer::getRequestTimedOutVehicleID()
{
	return requestTimedOutVehicleID;
}

/*
	If there is no signal request is received from a vehicle for more than 5 minutes delete that request.
*/
bool PriorityRequestServer::shouldDeleteTimedOutRequestfromActiveRequestTable()
{
	bool deleteSignalRequest = false;

	if (!ActiveRequestTable.empty())
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if ((getMinuteOfYear() - ActiveRequestTable[i].minuteOfYear) >= TIME_GAP_BETWEEN_RECEIVING_SIGNALREQUEST)
			{
				deleteSignalRequest = true;
				setRequestTimedOutVehicleID(ActiveRequestTable[i].vehicleID);
				break;
			}
		}
	}
	return deleteSignalRequest; //check this return logic
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

// /*
//     -Obtain Split PHase information if EV is in List
//         - If split phase is not found set it zero
// */
// void PriorityRequestSolver::findSplitPhase()
// {
//     std::vector<int>::iterator it;
//     vector<RequestList> temporarySplitPriorityRequestList;
//     int temporarySplitPhase{};
//     // Json::Value jsonObject_config;
//     // Json::Reader reader;
//     // std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
//     // std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
//     // reader.parse(configJsonString.c_str(), jsonObject_config);
//     // vehicleType = (jsonObject_config["VehicleType"]).asInt();
//     for (size_t i = 0; i < priorityRequestList.size(); i++)
//     {
//         if (priorityRequestList[i].vehicleType == 2)
//         {
//             switch (priorityRequestList[i].requestedPhase)
//             {
//             case 1:
//                 temporarySplitPhase = 6;
//                 it = std::find(P21.begin(), P21.end(), temporarySplitPhase);
//                 if (it != P21.end())
//                 {
//                     temporarySplitPriorityRequestList.push_back(priorityRequestList[i]);
//                     temporarySplitPriorityRequestList[i].requestedPhase = temporarySplitPhase;
//                 }
                
//                 break;

//             case 2:
//                 temporarySplitPhase = 5;
//                 it = std::find(P21.begin(), P21.end(), temporarySplitPhase);
//                 if (it != P21.end())
//                 {
//                     temporarySplitPriorityRequestList.push_back(priorityRequestList[i]);
//                     temporarySplitPriorityRequestList[i].requestedPhase = temporarySplitPhase;
//                 }
//                 // if (it != P21.end())
//                 //     priorityRequestList[i].splitPhase = temporarySplitPhase;
//                 // else
//                 //     priorityRequestList[i].splitPhase = 0;
//                 break;

//             case 3:
//                 temporarySplitPhase = 7;
//                 it = std::find(P22.begin(), P22.end(), temporarySplitPhase);
//                 if (it != P22.end())
//                 {
//                     temporarySplitPriorityRequestList.push_back(priorityRequestList[i]);
//                     temporarySplitPriorityRequestList[i].requestedPhase = temporarySplitPhase;
//                 }
//                 break;

//             case 4:
//                 temporarySplitPhase = 8;
//                 it = std::find(P22.begin(), P22.end(), temporarySplitPhase);
//                 if (it != P22.end())
//                 {
//                     temporarySplitPriorityRequestList.push_back(priorityRequestList[i]);
//                     temporarySplitPriorityRequestList[i].requestedPhase = temporarySplitPhase;
//                 }
//                 break;

//             case 5:
//                 temporarySplitPhase = 2;
//                 it = std::find(P11.begin(), P11.end(), temporarySplitPhase);
//                 if (it != P11.end())
//                 {
//                     temporarySplitPriorityRequestList.push_back(priorityRequestList[i]);
//                     temporarySplitPriorityRequestList[i].requestedPhase = temporarySplitPhase;
//                 }
//                 break;

//             case 6:
//                 temporarySplitPhase = 1;
//                 it = std::find(P11.begin(), P11.end(), temporarySplitPhase);
//                 if (it != P11.end())
//                 {
//                     temporarySplitPriorityRequestList.push_back(priorityRequestList[i]);
//                     temporarySplitPriorityRequestList[i].requestedPhase = temporarySplitPhase;
//                 }
//                 break;

//             case 7:
//                 temporarySplitPhase = 3;
//                 it = std::find(P12.begin(), P12.end(), temporarySplitPhase);
//                 if (it != P12.end())
//                 {
//                     temporarySplitPriorityRequestList.push_back(priorityRequestList[i]);
//                     temporarySplitPriorityRequestList[i].requestedPhase = temporarySplitPhase;
//                 }
//                 break;

//             case 8:
//                 temporarySplitPhase = 4;
//                 it = std::find(P12.begin(), P12.end(), temporarySplitPhase);
//                 if (it != P12.end())
//                 {
//                     temporarySplitPriorityRequestList.push_back(priorityRequestList[i]);
//                     temporarySplitPriorityRequestList[i].requestedPhase = temporarySplitPhase;
//                 }
//                 break;

//             default:
//                 break;
//             }
//         }
//     }
//     priorityRequestList.insert(priorityRequestList.end(), temporarySplitPriorityRequestList.begin(), temporarySplitPriorityRequestList.end());
// }

// /*
//     - If EV is priority request list, delete all the priority request from the list apart from EV
// */
// void PriorityRequestSolver::modifyPriorityRequestList()
// {
//     int temporaryVehicleID{};

//     for (size_t i = 0; i < priorityRequestList.size(); i++)
//     {
//         temporaryVehicleID = priorityRequestList[i].vehicleID;
//         if (priorityRequestList[i].vehicleType != 2)
//         {
//             vector<RequestList>::iterator findVehicleIDOnList = std::find_if(std::begin(priorityRequestList), std::end(priorityRequestList),
//                                                                              [&](RequestList const &p) { return p.vehicleID == temporaryVehicleID; });

//             priorityRequestList.erase(findVehicleIDOnList);
//             i--;
//             // if(findVehicleIDOnList != priorityRequestList.end())
//             // {
//             //     priorityRequestList.erase(findVehicleIDOnList);
//             //     i--;
//             // }
//         }
//     }
// }

/*
	Method to create, update and delete Active Request Table
*/
std::vector<ActiveRequest> PriorityRequestServer::creatingSignalRequestTable(SignalRequest signalRequest)
{
	ActiveRequest activeRequest;
	std::cout << "Add To ActiveRequesttable(True/False): " << addToActiveRequesttable(signalRequest) << std::endl;
	if (aceeptSignalRequest(signalRequest) == true)
	{
		if (addToActiveRequesttable(signalRequest) == true)
		{
			//setPriorityRequestStatus(signalRequest);
			activeRequest.vehicleID = signalRequest.getTemporaryVehicleID();
			activeRequest.requestID = signalRequest.getRequestID();
			activeRequest.msgCount = signalRequest.getMsgCount();
			activeRequest.basicVehicleRole = signalRequest.getBasicVehicleRole();
			activeRequest.vehicleLaneID = signalRequest.getInBoundLaneID();
			activeRequest.vehicleETA = signalRequest.getETA_Minute() * SECONDSINAMINUTE + signalRequest.getETA_Second();
			//activeRequest.prsStatus = getPriorityRequestStatus();
			activeRequest.minuteOfYear = getMinuteOfYear();
			activeRequest.secondOfMinute = getMsOfMinute()/SECONDTOMILISECOND;
			activeRequest.signalGroup = getSignalGroup(signalRequest);
			ActiveRequestTable.push_back(activeRequest);
			//setPriorityRequestStatus(signalRequest);
		}

		if (updateActiveRequestTable(signalRequest) == true)
		{
			//setPriorityRequestStatus(signalRequest);
			int vehid = signalRequest.getTemporaryVehicleID();
			std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																					 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });

			findVehicleIDOnTable->vehicleID = signalRequest.getTemporaryVehicleID();
			findVehicleIDOnTable->requestID = signalRequest.getRequestID();
			findVehicleIDOnTable->msgCount = signalRequest.getMsgCount();
			findVehicleIDOnTable->basicVehicleRole = signalRequest.getBasicVehicleRole();
			findVehicleIDOnTable->vehicleLaneID = signalRequest.getInBoundLaneID();
			findVehicleIDOnTable->vehicleETA = signalRequest.getETA_Minute() * SECONDSINAMINUTE + signalRequest.getETA_Second();
			//findVehicleIDOnTable->prsStatus = signalRequest.getPriorityRequestType();
			findVehicleIDOnTable->minuteOfYear = getMinuteOfYear();
			findVehicleIDOnTable->secondOfMinute = getMsOfMinute()/SECONDTOMILISECOND;
			findVehicleIDOnTable->signalGroup = getSignalGroup(signalRequest);
		}

		if (deleteRequestfromActiveRequestTable(signalRequest) == true)
		{
			int vehid = signalRequest.getTemporaryVehicleID();

			std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																					 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });
			if (findVehicleIDOnTable != ActiveRequestTable.end())
			{
				ActiveRequestTable.erase(findVehicleIDOnTable);
			}
			//setPriorityRequestStatus(signalRequest);
		}
		setPriorityRequestStatus();
	}

	return ActiveRequestTable;
}
/*
	Method to delete vehicle info from Active Request Table if Infrustracture doesn't receive and SRM for 5minutes
*/
void PriorityRequestServer::deleteTimedOutRequestfromActiveRequestTable()
{
	if (shouldDeleteTimedOutRequestfromActiveRequestTable() == true) //Check with Dr. Head
	{
		int vehid = getRequestTimedOutVehicleID();

		std::vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																				 [&](ActiveRequest const &p) { return p.vehicleID == vehid; });
		if (findVehicleIDOnTable != ActiveRequestTable.end())
		{
			ActiveRequestTable.erase(findVehicleIDOnTable);
		}
	}
}

/*
	Method to update ETA in Active Request Table
*/
void PriorityRequestServer::updateETAInActiveRequestTable()
{
	if (!ActiveRequestTable.empty())
	{
		
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if(ActiveRequestTable[i].minuteOfYear == getMinuteOfYear() && (getMsOfMinute()*SECONDTOMILISECOND -ActiveRequestTable[i].secondOfMinute)>= TIME_GAP_BETWEEN_ETA_Update)
			{
				ActiveRequestTable[i].vehicleETA = ActiveRequestTable[i].vehicleETA - (getMsOfMinute()*SECONDTOMILISECOND - ActiveRequestTable[i].secondOfMinute);
				ActiveRequestTable[i].secondOfMinute = getMsOfMinute()*SECONDTOMILISECOND;
				ActiveRequestTable[i].minuteOfYear = getMinuteOfYear();
			}

			else if(ActiveRequestTable[i].minuteOfYear != getMinuteOfYear() && abs(getMsOfMinute()*SECONDTOMILISECOND - ActiveRequestTable[i].secondOfMinute)>= TIME_GAP_BETWEEN_ETA_Update)
			{
				ActiveRequestTable[i].vehicleETA = ActiveRequestTable[i].vehicleETA - TIME_GAP_BETWEEN_ETA_Update;
				ActiveRequestTable[i].secondOfMinute = getMsOfMinute()*SECONDTOMILISECOND;
				ActiveRequestTable[i].minuteOfYear = getMinuteOfYear();
			}
		}
	}
}

/*
	Method to print Active Request Table
*/
void PriorityRequestServer::printvector()
{
	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	{
		std::cout << ActiveRequestTable[i].vehicleID << " " << ActiveRequestTable[i].vehicleLaneID << " " << ActiveRequestTable[i].vehicleETA << std::endl;
	}
}

/*
	Priority Policy: 1.Ambulance 2.Fire Truck 3.Police 4.School Bus(currently not present in MsgEnum namespace) 5.Transit 6.Truck
*/
void PriorityRequestServer::setPriorityRequestStatus() //work on this wih traffic controller or priority solver or whom. check page 176 of j2735 pdf
{
	if (findEVInList()== true)
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::fire)))
				priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
				priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::rejected);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
				priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::rejected);			
			
		}
	}

	else if (findEVInList()== false)
	{
		for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		{
			if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
				priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);

			else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
				priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
		}

	}

	// if (ActiveRequestTable.empty())
	// {
	// 	priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
	// }

	// else if (!ActiveRequestTable.empty() && signalRequest.getBasicVehicleRole() == (static_cast<int>(MsgEnum::basicRole::fire)))
	// {
	// 	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	// 	{
	// 		if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::fire)))
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);

	// 		else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::rejected);

	// 		else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::rejected);			
			
	// 	}
	// }

	// else if (!ActiveRequestTable.empty() && signalRequest.getBasicVehicleRole() == (static_cast<int>(MsgEnum::basicRole::transit) || static_cast<int>(MsgEnum::basicRole::truck)) && findEVInList() == true)
	// {
	// 	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	// 	{
	// 		if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::rejected);

	// 		else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::rejected);
			
	// 		else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::fire)))
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
	// 	}
	//}

	// else if (!ActiveRequestTable.empty() && signalRequest.getBasicVehicleRole() == (static_cast<int>(MsgEnum::basicRole::transit) || static_cast<int>(MsgEnum::basicRole::truck)) && findEVInList() == false)
	// {
	// 	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	// 	{
	// 		if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);

	// 		else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::truck)))
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
	// 	}
	// }


	// else if (!ActiveRequestTable.empty() && signalRequest.getBasicVehicleRole() == (static_cast<int>(MsgEnum::basicRole::transit) || static_cast<int>(MsgEnum::basicRole::truck)))
	// {
	// 	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	// 	{
	// 		if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::ambulance) || static_cast<int>(MsgEnum::basicRole::fire) || static_cast<int>(MsgEnum::basicRole::police)))
	// 		{
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::processing);
	// 			break;
	// 		}

	// 		else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::transit)))
	// 		{
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::watchOtherTraffic);
	// 			break;
	// 		}

	// 		else
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
	// 	}
	// }

	// else if (!ActiveRequestTable.empty() && signalRequest.getBasicVehicleRole() == (static_cast<int>(MsgEnum::basicRole::police)))
	// {
	// 	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	// 	{
	// 		if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::ambulance) || static_cast<int>(MsgEnum::basicRole::fire)))
	// 		{
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::processing);
	// 			break;
	// 		}

	// 		else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::police)))
	// 		{
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::watchOtherTraffic);
	// 			break;
	// 		}

	// 		else
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
	// 	}
	// }

	// else if (!ActiveRequestTable.empty() && signalRequest.getBasicVehicleRole() == (static_cast<int>(MsgEnum::basicRole::fire)))
	// {
	// 	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	// 	{
	// 		if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::ambulance)))
	// 		{
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::processing);
	// 			break;
	// 		}

	// 		else if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::fire)))
	// 		{
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::watchOtherTraffic);
	// 			break;
	// 		}

	// 		else
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
	// 	}
	// }

	// else if (!ActiveRequestTable.empty() && signalRequest.getBasicVehicleRole() == (static_cast<int>(MsgEnum::basicRole::ambulance)))
	// {
	// 	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
	// 	{
	// 		if (ActiveRequestTable[i].basicVehicleRole == (static_cast<int>(MsgEnum::basicRole::ambulance)))
	// 		{
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::watchOtherTraffic);
	// 			break;
	// 		}

	// 		else
	// 			priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::granted);
	// 	}
	// }

	else
		priorityRequestStatus = static_cast<int>(MsgEnum::requestStatus::unavailable); // In standard it is unknown
}

// int PriorityRequestServer::getPriorityRequestStatus()
// {
// 	return priorityRequestStatus;
// }

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
	Method for updating the updateCount from the infrastructure side.
*/
int PriorityRequestServer::getPRSUpdateCount(SignalRequest signalRequest)
{
	if (addToActiveRequesttable(signalRequest) == true || updateActiveRequestTable(signalRequest) == true)
	{
		if (updateCount < SEQUENCE_NUMBER_MAXLIMIT)
			updateCount++;

		else
			updateCount = SEQUENCE_NUMBER_MINLIMIT;
	}
	return updateCount;
}

/*
	Method for obtaining signal group based on vehicle laneID and approachID using MapEngine Library.
*/
int PriorityRequestServer::getSignalGroup(SignalRequest signalRequest)
{
    int phaseNo{};
    bool singleFrame = false;
    Json::Value jsonObject;
    Json::Reader reader;
    std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

    std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject);

    int intersectionId = getIntersectionID();
    int regionalId = getRegionalID();
    // std::string fmap = (jsonObject["IntersectionInfo"]["mapFileDirectory"]).asString();
    std::string intersectionName = (jsonObject["IntersectionName"]).asString();
    std::string fmap = "/nojournal/bin/" + intersectionName + ".map.payload";

    LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
    int approachID = plocAwareLib->getApproachIdByLaneId(regionalId, intersectionId, static_cast<uint8_t>(signalRequest.getInBoundLaneID()));
    phaseNo = unsigned(plocAwareLib->getControlPhaseByIds(regionalId, intersectionId, approachID,
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
	Method for deleting mapPayload in .map.payload file.
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


void PriorityRequestServer::getPhaseGroup(std::string jsonString)
{
	// std::vector<int> phaseGroup(8);
	Json::Value jsonObject;
    Json::Reader reader;
    // std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

    // std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
    //reader.parse(configJsonString.c_str(), jsonObject);
	reader.parse(jsonString.c_str(), jsonObject);

	for (unsigned int i=0; i<=phaseGroup.size(); i++)
	{
		if(static_cast<short unsigned int>(jsonObject["Spat"]["phaseState"][i]["phaseNo"].asInt())==i+1)
			phaseGroup.at(i) = i+1;
	} 
        
}

/*
	Method for creating Signal Status Message from the Active Request Table.
*/
std::string PriorityRequestServer::createSSMJsonString(SignalRequest signalRequest, SignalStatus signalStatus)
{
	std::string ssmJsonString{};

	signalStatus.setMinuteOfYear(getMinuteOfYear());
	signalStatus.setMsOfMinute(getMsOfMinute());
	signalStatus.setSequenceNumber(getPRSSequenceNumber());
	signalStatus.setUpdateCount(getPRSUpdateCount(signalRequest));
	signalStatus.setRegionalID(getRegionalID());
	signalStatus.setIntersectionID(getIntersectionID());
	ssmJsonString = signalStatus.signalStatus2Json(ActiveRequestTable);

	return ssmJsonString;
}

PriorityRequestServer::~PriorityRequestServer()
{
}