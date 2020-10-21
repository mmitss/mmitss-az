/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  SrmManager..cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1.
*/
#include <algorithm>
#include <ctime>
#include "msgEnum.h"
#include "SrmManager.h"

SrmManager::SrmManager()
{

}
void SrmManager::setParameters(PriorityRequestGenerator priorityRequestGenerator, vector<Map::ActiveMap> active_Map_List, vector<ActiveRequest>ART, BasicVehicle basicVehicle)
{
    if(!active_Map_List.empty())
        activeMapList = active_Map_List;

    if (!ART.empty())
        ActiveRequestTable = ART;
    
    // if(!bus_Stop_List.empty())
    //     busStopList = bus_Stop_List; 

    intersectionID = priorityRequestGenerator.getIntersectionID();
    vehicleID = priorityRequestGenerator.getVehicleID();
    vehicleType = priorityRequestGenerator.getVehicleType();
    vehicleSpeed = basicVehicle.getSpeed_MeterPerSecond();
    vehicleETA = priorityRequestGenerator.getTime2Go();
    vehicleIntersectionStatus = priorityRequestGenerator.getVehicleIntersectionStatus();
    signalGroup = priorityRequestGenerator.getSignalGroup();
    approachNo = priorityRequestGenerator.getApproachID();
    msgCount = priorityRequestGenerator.getMsgCount();
    srmTimeGapValue = SRM_GAPOUT_TIME;
    requestTimedOutValue = priorityRequestGenerator.getRequestTimedOutValue();
    activeMapStatus = priorityRequestGenerator.getActiveMapStatus();
}

/*
	- Define priority status based on vehicle status
		--If vehicle is at inBoundLane, the priority request type will be priorityRequest
		--If vehicle is at insideIntersectionBox or atIntersectionbox or outboundlane  or out of the map, the priority request type will be priorityCancellation
		--If vehicle changes its signal group or speed to a threshold value (for example- 5m/s) or ETA to a threshold value or msg count doesn't match in the ART, the priority request type will be requestUpdate
*/
void SrmManager::setPriorityRequestType(int priority_Request_Type)
{
    priorityRequestType = priority_Request_Type;
}

/*
	- Set the message count information for srm
*/
void SrmManager::setMessageCount(int msg_count)
{
    if (msg_Count < maxMsgCount)
		msgCount++;

	else
		msgCount = minMsgCount;

	return msgCount;
}

int SrmManager::getMessageCount()
{
    return msgCount;
}
/*
    - Following method is reponsible for obtaining the bu stop location from the busStopList
    - If the approch No and active map's intersection ID of the vehicle matches with the approch No and active map's intersection ID in the List, the method will store the location of the bus stop and return true.
*/
bool SrmManager::findNearestBusStopLocation()
{
    bool nearestBusStopStatus{false};

    for (size_t i = 0; i < busStopList.size(); i++)
    {
        if(intersectionID == busStopList[i].intersectionID && approachNo == busStopList[i].approachNo)
        {
            nearestBusStopStatus = true;
            busStopLattitude = busStopList[i].lattitude_DecimalDegree;
            busStopLongitude = busStopList[i].longitude_DecimalDegree;
            break;
        }
    }
}

/*
    - If there is a bus stop along the travel path of a intersection, the following method will check whether the vehicle bus stop or not.
    - If there is no bus stop along the travel path of a intersection, the method will always return true.
*/
bool SrmManager::checkPassedNearestBusStop(BasicVehicle basicVehicle)
{
    double distance{};

    if (findNearestBusStopLocation() == true)
    {
        distance = haversine(basicVehicle.getLatitude_DecimalDegree(), basicVehicle.getLongitude_DecimalDegree(), busStopLattitude, busStopLongitude);
        
        if (distance >= minimumDistance)
            busStopPassedStatus = true;
        else
            busStopPassedStatus = false;
    }

    else
        busStopPassedStatus = true;

    return busStopPassedStatus;
}

