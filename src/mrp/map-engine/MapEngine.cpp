/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    MapEngine.cpp
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:

*/

#include <fstream>
#include <vector>
#include "MapEngine.h"
#include "geoUtils.h"

using namespace GeoUtils;
using namespace MsgEnum;

MapEngine::MapEngine()
{
    fmap = readIntersectionMapConfig();
    bool singleFrame = false; /// TRUE to encode speed limit in lane, FALSE to encode in approach
    LocAware *tempPlocAwareLib = (new LocAware(fmap, singleFrame));
    plocAwareLib = (tempPlocAwareLib);

}

MapEngine::~MapEngine()
{
    delete plocAwareLib;
}

std::string MapEngine::readIntersectionMapConfig()
{
    Json::Value jsonObject;
	Json::Reader reader;
	std::ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject);

	intersectionName = (jsonObject["IntersectionName"]).asString();
	std::string mapPayload = (jsonObject["MapPayload"]).asString();

	const char *path = "/nojournal/bin";
	std::stringstream ss;
	ss << path;
	std::string s;
	ss >> s;
	std::ofstream outputfile;

	fmap = s + "/" + intersectionName + ".map.payload"; 

	outputfile.open(fmap);

	outputfile << "payload"
			   << " "
			   << intersectionName << " " << mapPayload << std::endl;
	outputfile.close();

    return fmap;
}

