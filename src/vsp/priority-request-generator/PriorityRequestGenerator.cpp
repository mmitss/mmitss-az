/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  PriorrityRequestGenerator.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This is the initial revision developed for receiving bsm, map, ssm data from the transceiver. 
  2. This script use mapengine library to obtain vehicle status based on active map. 
  3. This script has an API to calculate ETA. 
  4. This script has an API to generate srm json string which is compatible to asn1 j2735 standard.
  5. The generated srm json string will send to Transceiver over a UDP socket.
  6. This script has an API to create Active Request Table on the vehicle side based on the received ssm.
*/

#include <netinet/in.h>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <cstring>
#include <iterator>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ctime>
#include <cmath>
#include <math.h>
#include "AsnJ2735Lib.h"
#include "locAware.h"
#include "geoUtils.h"
#include "PriorityRequestGenerator.h"

using namespace GeoUtils;
using namespace MsgEnum;

PriorityRequestGenerator::PriorityRequestGenerator()
{
	readConfigFile();
	setVehicleType();
	setBasicVehicleRole(vehicleType);
}

/*
	- The following method can create Active request Table in the vehicle side based on the received ssm
	- updating existing ART can be an expensive process since each time it has to be checked if any fields has been updated for each requests in the ART. If two vehicle information is removed from the ART by PRS, it may cause problem.).
*/
vector<ActiveRequest> PriorityRequestGenerator::creatingSignalRequestTable(SignalStatus signalStatus)
{
	vector<int> vehicleID{};
	vector<int> requestID{};
	vector<int> msgCount_ssm{}; //insted of msgCount, msgCount_ssm is declared otherwise it shadowed the declaration
	vector<int> inBoundLaneID{};
	vector<int> basicVehicleRole_ssm{}; //insted of basicVehicleRole, basicVehicleRole_ssm is declared, otherwise it shadowed the declaration
	vector<int> expectedTimeOfArrival_Minute{};
	vector<int> expectedTimeOfArrival_Second{};
	vector<int> expectedTimeOfArrival_Duration{};
	vector<int> priorityRequestStatus{};
	ActiveRequest activeRequest;
	activeRequest.reset();

	//creating the active request table based on the stored information
	if (addToActiveRequestTable(signalStatus) == true)
	{
		vehicleID.clear();
		requestID.clear();
		msgCount_ssm.clear();
		inBoundLaneID.clear();
		basicVehicleRole_ssm.clear();
		expectedTimeOfArrival_Minute.clear();
		expectedTimeOfArrival_Second.clear();
		expectedTimeOfArrival_Duration.clear();
		priorityRequestStatus.clear();
		ActiveRequestTable.clear();

		vehicleID = signalStatus.getTemporaryVehicleID();
		requestID = signalStatus.getRequestID();
		msgCount_ssm = signalStatus.getMsgCount();
		inBoundLaneID = signalStatus.getInBoundLaneID();
		basicVehicleRole_ssm = signalStatus.getBasicVehicleRole();
		expectedTimeOfArrival_Minute = signalStatus.getETA_Minute();
		expectedTimeOfArrival_Second = signalStatus.getETA_Second();
		expectedTimeOfArrival_Duration = signalStatus.getETA_Duration();
		priorityRequestStatus = signalStatus.getPriorityRequestStatus();

		for (int i = 0; i < signalStatus.getNoOfRequest(); i++)
		{
			activeRequest.vehicleID = vehicleID[i];
			activeRequest.requestID = requestID[i];
			activeRequest.msgCount = msgCount_ssm[i];
			activeRequest.basicVehicleRole = basicVehicleRole_ssm[i];
			activeRequest.vehicleLaneID = inBoundLaneID[i];
			activeRequest.vehicleETAMinute = expectedTimeOfArrival_Minute[i];
			activeRequest.vehicleETASecond = expectedTimeOfArrival_Second[i];
			activeRequest.vehicleETADuration = static_cast<int>(expectedTimeOfArrival_Duration[i] / SECOND_MILISECOND_CONVERSION);
			
			if (getMsOfMinute() > expectedTimeOfArrival_Second[i])
				activeRequest.vehicleETA = (getMinuteOfYear() - expectedTimeOfArrival_Minute[i]) * SECOND_MINTUTE_CONVERSION + ((getMsOfMinute() - expectedTimeOfArrival_Second[i]) / SECOND_MILISECOND_CONVERSION) + SECOND_MINTUTE_CONVERSION;

			else if (getMsOfMinute() < expectedTimeOfArrival_Second[i])
				activeRequest.vehicleETA = (getMinuteOfYear() - expectedTimeOfArrival_Minute[i]) * SECOND_MINTUTE_CONVERSION + (expectedTimeOfArrival_Second[i] - getMsOfMinute()) / SECOND_MILISECOND_CONVERSION;

			else if (getMsOfMinute() == expectedTimeOfArrival_Second[i])
				activeRequest.vehicleETA = (getMinuteOfYear() - expectedTimeOfArrival_Minute[i]) * SECOND_MINTUTE_CONVERSION;

			activeRequest.prsStatus = priorityRequestStatus[i];
			activeRequest.minuteOfYear = getMinuteOfYear();
			ActiveRequestTable.push_back(activeRequest);
		}
	}

	printActiveRequestTable();
	return ActiveRequestTable;
}