/*
    Haversine Formula: 
*/
double SrmManager::haversine(double lat1, double lon1, double lat2, double lon2) 
{ 
    // distance between latitudes and longitudes 
    double lattitude_Distance = (lat2 - lat1) * M_PI / 180.0; 
    double longitude_Distance = (lon2 - lon1) * M_PI / 180.0; 
    double earthRadius = 6371; //unit km
    // convert to radians 
    lat1 = (lat1) * M_PI / 180.0; 
    lat2 = (lat2) * M_PI / 180.0; 

    // apply formulae 
    double intermediateCalculation = pow(sin(lattitude_Distance / 2), 2) +  pow(sin(longitude_Distance / 2), 2) *  cos(lat1) * cos(lat2); 
    
    double distance = 2 *earthRadius * asin(sqrt(intermediateCalculation)) * kmToMeter; 
    return  distance; 
} 

/*
	- Create srm json string based on te bsm and associated information obtained based on the active map (laneID, approachID, ETA)
*/
string SrmManager::createSRMJsonObject(PriorityRequestGenerator priorityRequestGenerator, BasicVehicle basicVehicle, SignalRequest signalRequest, MapManager mapManager)
{
	std::string srmJsonString{};
	int vehExpectedTimeOfArrival_Minute{};
	double vehExpectedTimeOfArrival_Second{};
	double vehicleETADuration{};

	vehExpectedTimeOfArrival_Second = remquo((vehicleETA / SECONDSINAMINUTE), 1.0, &vehExpectedTimeOfArrival_Minute);
	vehicleETADuration = minimumETADuration;
    setMessageCount();

	signalRequest.setMinuteOfYear(priorityRequestGenerator.getMinuteOfYear());
	signalRequest.setMsOfMinute(priorityRequestGenerator.getMsOfMinute());
	signalRequest.setMsgCount(getMessageCount());
	signalRequest.setRegionalID(priorityRequestGenerator.getRegionalID());
	signalRequest.setIntersectionID(intersectionID);
	signalRequest.setVehicleType(vehicleType); 
	signalRequest.setPriorityRequestType(getPriorityRequestType());
	signalRequest.setInBoundLaneIntersectionAccessPoint(priorityRequestGenerator.getLaneID(), priorityRequestGenerator.getApproachID());
	signalRequest.setETA(vehExpectedTimeOfArrival_Minute, vehExpectedTimeOfArrival_Second * SECONDSINAMINUTE, vehicleETADuration);
	signalRequest.setTemporaryVechileID(vehicleID);
	signalRequest.setBasicVehicleRole(priorityRequestGenerator.getBasicVehicleRole());
	signalRequest.setPosition(basicVehicle.getLatitude_DecimalDegree(), basicVehicle.getLongitude_DecimalDegree(), basicVehicle.getElevation_Meter());
	signalRequest.setHeading_Degree(basicVehicle.getHeading_Degree());
	signalRequest.setSpeed_MeterPerSecond(basicVehicle.getSpeed_MeterPerSecond());
	srmJsonString = signalRequest.signalRequest2Json();

	priorityRequestGenerator.loggingData(srmJsonString);

	return srmJsonString;
}


