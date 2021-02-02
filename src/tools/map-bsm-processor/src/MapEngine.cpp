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
#include <string>
#include <sstream>
#include "MapEngine.h"
#include "geoUtils.h"


using namespace GeoUtils;
using namespace MsgEnum;

MapEngine::MapEngine(std::string configFilename)
{
    fmap = readIntersectionMapConfig(configFilename);
    bool singleFrame = false; /// TRUE to encode speed limit in lane, FALSE to encode in approach
    LocAware *tempPlocAwareLib = (new LocAware(fmap, singleFrame));
    plocAwareLib = (tempPlocAwareLib);
    uint32_t referenceId = plocAwareLib->getIntersectionIdByName(intersectionName);
    regionalId = static_cast<uint16_t>((referenceId >> 16) & 0xFFFF);
    intersectionId = static_cast<uint16_t>(referenceId & 0xFFFF);
}

MapEngine::~MapEngine()
{
    delete plocAwareLib;
}

bool MapEngine::isVehicleOnMap(double latitude, double longitude, double elevation, double speed, double heading)
{
    std::bitset<4> maneuvers;
	std::vector<connectTo_t> connect2go;
	struct geoPoint_t geoPoint_t_1 = {latitude, longitude, elevation};
	struct motion_t motion_t_1 = {speed, heading};
	struct intersectionTracking_t intersectionTracking_t_1 = {mapLocType::onInbound, 0, 0, 0};
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

    bool isOnMap = plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1);

    return isOnMap;
}

std::string MapEngine::readIntersectionMapConfig(std::string configFilename)
{
	Json::Value jsonObject;
	// Json::Reader reader;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	std::string errors{};
	std::ifstream jsonconfigfile(configFilename);
    std::string mapPayload{};
	std::string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	// reader.parse(configJsonString.c_str(), jsonObject);
	bool parsingSuccessful = reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
	// set the request timed out value to avoid clearing the old request in PRS
	    
    if (parsingSuccessful == true)
    {
	    intersectionName = (jsonObject["IntersectionName"]).asString();
	    mapPayload = (jsonObject["MapPayload"]).asString();
    }

    delete reader;    

	const char *path = "../config";
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

std::string MapEngine::getOnMapCsv(double latitude, double longitude, double elevation, double speed, double heading)
{
    // Internal variables  required for MapEngine library:
    std::bitset<4> maneuvers;
	std::vector<connectTo_t> connect2go;
	struct geoPoint_t geoPoint_t_1 = {latitude, longitude, elevation};
	struct motion_t motion_t_1 = {speed, heading};
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
    plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1);
    int vehLaneId = plocAwareLib->getLaneIdByIndexes(unsigned(vehicleTracking_t_1.intsectionTrackingState.intersectionIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.approachIndex), unsigned(vehicleTracking_t_1.intsectionTrackingState.laneIndex));
    int vehApproachId = plocAwareLib->getApproachIdByLaneId(regionalId, intersectionId, (unsigned char)((unsigned)vehLaneId));
    plocAwareLib->getPtDist2D(vehicleTracking_t_1, point2D_t_2);
    double vehDistanceToStopBar = unsigned(point2D_t_1.distance2pt(point2D_t_2))/100.0; //unit of meters;
    int vehSignalGroup = plocAwareLib->getControlPhaseByIds(static_cast<uint16_t>(regionalId), static_cast<uint16_t>(intersectionId), static_cast<uint8_t>(vehApproachId), static_cast<uint8_t>(vehLaneId));
    std::string locationOnMap{};

    if(unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::onInbound))
    {
        locationOnMap = "inbound";
    }
    else if(unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::onOutbound))
    {
        locationOnMap = "outbound";
    } 
    else if(unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::insideIntersectionBox))
    {
        locationOnMap = "insideIntersectionBox";
    }

    if(locationOnMap=="insideIntersectionBox")
    {
        vehSignalGroup = 0;
        vehApproachId = 0;
        vehLaneId = 0;
    }   

    std::stringstream ss{};
    ss << locationOnMap << "," << vehApproachId << "," << vehLaneId << "," << vehSignalGroup << "," << vehDistanceToStopBar;

    std::string outgoingCsv = ss.str();

    return outgoingCsv;
}