/*
	- Create srm json string based on te bsm and associated information obtained based on the active map (laneID, approachID, ETA)
*/
string PriorityRequestGenerator::createSRMJsonString(BasicVehicle basicVehicle, SignalRequest signalRequest, MapManager mapManager)
{
	string srmJsonString{};
	int vehicleExpectedTimeOfArrival_Minute{};
	int vehicleExpectedTimeOfArrival_Second{};
	int vehicleExpectedTimeOfArrival_Duration{};
	int relativeETAInMiliSecond = static_cast <int>(getETA() * SECOND_MILISECOND_CONVERSION + getMsOfMinute());
	
	// vehicleExpectedTimeOfArrival_Second = static_cast<int>(remquo((relativeETAInSecond / SECOND_MINTUTE_CONVERSION), 1.0, &vehicleExpectedTimeOfArrival_Minute) * SECOND_MINTUTE_CONVERSION * SECOND_MILISECOND_CONVERSION);
	// vehicleExpectedTimeOfArrival_Minute = getMinuteOfYear() + vehicleExpectedTimeOfArrival_Minute;
	vehicleExpectedTimeOfArrival_Minute = getMinuteOfYear() + (relativeETAInMiliSecond / static_cast<int>(SECOND_MINTUTE_CONVERSION * SECOND_MILISECOND_CONVERSION));
	vehicleExpectedTimeOfArrival_Second = relativeETAInMiliSecond % static_cast<int>(SECOND_MINTUTE_CONVERSION * SECOND_MILISECOND_CONVERSION);
	vehicleExpectedTimeOfArrival_Duration = minimumETA_Duration * SECOND_MILISECOND_CONVERSION;
	setMsgCount(msgCount);
	tempVehicleSpeed = getVehicleSpeed(); //storing vehicle speed while sending srm. It will be use to compare if there is any speed change or not
	tempVehicleSignalGroup = getSignalGroup();

	signalRequest.setMinuteOfYear(getMinuteOfYear());
	signalRequest.setMsOfMinute(getMsOfMinute());
	signalRequest.setMsgCount(getMsgCount());
	signalRequest.setRegionalID(getRegionalID());
	signalRequest.setIntersectionID(getIntersectionID());
	// signalRequest.setVehicleType(getVehicleType()); //getVehicleType() function has to be executed before the getBasicVehicleRole()
	signalRequest.setPriorityRequestType(getPriorityRequestType());
	signalRequest.setInBoundLaneIntersectionAccessPoint(getLaneID(), getApproachID());
	signalRequest.setETA(vehicleExpectedTimeOfArrival_Minute, vehicleExpectedTimeOfArrival_Second, vehicleExpectedTimeOfArrival_Duration);
	signalRequest.setTemporaryVechileID(getVehicleID());
	signalRequest.setBasicVehicleRole(getBasicVehicleRole());
	signalRequest.setPosition(basicVehicle.getLatitude_DecimalDegree(), basicVehicle.getLongitude_DecimalDegree(), basicVehicle.getElevation_Meter());
	signalRequest.setHeading_Degree(basicVehicle.getHeading_Degree());
	signalRequest.setSpeed_MeterPerSecond(basicVehicle.getSpeed_MeterPerSecond());
	srmJsonString = signalRequest.signalRequest2Json();

	clearActiveMapInformation(mapManager);
	loggingData(srmJsonString);

	return srmJsonString;
}

/*
	- Check whether vehicle should accept the ssm and populate active request table or not.
		-- If current intersectionID and regionalID of the vehicle and intersectionID and regionalID of ssm match accept the ssm 
*/
bool PriorityRequestGenerator::addToActiveRequestTable(SignalStatus signalStatus)
{
	bool matchIntersection{false};

	if (getIntersectionID() == signalStatus.getIntersectionID() && getRegionalID() == signalStatus.getRegionalID())
		matchIntersection = true;

	return matchIntersection;
}

/*
	- Method to get SRM sending requirement status 
*/
bool PriorityRequestGenerator::checkPriorityRequestSendingRequirementStatus()
{
	bool srmSendingRequirement{};

	switch (vehicleType)
	{
	case 2:
		srmSendingRequirement = checkRequestSendingRequirement(lightSirenStatus);
		break;
	case 6:
		srmSendingRequirement = checkRequestSendingRequirement(busStopList);
		break;
	case 9:
		srmSendingRequirement = checkRequestSendingRequirement();
		break;
	default:
		break;
	}

	return srmSendingRequirement;
}
/*
	- Check whether a Truck needs to send srm or not, based on the active map list, vehicle status.
*/
bool PriorityRequestGenerator::checkRequestSendingRequirement()
{
	bool requestSendingRequirement{false};
	double currentTime = getPosixTimestamp();
	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p)
																		{ return p.vehicleID == temporaryVehicleID; });

	//If vehicle is out of the map but priority request is available in the ART, it is required to send a cancellation request.
	if ((activeMapStatus == false) && (findVehicleIDOnTable != ActiveRequestTable.end()))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));

		displayConsoleData("SRM is sent since vehicle is out off the map");
	}

	//If there is an active map but vehicle is leaving the intersection, it is required to send a cancellation request.
	else if (activeMapStatus && (vehicleIntersectionStatus ==
								 (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) ||
								  static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) ||
								  static_cast<int>(MsgEnum::mapLocType::onOutbound))))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is either leaving or not in inBoundlane of the Intersection");
	}

	//If there is an active map and vehilce ID is available in the ART but vehicle is leaving the intersection, it is required to send a cancellation request.
	else if (activeMapStatus && (findVehicleIDOnTable != ActiveRequestTable.end()) &&
			 (vehicleIntersectionStatus == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) ||
											static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) ||
											static_cast<int>(MsgEnum::mapLocType::onOutbound))))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is leaving the Intersection");
	}

	else if (activeMapStatus && (vehicleIntersectionStatus == static_cast<int>(MsgEnum::mapLocType::onInbound)) &&
			 ((currentTime - srmSendingTime) >= SRM_TIME_GAP_VALUE))
	{
		//If vehicleID is not in the ART, vehicle should send SRM
		if (findVehicleIDOnTable == ActiveRequestTable.end())
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityRequest));
			displayConsoleData("SRM is sent since ART is empty at time");
		}
		//If vehicle signal group changed it is required to send SRM
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (tempVehicleSignalGroup != getSignalGroup()))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle signalGroup has been changed");
		}

		//If vehicleID is in ART and vehicle speed changes by threshold value (for example 5m/s), vehicle should send srm.
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) &&
				 (abs(vehicleSpeed - tempVehicleSpeed) >= ALLOWED_SPEED_DEVIATION))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle speed has been changed");
		}

		//If vehicleID is in ART and vehicle ETA changes by threshold value, vehicle should send srm
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) &&
				 (abs(findVehicleIDOnTable->vehicleETA - vehicleETA) >= ALLOWED_ETA_DIFFERENCE))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle ETA has been changed from " + std::to_string(findVehicleIDOnTable->vehicleETA) + " to " + std::to_string(vehicleETA));
		}

		//If vehicleID is in the ART and the message count of the last sent out srm and message count in the ART doesn't match, vehicle should send srm
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (findVehicleIDOnTable->msgCount != msgCount))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since msgCount doesn't match");
		}

		//Vehicle needs to send SRM to avoid request Timedout scenario in the PRS
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) &&
				 ((currentTime - srmSendingTime) >= requestTimedOutValue))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent to avoid PRS timed out");
		}
	}

	if (requestSendingRequirement)
		srmSendingTime = currentTime;

	return requestSendingRequirement;
}