bool SrmManager::checkRequestSendingRequirement()
{
    bool requestSendingRequirement{false};
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehicleID; });

    //If vehicle is out of the map but priority request is available in the ART, it is required to send a cancellation request.
    if (activeMapStatus == false && findVehicleIDOnTable != ActiveRequestTable.end())
	{
		requestSendingRequirement = true;
		requestSendStatus  = false;
        setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		cout << "[" << currentTime << "] SRM is sent since vehicle is out off the map" << std::endl;	
	}
	
    //If there is an active map and vehicle is leaving the intersection, it is required to send a cancellation request.
    else if (activeMapStatus == true && vehicleIntersectionStatus == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::onOutbound))) //If vehicle is out of the intersection (not in inBoundLane), vehicle should send srm and clear activeMapList
	{
		requestSendingRequirement = true;
        setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		cout << "[" << currentTime << "] SRM is sent since vehicle is either leaving or not in inBoundlane of the Intersection" << std::endl;
	}

    else if (activeMapStatus == true && vehicleIntersectionStatus == static_cast<int>(MsgEnum::mapLocType::onInbound) && (currentTime-srmTimeGapValue) >= SRM_GAPOUT_TIME) 
    {
        //If vehicleID is not in the ART, vehicle should send SRM
        if (findVehicleIDOnTable == ActiveRequestTable.end()) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityRequest));
            std::cout << "[" << currentTime << "] SRM is sent since ART is empty at time" << std::endl;
        }
        //If vehicle signal group changed it is required to send SRM
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->signalGroup != signalGroup) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle signalGroup has been changed " << std::endl;
        }

        //If vehicleID is in ART and vehicle speed changes by threshold value (for example 5m/s), vehicle should send srm. 
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && abs(findVehicleIDOnTable->vehicleSpeed - vehicleSpeed) >= vehicleSpeedDeviationLimit) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle speed has been changed" << std::endl;
        }

        //If vehicleID is in ART and vehicle ETA changes by threshold value, vehicle should send srm
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && abs(findVehicleIDOnTable->vehicleETA - vehicleETA) >= allowed_ETA_Difference) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle ETAhas been changed from " << findVehicleIDOnTable->vehicleETA << " to " << getTime2Go() << " at time " << currentTime << std::endl;
        }

        //If vehicleID is in the ART and the message count of the last sent out srm and message count in the ART doesn't match, vehicle should send srm
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->msgCount != msgCount) 
        {
        	requestSendingRequirement = true;
        	std::cout << "[" << currentTime <<"] SRM is sent since msgCount doesn't match"  << std::endl;
        }

        //Vehicle needs to send SRM to avoid request Timedout scenario in the PRS
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && (currentTime - srmTimeGapValue) >= requestTimedOutValue)
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent to avoid PRS timed out " << std::endl;
        }        
    }
}


bool SrmManager::checkRequestSendingRequirement(vector<BusStopInformation> bus_Stop_List, BasicVehicle basicVehicle)
{
    bool requestSendingRequirement{false};
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    
    if (!bus_Stop_List.empty())
        busStopList = bus_Stop_List;

    checkPassedNearestBusStop(basicVehicle);
    
    vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehicleID; });

    //If vehicle is out of the map but priority request is available in the ART, it is required to send a cancellation request.
    if (activeMapStatus == false && findVehicleIDOnTable != ActiveRequestTable.end())
	{
		requestSendingRequirement = true;
		requestSendStatus  = false;
        setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		cout << "[" << currentTime << "] SRM is sent since vehicle is out off the map" << std::endl;	
	}
	
    //If there is an active map and vehicle is leaving the intersection, it is required to send a cancellation request.
    else if (activeMapStatus == true && vehicleIntersectionStatus == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::onOutbound))) //If vehicle is out of the intersection (not in inBoundLane), vehicle should send srm and clear activeMapList
	{
		requestSendingRequirement = true;
        setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		cout << "[" << currentTime << "] SRM is sent since vehicle is either leaving or not in inBoundlane of the Intersection" << std::endl;
	}

    else if (activeMapStatus == true && busStopPassedStatus == true && vehicleIntersectionStatus == static_cast<int>(MsgEnum::mapLocType::onInbound) && (currentTime-srmTimeGapValue) >= SRM_GAPOUT_TIME) 
    {
        //If vehicleID is not in the ART, vehicle should send SRM
        if (findVehicleIDOnTable == ActiveRequestTable.end()) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityRequest));
            std::cout << "[" << currentTime << "] SRM is sent since ART is empty at time" << std::endl;
        }
        //If vehicle's signal group is changed it is required to send SRM
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->signalGroup != signalGroup) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle signalGroup has been changed " << std::endl;
        }

        //If vehicleID is in the ART and its speed is changed by threshold value (for example 5m/s), vehicle should send srm. 
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && abs(findVehicleIDOnTable->vehicleSpeed - vehicleSpeed) >= vehicleSpeedDeviationLimit) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle speed has been changed" << std::endl;
        }

        //If vehicleID is in the ART and its ETA is changed by threshold value, vehicle should send srm
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && abs(findVehicleIDOnTable->vehicleETA - vehicleETA) >= allowed_ETA_Difference) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle ETAhas been changed from " << findVehicleIDOnTable->vehicleETA << " to " << getTime2Go() << " at time " << currentTime << std::endl;
        }

        //If vehicleID is in the ART and the message count of the last sent out srm and message count in the ART doesn't match, vehicle should send srm
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->msgCount != msgCount) 
        {
        	requestSendingRequirement = true;
        	std::cout << "[" << currentTime <<"] SRM is sent since msgCount doesn't match"  << std::endl;
        }

        //Vehicle needs to send SRM to avoid request Timedout scenario in the PRS
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && (currentTime - srmTimeGapValue) >= requestTimedOutValue)
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent to avoid PRS timed out " << std::endl;
        }        
    }
}