std::string MapEngine::getVehicleStatusOnMap(std::string locateVehicleOnMapRequest)
{
    uint32_t referenceId = plocAwareLib->getIntersectionIdByName(intersectionName);
    regionalId = static_cast<uint16_t>((referenceId >> 16) & 0xFFFF);
    intersectionId = static_cast<uint16_t>(referenceId & 0xFFFF);

    Json::Value incomingJsonObject;
	Json::Reader reader;

	reader.parse(locateVehicleOnMapRequest.c_str(), incomingJsonObject);
    
    // Input information:
    double vehLatitude = incomingJsonObject["Vehicle"]["Latitude"].asDouble();
    double vehLongitude = incomingJsonObject["Vehicle"]["Longitude"].asDouble();
    double vehElevation = incomingJsonObject["Vehicle"]["Elevation"].asDouble();
    double vehSpeed = incomingJsonObject["Vehicle"]["Speed"].asDouble();
    double vehHeading = incomingJsonObject["Vehicle"]["Heading"].asDouble();

    // Internal variables  required for MapEngine library:
    std::bitset<4> maneuvers;
	std::vector<connectTo_t> connect2go;
	struct geoPoint_t geoPoint_t_1 = {vehLatitude, vehLongitude, vehElevation};
	struct motion_t motion_t_1 = {vehSpeed, vehHeading};
	struct intersectionTracking_t intersectionTracking_t_1 = {mapLocType::onInbound, 0, 0, 0};
    struct point2D_t point2D_t_1 = {0, 0};
    struct point2D_t point2D_t_2 = {0, 0};
	struct projection_t projection_t_1 = {0.0, 0.0, 0.0};
	struct laneProjection_t laneProjection_t_1 = {0, projection_t_1};
	struct vehicleTracking_t vehicleTracking_t_1 = {intersectionTracking_t_1, laneProjection_t_1};
	struct dist2go_t dist2go_t_1 = {0.0, 0.0};
	struct locationAware_t locationAware_t_1 = {0, 0, 0, 0, 0.0, maneuvers, dist2go_t_1, connect2go};
	struct signalAware_t signalAware_t_1 = {phaseColor::dark, phaseState::redLight, unknown_timeDetail, unknown_timeDetail, unknown_timeDetail};
	struct connectedVehicle_t connectedVehicle_t_1 = {0, 0, 0, geoPoint_t_1, motion_t_1, vehicleTracking_t_1, locationAware_t_1, signalAware_t_1};
    struct connectTo_t connectTo_t_1 = {0, 0, 0, maneuverType::straightAhead};
    std::vector<connectTo_t> connect2go1;
    connect2go1.push_back(connectTo_t_1);
    
    // Output information:    
    Json::Value outgoingJsonObject;
	outgoingJsonObject["MsgType"]="LocateVehicleOnMapStatus";
	Json::FastWriter fastWriter;
	std::string outgoingJsonString{};

    // Check if the vehicle is on the map (any approach). 
    bool vehOnMapStatus = plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1);

    // Based on the value of vehOnMapStatus, formulate the corresponding JSON string of LocateVehicleOnMapStatus message.
    if (vehOnMapStatus == true)
    {
        outgoingJsonObject["Vehicle"]["OnMap"] = true;
        std::string vehPositionOnMap;
        if(unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::onInbound) || unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::atIntersectionBox))
        /* that is if vehicle is on the map and is on the inbound approach then gather the information about for field under "Vehicle: key: laneId, approachId, controlPhaseId, distance_to_stopbar (meter),time_to_stop_bar (seconds), and inQueueStatus of the vehicle. If the vehicle is in queue, then time_to_stop_bar (seconds) equals zero.
        */
        {
            outgoingJsonObject["Vehicle"]["PositionOnMap"] = "inbound";
            int vehLaneId = plocAwareLib->getLaneIdByIndexes(unsigned(vehicleTracking_t_1.intsectionTrackingState.intersectionIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.approachIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.laneIndex));
            outgoingJsonObject["Vehicle"]["InboundStatus"]["LaneId"] = vehLaneId;
            int vehApproachId = plocAwareLib->getApproachIdByLaneId(regionalId, intersectionId, (unsigned char)((unsigned)vehLaneId));
            outgoingJsonObject["Vehicle"]["InboundStatus"]["ApproachId"] = vehApproachId;
            int vehSignalGroup = plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalId), static_cast<uint16_t>(intersectionId), static_cast<uint8_t>(vehApproachId), static_cast<uint8_t>(vehLaneId));
            outgoingJsonObject["Vehicle"]["InboundStatus"]["SignalGroup"] = vehSignalGroup;
            plocAwareLib->getPtDist2D(vehicleTracking_t_1, point2D_t_2);
            double vehDistanceToStopBar = unsigned(point2D_t_1.distance2pt(point2D_t_2))/100.0; //unit of meters;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["DistanceToStopBar_Meter"] = vehDistanceToStopBar;
            
            if (vehSpeed < queueThresholdSpeed)
            {
                outgoingJsonObject["Vehicle"]["InboundStatus"]["InQueueStatus"]=true;
                outgoingJsonObject["Vehicle"]["InboundStatus"]["TimeToStopBar_Seconds"] = false;
            } 
            else
            {
                outgoingJsonObject["Vehicle"]["InboundStatus"]["InQueueStatus"]=false;
                double vehTimeToStopBar = vehDistanceToStopBar/vehSpeed; // Seconds
                outgoingJsonObject["Vehicle"]["InboundStatus"]["TimeToStopBar_Seconds"] = vehTimeToStopBar;           
            }

        }
        else if (unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::onOutbound))
        // that is if the vehicle is in the map and is on outbound approach the gather information about laneId and approachId and set remaining fields under the "Vehicle" key to their default values.
        {   
            outgoingJsonObject["Vehicle"]["PositionOnMap"] = "outbound";
            outgoingJsonObject["Vehicle"]["OnMap"] = true;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["InQueueStatus"] = false;
            int vehLaneId = plocAwareLib->getLaneIdByIndexes(unsigned(vehicleTracking_t_1.intsectionTrackingState.intersectionIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.approachIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.laneIndex));
            outgoingJsonObject["Vehicle"]["InboundStatus"]["LaneId"] = vehLaneId;
            
            int vehApproachId = plocAwareLib->getApproachIdByLaneId(regionalId, intersectionId, (unsigned char)((unsigned)vehLaneId));
            outgoingJsonObject["Vehicle"]["InboundStatus"]["ApproachId"] = vehApproachId;

            outgoingJsonObject["Vehicle"]["InboundStatus"]["SignalGroup"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["DistanceToStopBar_Meter"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["TimeToStopBar_Seconds"] = false;
        }
        else if (unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox))
        // that is if the vehicle is in the map abd is inside the intersection box, set all fields under the "Vehicle" key to their default values.
        {
            outgoingJsonObject["Vehicle"]["PositionOnMap"] = "insideIntersectionBox";
            outgoingJsonObject["Vehicle"]["OnMap"] = true;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["InQueueStatus"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["ApproachId"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["LaneId"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["SignalGroup"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["DistanceToStopBar_Meter"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["TimeToStopBar_Seconds"] = false;
        }        
        else if (unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::outside))
        // that is if the vehicle is on the map but is in the region marked by "outside", set all fields under "Vehicle" key to their default values
        {
            outgoingJsonObject["Vehicle"]["PositionOnMap"] = "outside";
            outgoingJsonObject["Vehicle"]["OnMap"] = true;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["InQueueStatus"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["ApproachId"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["LaneId"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["SignalGroup"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["DistanceToStopBar_Meter"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["TimeToStopBar_Seconds"] = false;
        }    
    }
    else // that is if the vehicle is not on the map:
        {
            outgoingJsonObject["Vehicle"]["PositionOnMap"] = false;
            outgoingJsonObject["Vehicle"]["OnMap"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["InQueueStatus"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["ApproachId"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["LaneId"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["SignalGroup"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["DistanceToStopBar_Meter"] = false;
            outgoingJsonObject["Vehicle"]["InboundStatus"]["TimeToStopBar_Seconds"] = false;
        }

    // Create the JSON string based on developed JSON object.    
    outgoingJsonString = fastWriter.write(outgoingJsonObject);

    // Return the formulated JSON string.
    return outgoingJsonString;

}