/*
	- Check whether a Transit needs to send srm or not, based on the active map list, vehicle status and bus stop information.
*/
bool PriorityRequestGenerator::checkRequestSendingRequirement(vector<BusStopInformation> bus_Stop_List)
{
	bool requestSendingRequirement{false};
	double currentTime = getPosixTimestamp();

	if (!bus_Stop_List.empty())
		busStopList = bus_Stop_List;

	checkPassedNearestBusStop();

	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p)
																		{ return p.vehicleID == temporaryVehicleID; });

	//If vehicle is out of the map but priority request is available in the ART, it is required to send a cancellation request.
	if (!activeMapStatus && (findVehicleIDOnTable != ActiveRequestTable.end()))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is out off the map");
	}

	//If there is an active map and vehicle is leaving the intersection, it is required to send a cancellation request.
	else if (activeMapStatus && (vehicleIntersectionStatus ==
								 (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) ||
								  static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) ||
								  static_cast<int>(MsgEnum::mapLocType::onOutbound))))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is either leaving or not in inBoundlane of the Intersection");
	}

	//If there is an active map and vehilce ID is available in the ART but vehicle is leaving the intersection, it is required to send a cancellation request.
	else if (activeMapStatus && (findVehicleIDOnTable != ActiveRequestTable.end()) &&
			 (vehicleIntersectionStatus == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) ||
											static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) ||
											static_cast<int>(MsgEnum::mapLocType::onOutbound))))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is leaving the Intersection");
	}

	else if (activeMapStatus && (busStopPassedStatus == true) &&
			 (vehicleIntersectionStatus == static_cast<int>(MsgEnum::mapLocType::onInbound)) &&
			 ((currentTime - srmSendingTime) >= SRM_TIME_GAP_VALUE))
	{
		//If vehicleID is not in the ART, vehicle should send SRM
		if (findVehicleIDOnTable == ActiveRequestTable.end())
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityRequest));
			displayConsoleData("SRM is sent since ART is empty at time");
		}
		//If vehicle's signal group is changed it is required to send SRM
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (tempVehicleSignalGroup != getSignalGroup()))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle signalGroup has been changed");
		}

		//If vehicleID is in ART and vehicle speed changes by threshold value (for example 5m/s), vehicle should send srm.
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (abs(vehicleSpeed - tempVehicleSpeed) >= ALLOWED_SPEED_DEVIATION))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle speed has been changed");
		}

		//If vehicleID is in the ART and its ETA is changed by threshold value, vehicle should send srm
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (abs(findVehicleIDOnTable->vehicleETA - vehicleETA) >= ALLOWED_ETA_DIFFERENCE))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle ETA has been changed from " + std::to_string(findVehicleIDOnTable->vehicleETA) + " to " + std::to_string(vehicleETA));
		}

		//If vehicleID is in the ART and the message count of the last sent out srm and message count in the ART doesn't match, vehicle should send srm
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (findVehicleIDOnTable->msgCount != msgCount))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since msgCount doesn't match");
		}

		//Vehicle needs to send SRM to avoid request Timedout scenario in the PRS
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && ((currentTime - srmSendingTime) >= requestTimedOutValue))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent to avoid PRS timed out");
		}
	}

	//If vehicle doesn't pass the bus stop but the priority request is available in the ART, it is required to send a cancellation request.
	else if (!busStopPassedStatus && (findVehicleIDOnTable != ActiveRequestTable.end()))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is out off the map");
	}

	if (requestSendingRequirement)
		srmSendingTime = currentTime;

	return requestSendingRequirement;
}