bool SrmManager::checkRequestSendingRequirement(bool light_Siren_Status)
{
    bool requestSendingRequirement{false};
    double currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    lightSirenStatus = light_Siren_Status;
    
    vector<ActiveRequest>::iterator findVehicleIDOnTable = std::find_if(std::begin(ActiveRequestTable), std::end(ActiveRequestTable),
																			 [&](ActiveRequest const &p) { return p.vehicleID == vehicleID; });

    //If vehicle is out of the map but priority request is available in the ART, it is required to send a cancellation request.
    if (activeMapStatus == false && findVehicleIDOnTable != ActiveRequestTable.end())
	{
		requestSendingRequirement = true;
		requestSendStatus  = false;
        setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		cout << "[" << currentTime << "] SRM is sent since vehicle is out off the map" << std::endl;	
	}
	
    //If there is an active map and vehicle is leaving the intersection, it is required to send a cancellation request.
    else if (activeMapStatus == true && vehicleIntersectionStatus == (static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::atIntersectionBox) || static_cast<int>(MsgEnum::mapLocType::onOutbound))) //If vehicle is out of the intersection (not in inBoundLane), vehicle should send srm and clear activeMapList
	{
		requestSendingRequirement = true;
        setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityCancellation));
		cout << "[" << currentTime << "] SRM is sent since vehicle is either leaving or not in inBoundlane of the Intersection" << std::endl;
	}

    else if (activeMapStatus == true && lightSirenStatus == true && vehicleIntersectionStatus == static_cast<int>(MsgEnum::mapLocType::onInbound) && (currentTime-srmTimeGapValue) >= SRM_GAPOUT_TIME) 
    {
        //If vehicleID is not in the ART, vehicle should send SRM
        if (findVehicleIDOnTable == ActiveRequestTable.end()) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::priorityRequest));
            std::cout << "[" << currentTime << "] SRM is sent since ART is empty at time" << std::endl;
        }
        //If vehicle signal group changed it is required to send SRM
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->signalGroup != signalGroup) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle signalGroup has been changed " << std::endl;
        }

        //If vehicleID is in the ART and its speed is changed by threshold value (for example 5m/s), vehicle should send srm. 
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && abs(findVehicleIDOnTable->vehicleSpeed - vehicleSpeed) >= vehicleSpeedDeviationLimit) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle speed has been changed" << std::endl;
        }

        //If vehicleID is in the ART and its ETA is changed by threshold value, vehicle should send srm
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && abs(findVehicleIDOnTable->vehicleETA - vehicleETA) >= allowed_ETA_Difference) 
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent since vehicle ETAhas been changed from " << findVehicleIDOnTable->vehicleETA << " to " << getTime2Go() << " at time " << currentTime << std::endl;
        }

        //If vehicleID is in the ART and the message count of the last sent out srm and message count in the ART doesn't match, vehicle should send srm
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && findVehicleIDOnTable->msgCount != msgCount)
        {
        	requestSendingRequirement = true;
        	std::cout << "[" << currentTime <<"] SRM is sent since msgCount doesn't match"  << std::endl;
        }

        //Vehicle needs to send SRM to avoid request Timedout scenario in the PRS
        else if (findVehicleIDOnTable != ActiveRequestTable.end() && (currentTime - srmTimeGapValue) >= requestTimedOutValue)
        {
            requestSendingRequirement = true;
            setPriorityRequestType(static_cast<int>(MsgEnum::requestType::requestUpdate));
            std::cout << "[" << currentTime <<"] SRM is sent to avoid PRS timed out " << std::endl;
        }        
    }
}

SrmManager::~SrmManager()
{
}