/*
	- Check whether an Emergency Vehicle needs to send srm or not based on the active map list, vehicle status, and light-siren status.
*/
bool PriorityRequestGenerator::checkRequestSendingRequirement(bool light_Siren_Status)
{
	bool requestSendingRequirement{false};
	double currentTime = getPosixTimestamp();
	lightSirenStatus = light_Siren_Status;

	vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																		[&](ActiveRequest const &p)
																		{ return p.vehicleID == temporaryVehicleID; });

	//If vehicle is out of the map but priority request is available in the ART, it is required to send a cancellation request.
	if (!activeMapStatus && (findVehicleIDOnTable != ActiveRequestTable.end()))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is out off the map");
	}

	//If there is an active map and vehicle is leaving the intersection, it is required to send a cancellation request.
	else if (activeMapStatus && (vehicleIntersectionStatus ==
								 (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) ||
								  static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) ||
								  static_cast<int>(MsgEnum::mapLocType::onOutbound))))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is either leaving or not in inBoundlane of the Intersection");
	}

	//If there is an active map and vehilce ID is available in the ART but vehicle is leaving the intersection, it is required to send a cancellation request.
	else if (activeMapStatus && (findVehicleIDOnTable != ActiveRequestTable.end()) &&
			 (vehicleIntersectionStatus == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) ||
											static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) ||
											static_cast<int>(MsgEnum::mapLocType::onOutbound))))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since vehicle is leaving the Intersection");
	}

	else if (activeMapStatus && (lightSirenStatus == true) &&
			 (vehicleIntersectionStatus == static_cast<int>(MsgEnum::mapLocType::onInbound)) &&
			 ((currentTime - srmSendingTime) >= SRM_TIME_GAP_VALUE))
	{
		//If vehicleID is not in the ART, vehicle should send SRM
		if (findVehicleIDOnTable == ActiveRequestTable.end())
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityRequest));
			displayConsoleData("SRM is sent since ART is empty at time");
		}
		//If vehicle signal group changed it is required to send SRM
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (tempVehicleSignalGroup != getSignalGroup()))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle signalGroup has been changed");
		}

		//If vehicleID is in ART and vehicle speed changes by threshold value (for example 5m/s), vehicle should send srm.
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (abs(vehicleSpeed - tempVehicleSpeed) >= ALLOWED_SPEED_DEVIATION))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle speed has been changed");
		}

		//If vehicleID is in the ART and its ETA is changed by threshold value, vehicle should send srm
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (abs(findVehicleIDOnTable->vehicleETA - vehicleETA) >= ALLOWED_ETA_DIFFERENCE))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since vehicle ETA has been changed from " + std::to_string(findVehicleIDOnTable->vehicleETA) + " to " + std::to_string(vehicleETA));
		}

		//If vehicleID is in the ART and the message count of the last sent out srm and message count in the ART doesn't match, vehicle should send srm
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && (findVehicleIDOnTable->msgCount != msgCount))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent since msgCount doesn't match");
		}

		//Vehicle needs to send SRM to avoid request Timedout scenario in the PRS
		else if ((findVehicleIDOnTable != ActiveRequestTable.end()) && ((currentTime - srmSendingTime) >= requestTimedOutValue))
		{
			requestSendingRequirement = true;
			requestSendStatus = true;
			setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
			displayConsoleData("SRM is sent to avoid PRS timed out");
		}
	}

	//If vehicle ID is available in the ART but the light-siren is turned off, it is required to send cancellation request
	else if ((lightSirenStatus == false) && (findVehicleIDOnTable != ActiveRequestTable.end()))
	{
		requestSendingRequirement = true;
		requestSendStatus = false;
		setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		displayConsoleData("SRM is sent since light-siren is off");
	}

	if (requestSendingRequirement)
		srmSendingTime = currentTime;

	return requestSendingRequirement;
}

/*
    - Following method is reponsible for obtaining the bu stop location from the busStopList
    - If the approchID and active map's intersection ID of the vehicle matches with the approchID and active map's intersection ID in the List, the method will store the location of the bus stop and return true.
*/
bool PriorityRequestGenerator::findNearestBusStopLocation()
{
	bool nearestBusStopStatus{false};

	for (size_t i = 0; i < busStopList.size(); i++)
	{
		if ((intersectionID == busStopList[i].intersectionID) && (vehicleAprroachID == busStopList[i].approachNo))
		{
			nearestBusStopStatus = true;
			busStopLattitude = busStopList[i].lattitude_DecimalDegree;
			busStopLongitude = busStopList[i].longitude_DecimalDegree;
			busStopElevation = busStopList[i].elevation_Meter;
			busStopHeading = busStopList[i].heading_Degree;
			break;
		}
	}

	return nearestBusStopStatus;
}

/*
    - If there is a bus stop along the travel path of a intersection, the following method will check whether the vehicle has passed the bus stop or not.
	- The method will compute bus stop distance from the stop bar using the map engine library. The method will compare the vehicle distance from the stop bar and bus stop distance from the stop bar to take decision
	- If there is no bus stop along the travel path of a intersection, the method will always return true.
*/
bool PriorityRequestGenerator::checkPassedNearestBusStop()
{
	double busStopDistanceFromStopBar{};
	string fmap{};
	string intersectionName{};
	bool singleFrame{false}; /// TRUE to encode speed limit in lane, FALSE to encode in approach

	if (findNearestBusStopLocation() == true)
	{
		fmap = activeMapList.front().activeMapFileDirectory;
		intersectionName = activeMapList.front().activeMapFileName;

		//initialize mapengine library
		LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
		//initialize all the struct require to locate vehicle in Map.
		struct geoPoint_t geoPoint_t_1 = {busStopLattitude, busStopLongitude, busStopElevation};
		struct motion_t motion_t_1 = {1.0, busStopHeading};
		struct intersectionTracking_t intersectionTracking_t_1 = {mapLocType::onInbound, 0, 0, 0};
		struct point2D_t point2D_t_1 = {0, 0};
		struct point2D_t point2D_t_2 = {0, 0};
		struct projection_t projection_t_1 = {0.0, 0.0, 0.0};
		struct laneProjection_t laneProjection_t_1 = {0, projection_t_1};
		struct vehicleTracking_t vehicleTracking_t_1 = {intersectionTracking_t_1, laneProjection_t_1};
		std::bitset<4> maneuvers;
		struct dist2go_t dist2go_t_1 = {0.0, 0.0};
		struct connectTo_t connectTo_t_1 = {0, 0, 0, maneuverType::straightAhead};
		vector<connectTo_t> connect2go1;
		connect2go1.push_back(connectTo_t_1);
		struct locationAware_t locationAware_t_1 = {0, 0, 0, 0, 0.0, maneuvers, dist2go_t_1, connect2go1};
		struct signalAware_t signalAware_t_1 = {phaseColor::dark, phaseState::redLight, unknown_timeDetail, unknown_timeDetail, unknown_timeDetail};
		struct connectedVehicle_t connectedVehicle_t_1 = {0, 0, 0, geoPoint_t_1, motion_t_1, vehicleTracking_t_1, locationAware_t_1, signalAware_t_1};

		plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1);
		plocAwareLib->getPtDist2D(vehicleTracking_t_1, point2D_t_2);
		busStopDistanceFromStopBar = unsigned(point2D_t_1.distance2pt(point2D_t_2)); //unit of centimeters
		delete plocAwareLib;

		if (busStopDistanceFromStopBar / DISTANCEUNITCONVERSION > vehicleDistanceFromStopBar / DISTANCEUNITCONVERSION)
			busStopPassedStatus = true;

		else
			busStopPassedStatus = false;
	}

	else
		busStopPassedStatus = true;

	return busStopPassedStatus;
}

void PriorityRequestGenerator::setIntersectionID(int vehicleNearByIntersectionId)
{
	intersectionID = vehicleNearByIntersectionId;
}

void PriorityRequestGenerator::setRegionalID(int vehicleNearByRegionalId)
{
	regionalID = vehicleNearByRegionalId;
}

void PriorityRequestGenerator::setVehicleID(BasicVehicle basicVehicle)
{
	temporaryVehicleID = basicVehicle.getTemporaryID();
}

void PriorityRequestGenerator::setVehicleSpeed(BasicVehicle basicVehicle)
{
	vehicleSpeed = basicVehicle.getSpeed_MeterPerSecond();
}

void PriorityRequestGenerator::setLaneID(int laneId)
{
	vehicleLaneID = laneId;
}

void PriorityRequestGenerator::setApproachID(int approachID)
{
	vehicleAprroachID = approachID;
}

void PriorityRequestGenerator::setSignalGroup(int phaseNo)
{
	signalGroup = phaseNo;
}

/*
	-calculation for ETA. Units will be second
*/
void PriorityRequestGenerator::setETA(double distance2go, double vehicle_Speed)
{
	if (vehicle_Speed >= minimumVehicleSpeed)
		vehicleETA = static_cast<double>((distance2go / DISTANCEUNITCONVERSION) / vehicle_Speed); //distance2go is cm. DISTANCEUNITCONVERSION is used converst distance2go into meter

	else
		vehicleETA = minimumETA + vehicleStartUpLossTime;

	if (vehicleETA <= 0)
		vehicleETA = minimumETA;
}

/*
	-obtain vehicle location in the map-  whether it is in inBound or in intersectionBox or in outBound
*/
void PriorityRequestGenerator::setVehicleIntersectionStatus(int vehIntersectionStatus)
{
	vehicleIntersectionStatus = vehIntersectionStatus;
}

/*
	-Get the message type based on the received json string from Transceiver
*/
int PriorityRequestGenerator::getMessageType(string jsonString)
{
	int messageType{};
	Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};

	bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;

	if (parsingSuccessful == true)
	{
		if ((jsonObject["MsgType"]).asString() == "MAP")
			messageType = MsgEnum::DSRCmsgID_map;

		else if ((jsonObject["MsgType"]).asString() == "BSM")
			messageType = MsgEnum::DSRCmsgID_bsm;

		else if ((jsonObject["MsgType"]).asString() == "SSM")
		{
			messageType = MsgEnum::DSRCmsgID_ssm;
			displayConsoleData("Received SSM");
			loggingData(jsonString);
		}

		else if ((jsonObject["MsgType"]).asString() == "LightSirenStatusMessage")
			messageType = static_cast<int>(msgType::lightSirenStatus);
	}

	return messageType;
}

vector<Map::ActiveMap> PriorityRequestGenerator::getActiveMapList(MapManager mapManager)
{
	activeMapList = mapManager.getActiveMapList();

	return activeMapList;
}

/*
	-If there is active map, based on the bsm data this function will locate vehicle on the map and obtain inBoundLaneID, inBoundApproachID, distance from the stop-bar and time requires to reach the stop-bar
*/
void PriorityRequestGenerator::getVehicleInformationFromMAP(MapManager mapManager, BasicVehicle basicVehicle)
{

	string fmap{};
	string intersectionName{};
	bool singleFrame{false}; /// TRUE to encode speed limit in lane, FALSE to encode in approach

	// displayConsoleData("Received BSM");
	//If active map list is empty, look for active map
	if (activeMapList.empty())
	{
		mapManager.createActiveMapList(basicVehicle);
		getActiveMapList(mapManager);
	}

	//If active map List is not empty, locate vehicle on the map and obtain inBoundLaneID, inBoundApproachID, distance from the stop-bar and time requires to reach the stop-bar
	if (!activeMapList.empty())
	{
		activeMapStatus = true; //This variables will be used by while checking if vehicle needs to send srm or not. If there is active map the value of this variable will true.
		fmap = activeMapList.front().activeMapFileDirectory;
		intersectionName = activeMapList.front().activeMapFileName;

		//initialize mapengine library
		LocAware *plocAwareLib = new LocAware(fmap, singleFrame);

		uint32_t referenceId = plocAwareLib->getIntersectionIdByName(intersectionName);
		uint16_t regionalId = static_cast<uint16_t>((referenceId >> 16) & 0xFFFF);
		uint16_t intersectionId = static_cast<uint16_t>(referenceId & 0xFFFF);

		//get the vehicle data from bsm
		double vehicle_Latitude = basicVehicle.getLatitude_DecimalDegree();
		double vehicle_Longitude = basicVehicle.getLongitude_DecimalDegree();
		double vehicle_Elevation = basicVehicle.getElevation_Meter();
		setVehicleSpeed(basicVehicle);
		double vehicle_Heading = basicVehicle.getHeading_Degree();
		//initialize all the struct require to locate vehicle in Map.
		struct geoPoint_t geoPoint_t_1 = {vehicle_Latitude, vehicle_Longitude, vehicle_Elevation};
		struct motion_t motion_t_1 = {vehicleSpeed, vehicle_Heading};
		struct intersectionTracking_t intersectionTracking_t_1 = {mapLocType::onInbound, 0, 0, 0};
		struct point2D_t point2D_t_1 = {0, 0};
		struct point2D_t point2D_t_2 = {0, 0};
		struct projection_t projection_t_1 = {0.0, 0.0, 0.0};
		struct laneProjection_t laneProjection_t_1 = {0, projection_t_1};
		struct vehicleTracking_t vehicleTracking_t_1 = {intersectionTracking_t_1, laneProjection_t_1};
		std::bitset<4> maneuvers;
		struct dist2go_t dist2go_t_1 = {0.0, 0.0};
		struct connectTo_t connectTo_t_1 = {0, 0, 0, maneuverType::straightAhead};
		vector<connectTo_t> connect2go1;
		connect2go1.push_back(connectTo_t_1);
		struct locationAware_t locationAware_t_1 = {0, 0, 0, 0, 0.0, maneuvers, dist2go_t_1, connect2go1};
		struct signalAware_t signalAware_t_1 = {phaseColor::dark, phaseState::redLight, unknown_timeDetail, unknown_timeDetail, unknown_timeDetail};
		struct connectedVehicle_t connectedVehicle_t_1 = {0, 0, 0, geoPoint_t_1, motion_t_1, vehicleTracking_t_1, locationAware_t_1, signalAware_t_1};

		//counter_VehicleInMap will ensure after being inside the map vehicle doesn't go out of inBoundLane(stopped in the parking lot)
		if (counter_VehicleInMap > 10)
		{
			//If vehicle is on Map, update all the information
			if (plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1) == true && unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::onInbound))
			{
				setVehicleIntersectionStatus(unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus));
				setIntersectionID(intersectionId);
				setRegionalID(regionalId);
				setLaneID(plocAwareLib->getLaneIdByIndexes(unsigned(vehicleTracking_t_1.intsectionTrackingState.intersectionIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.approachIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.laneIndex)));
				setApproachID(plocAwareLib->getApproachIdByLaneId(regionalId, intersectionId, (unsigned char)((unsigned)getLaneID())));
				setSignalGroup(plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalID), static_cast<uint16_t>(intersectionID), static_cast<uint8_t>(vehicleAprroachID), static_cast<uint8_t>(vehicleLaneID))); //Method for obtaining signal group based on vehicle laneID and approachID using MapEngine Library.
				plocAwareLib->getPtDist2D(vehicleTracking_t_1, point2D_t_2);
				vehicleDistanceFromStopBar = unsigned(point2D_t_1.distance2pt(point2D_t_2)); //unit of centimeters
				setETA(vehicleDistanceFromStopBar, vehicleSpeed);
				setVehicleID(basicVehicle); //Vehicle change its ID on a regular basis. Need to check the vehicle id.
			}
			//If vehicle is not on Map, clear the active map related information
			else
			{
				mapManager.deleteActiveMapfromList();
				activeMapList.clear();
				ActiveRequestTable.clear();
				setIntersectionID(0);
				setSignalGroup(0);
				activeMapStatus = false;
				requestSendStatus = false;
			}
			counter_VehicleInMap = 0;
		}
		//If vehicle is on Map, update all the information
		else
		{
			plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1);
			setVehicleIntersectionStatus(unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus));
			setIntersectionID(intersectionId);
			setRegionalID(regionalId);
			setLaneID(plocAwareLib->getLaneIdByIndexes(unsigned(vehicleTracking_t_1.intsectionTrackingState.intersectionIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.approachIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.laneIndex)));
			setApproachID(plocAwareLib->getApproachIdByLaneId(regionalId, intersectionId, (unsigned char)((unsigned)getLaneID())));
			setSignalGroup(plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalID), static_cast<uint16_t>(intersectionID), static_cast<uint8_t>(vehicleAprroachID), static_cast<uint8_t>(vehicleLaneID))); //Method for obtaining signal group based on vehicle laneID and approachID using MapEngine Library.
			plocAwareLib->getPtDist2D(vehicleTracking_t_1, point2D_t_2);
			vehicleDistanceFromStopBar = unsigned(point2D_t_1.distance2pt(point2D_t_2)); //unit of centimeters
			setETA(vehicleDistanceFromStopBar, vehicleSpeed);
			setVehicleID(basicVehicle); //Vehicle change its ID on a regular basis. Need to check the vehicle id.
			counter_VehicleInMap++;
		}

		delete plocAwareLib;
	}
	else
		activeMapStatus = false;
}

int PriorityRequestGenerator::getIntersectionID()
{
	return intersectionID;
}

int PriorityRequestGenerator::getRegionalID()
{
	return regionalID;
}

int PriorityRequestGenerator::getVehicleID()
{
	return temporaryVehicleID;
}

double PriorityRequestGenerator::getVehicleSpeed()
{
	return vehicleSpeed;
}

int PriorityRequestGenerator::getLaneID()
{
	return vehicleLaneID;
}

int PriorityRequestGenerator::getApproachID()
{
	return vehicleAprroachID;
}

int PriorityRequestGenerator::getSignalGroup()
{
	return signalGroup;
}

double PriorityRequestGenerator::getVehicleDistanceFromStopBar()
{
	return vehicleDistanceFromStopBar;
}

double PriorityRequestGenerator::getETA()
{
	return vehicleETA;
}

int PriorityRequestGenerator::getVehicleIntersectionStatus()
{
	return vehicleIntersectionStatus;
}

/*
	- set vehicle type from VehicleConfiguration.json file 
	- According J2735 2016 standard Bus/Transit is 6, Truck is 9, and EmergencyVehicle is 2 
*/
void PriorityRequestGenerator::setVehicleType()
{
	Json::Value jsonObject;
	std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
	string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};

	bool parsingSuccessful = reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
	delete reader;

	if (parsingSuccessful == true)
	{
		if (jsonObject["VehicleType"].asString() == "Transit")
		{
			vehicleType = static_cast<int>(MsgEnum::vehicleType::bus);
			getBusStopInformation();
		}

		else if (jsonObject["VehicleType"].asString() == "Truck")
			vehicleType = static_cast<int>(MsgEnum::vehicleType::axleCnt4);

		else if (jsonObject["VehicleType"].asString() == "EmergencyVehicle")
		{
			vehicleType = static_cast<int>(MsgEnum::vehicleType::special);
			lightSirenStatus = false;
		}
	}
}

void PriorityRequestGenerator::setSimulationVehicleType(string vehType)
{
	lightSirenStatus = true;

	if (vehType == "Transit")
	{
		vehicleType = static_cast<int>(MsgEnum::vehicleType::bus);
		getBusStopInformation();
	}

	else if (vehType == "Truck")
		vehicleType = static_cast<int>(MsgEnum::vehicleType::axleCnt4);

	else if (vehType == "EmergencyVehicle")
	{
		vehicleType = static_cast<int>(MsgEnum::vehicleType::special);
		lightSirenStatus = true;
	}
	else
		vehicleType = static_cast<int>(MsgEnum::vehicleType::car);
}

/*
	- Define vehile role based on vehicle type
*/
void PriorityRequestGenerator::setBasicVehicleRole(int vehicle_Type)
{
	if (vehicle_Type == static_cast<int>(MsgEnum::vehicleType::bus))
		basicVehicleRole = static_cast<int>(MsgEnum::basicRole::transit);

	else if (vehicle_Type == static_cast<int>(MsgEnum::vehicleType::axleCnt4))
		basicVehicleRole = static_cast<int>(MsgEnum::basicRole::truck);

	else if (vehicle_Type == static_cast<int>(MsgEnum::vehicleType::special))
		basicVehicleRole = static_cast<int>(MsgEnum::basicRole::fire);

	else if (vehicle_Type == static_cast<int>(vehicleType::unavailable))
		basicVehicleRole = static_cast<int>(MsgEnum::basicRole::unavailable);
}
/*
	- Define priority status based on vehicle status
		--If vehicle is at inBoundLane, the priority request type will be priorityRequest
		--If vehicle is at insideIntersectionBox or atIntersectionbox or outboundlane  or out of the map, the priority request type will be priorityCancellation
		--If vehicle changes its signal group or speed to a threshold value (for example- 5m/s) or ETA to a threshold value or msg count doesn't match in the ART, the priority request type will be requestUpdate
*/
void PriorityRequestGenerator::setPriorityRequestType(int priority_Request_Type)
{
	priorityRequestType = priority_Request_Type;
}

void PriorityRequestGenerator::setMsgCount(int msg_count)
{
	if (msg_count < maxMsgCount)
		msgCount++;

	else
		msgCount = minMsgCount;
}

/*
	- getters for vehicle type
*/
int PriorityRequestGenerator::getVehicleType()
{
	return vehicleType;
}

/*
	- getter for vehile role 
*/
int PriorityRequestGenerator::getBasicVehicleRole()
{
	return basicVehicleRole;
}

int PriorityRequestGenerator::getPriorityRequestType()
{
	return priorityRequestType;
}

int PriorityRequestGenerator::getActiveMapStatus()
{
	return activeMapStatus;
}

double PriorityRequestGenerator::getRequestTimedOutValue()
{
	return requestTimedOutValue;
}

/*
	- Obtain system time and calculte current Minute of the Year
		--get the number of years passed (today's number of day in the year -1)
		--get number of hours and minutes elapsed today
*/
int PriorityRequestGenerator::getMinuteOfYear()
{
	int minuteOfYear{};
	time_t t = time(NULL);
	tm *timePtr = gmtime(&t);

	int dayOfYear = timePtr->tm_yday;
	int currentHour = timePtr->tm_hour;
	int currentMinute = timePtr->tm_min;

	minuteOfYear = (dayOfYear - 1) * HOUR_DAY_CONVERSION * MINTUTE_HOUR_CONVERSION + currentHour * MINTUTE_HOUR_CONVERSION + currentMinute;

	return minuteOfYear;
}

/*
	- Obtain system time for time stamp
*/
int PriorityRequestGenerator::getMsOfMinute()
{
	int msOfMinute{};
	time_t t = time(NULL);
	tm *timePtr = gmtime(&t);

	int currentSecond = timePtr->tm_sec;

	msOfMinute = currentSecond * static_cast<int>(SECOND_MILISECOND_CONVERSION);

	return msOfMinute;
}

/*
	- Get the message count information for srm
*/
int PriorityRequestGenerator::getMsgCount()
{
	return msgCount;
}

/*
	- Get vehicle status on current map (Whether on map or not) for HMI Controller
*/
string PriorityRequestGenerator::getVehicleMapStatus()
{
	string vehicleMapStatus{"False"};

	if (activeMapStatus == true)
		vehicleMapStatus = "True";

	else if (activeMapStatus == false)
		vehicleMapStatus = "False";

	return vehicleMapStatus;
}

/*
	- Get information whether SRM is sent or not for HMI Controller
*/
string PriorityRequestGenerator::getVehicleRequestSentStatus()
{
	string vehicleSRMStatus{"False"};

	if (requestSendStatus == true)
		vehicleSRMStatus = "True";
	else if (requestSendStatus == false)
		vehicleSRMStatus = "False";

	return vehicleSRMStatus;
}

/*
	- Getters for ART table
*/
vector<ActiveRequest> PriorityRequestGenerator::getActiveRequestTable()
{
	return ActiveRequestTable;
}

/*
	-Methods for updating map status for HMI
	-If vehicle in on Map then for the active map, activeMapStatus will be true for the active map
	-If vehicle is leaving the map (either leaving the intersection or going to parking lot) then activeMapStatus will be false for all available map
*/
vector<Map::AvailableMap> PriorityRequestGenerator::manageMapStatusInAvailableMapList(MapManager mapManager)
{
	mapManager.updateMapAge();
	mapManager.deleteMap();
	if (!activeMapList.empty())
	{
		vector<Map::AvailableMap>::iterator findActiveMap = std::find_if(std::begin(mapManager.availableMapList), std::end(mapManager.availableMapList),
																		 [&](Map::AvailableMap const &p)
																		 { return p.availableMapFileName == activeMapList.front().activeMapFileName; });

		if (findActiveMap != availableMapList.end())
			findActiveMap->activeMapStatus = "True";

		availableMapList = mapManager.availableMapList;
	}

	else
	{
		for (size_t i = 0; i < availableMapList.size(); i++)
			availableMapList[i].activeMapStatus = "False";

		availableMapList = mapManager.availableMapList;
	}

	return availableMapList;
}

/*
	- This function is for printing Active request table. Here only few attributes are printed
*/
void PriorityRequestGenerator::printActiveRequestTable()
{
	double timeStamp = getPosixTimestamp();

	cout << "[" << fixed << showpoint << setprecision(2) << timeStamp << "] Active Request Table is following: " << endl;
	cout << "VehicleID"
		 << "	"
		 << "ETA"
		 << "	"
		 << "ETADuration"
		 << "	"
		 << "Vehicle Role" << endl;

	for (size_t i = 0; i < ActiveRequestTable.size(); i++)
		cout << ActiveRequestTable[i].vehicleID << "	 " << ActiveRequestTable[i].vehicleETA << " 		" << ActiveRequestTable[i].vehicleETADuration << "		" << ActiveRequestTable[i].basicVehicleRole << endl;
}

/*
	- Method for logging data in a file
*/
void PriorityRequestGenerator::loggingData(string logString)
{
	ofstream logFile;
	double timeStamp = getPosixTimestamp();

	if (logging)
	{
		logFile.open(logFileName, std::ios_base::app);
		logFile << "\n[" << fixed << showpoint << setprecision(4) << timeStamp << "] ";
		logFile << logString << endl;
		logFile.close();
	}
}

/*
	- Method for displaying console output
*/
void PriorityRequestGenerator::displayConsoleData(string consoleString)
{
	ofstream logFile;
	double timestamp = getPosixTimestamp();

	if (consoleOutput)
	{
		logFile.open(logFileName, std::ios_base::app);
		cout << "\n[" << fixed << showpoint << setprecision(4) << timestamp << "] ";
		cout << consoleString << endl;
		logFile.close();
	}
}

/*
	- Check the logging requirement from the confid file.
	- If it is required to log the data, the following method will the open log file
*/
void PriorityRequestGenerator::readConfigFile()
{
	ofstream logFile;
	double timeStamp = getPosixTimestamp();
	Json::Value jsonObject;
	ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");
	string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};

	bool parsingSuccessful = reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
	delete reader;

	if (parsingSuccessful)
	{
		requestTimedOutValue = (jsonObject["SRMTimedOutTime"]).asDouble() - SRM_TIME_GAP_VALUE;
		logging = jsonObject["Logging"].asBool();
		consoleOutput = jsonObject["ConsoleOutput"].asBool();
	}

	//Create Log File, if requires
	time_t now = time(0);
	struct tm tstruct;
	char logFileOpenningTime[80];
	tstruct = *localtime(&now);
	strftime(logFileOpenningTime, sizeof(logFileOpenningTime), "%m%d%Y_%H%M%S", &tstruct);

	logFileName = string("/nojournal/bin/log/vehicle_prgLog_") + logFileOpenningTime + ".log";

	if (logging)
	{
		logFile.open(logFileName);
		logFile << "[" << fixed << showpoint << setprecision(4) << timeStamp << "] Open PRG logfile" << endl;
		logFile.close();
	}
}

/*
	- Method to set the light-siren status based on the received message
	- If the vehicle type is emergencyVehicle(2), the following method will not change the lightSirenStatus value.
*/
void PriorityRequestGenerator::setLightSirenStatus(string jsonString)
{
	Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};

	bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;

	if (parsingSuccessful)
	{

		if ((jsonObject["LightSirenStatus"]).asString() == "ON" && vehicleType == static_cast<int>(MsgEnum::vehicleType::special))
			lightSirenStatus = true;

		else if ((jsonObject["LightSirenStatus"]).asString() == "OFF" && vehicleType == static_cast<int>(MsgEnum::vehicleType::special))
			lightSirenStatus = false;
	}
}

void PriorityRequestGenerator::getBusStopInformation()
{
	Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-bus-stop-location.json");
	string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
	int noOfBusStop = jsonObject["NoOfBusStop"].asInt();
	BusStopInformation busStopInformation;
	busStopList.clear();
	const Json::Value values = jsonObject["BusStopInformation"];
	for (int i = 0; i < noOfBusStop; i++)
	{
		busStopInformation.reset();

		for (size_t j = 0; j < values[i].getMemberNames().size(); j++)
		{
			if (values[i].getMemberNames()[j] == "IntersectionName")
				busStopInformation.intersectionName = values[i][values[i].getMemberNames()[j]].asString();

			else if (values[i].getMemberNames()[j] == "IntersectionID")
				busStopInformation.intersectionID = values[i][values[i].getMemberNames()[j]].asInt();

			else if (values[i].getMemberNames()[j] == "TravelDirection")
				busStopInformation.travelDirection = values[i][values[i].getMemberNames()[j]].asString();

			else if (values[i].getMemberNames()[j] == "ApproachNo")
				busStopInformation.approachNo = values[i][values[i].getMemberNames()[j]].asInt();

			else if (values[i].getMemberNames()[j] == "Latitude_DecimalDegree")
				busStopInformation.lattitude_DecimalDegree = values[i][values[i].getMemberNames()[j]].asDouble();

			else if (values[i].getMemberNames()[j] == "Longitude_DecimalDegree")
				busStopInformation.longitude_DecimalDegree = values[i][values[i].getMemberNames()[j]].asDouble();

			else if (values[i].getMemberNames()[j] == "Elevation_Meter")
				busStopInformation.elevation_Meter = values[i][values[i].getMemberNames()[j]].asDouble();

			else if (values[i].getMemberNames()[j] == "Heading_Degree")
				busStopInformation.heading_Degree = values[i][values[i].getMemberNames()[j]].asDouble();
		}
		busStopList.push_back(busStopInformation);
	}
	delete reader;
}

void PriorityRequestGenerator::clearActiveMapInformation(MapManager mapManager)
{
	if (getPriorityRequestType() == static_cast<int>(MsgEnum::requestType::priorityCancellation))
	{
		mapManager.deleteActiveMapfromList();
		activeMapList.clear();
		ActiveRequestTable.clear();
		setIntersectionID(0);
		activeMapStatus = false; //Required for HMI json
		requestSendStatus = false;
		busStopPassedStatus = false;
	}
}

PriorityRequestGenerator::~PriorityRequestGenerator()
{
}