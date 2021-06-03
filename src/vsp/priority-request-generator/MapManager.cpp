/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  MapManager.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This is the initial revision developed for maintaining map which will be broadcasted over the dsrc by the mapSPaT Broadcaster. 
  2. This script contains API which will manage to list (i) available map list - contain all the received maps (ii) active map list - contain only the map which is active based on bsm data.
  3. This script will delete the maps if no update is received for the corresponding map for a time being.
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <cstdio>
#include "MapManager.h"
#include "json/json.h"
#include "AsnJ2735Lib.h"
#include "locAware.h"
#include "geoUtils.h"
#include "msgEnum.h"

using namespace GeoUtils;
using namespace MsgEnum;

const double TIME_GAP_BETWEEN_RECEIVING_MAPPAYLOAD = 300.0;
const int HOURSINADAY = 24;
const int MINUTESINAHOUR = 60;
const double SECONDSINAMINUTE = 60.0;

MapManager::MapManager()
{
}

/*
    -obtain uper hex string payload from the received json string
*/
void MapManager::json2MapPayload(string jsonString)
{
    Json::Value jsonObject;
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    string errors{};
    bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
    
    if(parsingSuccessful)
    {    
        mapPayload = (jsonObject["MapPayload"]).asString();
        intersectionMapName = (jsonObject["IntersectionName"]).asString();
        intersectinID = (jsonObject["IntersectionID"]).asInt();
    }
    delete reader;
}

/*
    - setters for Map which has to be deleted from available map list.
    - If time difference between last time mapPpayload has been received and elapsed time is atleast 5 minutes delete that map. 
*/
void MapManager::setTimedOutMapPayLoad(string timedOutPayLoad)
{
    timedOutMapPayLoad = timedOutPayLoad;
}

std::string MapManager::getTimedOutMapPayLoad()
{
    return timedOutMapPayLoad;
}

/*
    -Check whether mapPayload has to be added in the available map list or not
        --If availableMapList is empty, mapPayload has to be added in the availableMapList
        --If avilableMapList is not empty but mapPayload is not in the availableMapList, mapPayload has to be added in the availableMapList
*/
bool MapManager::addToMapInList() 
{
    bool addInList{false};
    std::vector<Map::AvailableMap>::iterator findVehicleMapPayLoad = std::find_if(std::begin(availableMapList), std::end(availableMapList),
                                                                                  [&](Map::AvailableMap const &p) { return p.availableMapPayload == mapPayload; });

    if (mapPayload.size() > 0 && intersectinID > 0)
    {
        if (availableMapList.empty())
            addInList = true;

        else if (findVehicleMapPayLoad != availableMapList.end())
            addInList = false;

        else if (findVehicleMapPayLoad == availableMapList.end())
            addInList = true;
    }

    else
        addInList = false;

    return addInList;
}

/*
    -Check whether mapPayload information has to be updated in the available map list or not
        --If avilableMapList is not empty and mapPayload is in the availableMapList, mapPayload has to be added in the availableMapList
*/
bool MapManager::updateMapPayLoadList()
{
    bool updateMapList{false};
    std::vector<Map::AvailableMap>::iterator findMapPayLoad = std::find_if(std::begin(availableMapList), std::end(availableMapList),
                                                                           [&](Map::AvailableMap const &p) { return p.availableMapPayload == mapPayload; });

    if (availableMapList.empty())
        updateMapList = false;

    else if (findMapPayLoad != availableMapList.end())
        updateMapList = true;

    else if (findMapPayLoad == availableMapList.end())
        updateMapList = false;

    return updateMapList;
}

/*
    -Check whether mapPayload has to be deleted in the available map list or not
        --If time difference between last time mapPpayload has been received and elapsed time is atleast 5minutes delete that map.
*/
bool MapManager::deleteMapPayLoadFromList()
{
    bool deleteMapPayload{false};

    if (!availableMapList.empty())
    {
        for (size_t i = 0; i < availableMapList.size(); i++)
        {
            if (availableMapList[i].mapAge >= TIME_GAP_BETWEEN_RECEIVING_MAPPAYLOAD) //If year changed getMapPayloadReceivedTime() will less than availableMapList[i].minuteOfYear. Thus, abs() is used.
            {
                deleteMapPayload = true;
                setTimedOutMapPayLoad(availableMapList[i].availableMapPayload);
                break;
            }
        }
    }

    return deleteMapPayload;
}

/*
    - Method to write the mapPayload in a file based on structure require for map Engine Library.
*/
void MapManager::writeMAPPayloadInFile()
{
    const char *path = "./map";
    stringstream ss{};
    ss << path;
    string pathDirectory{};
    ss >> pathDirectory;
    ofstream outputfile;

    outputfile.open(pathDirectory + "/" + intersectionMapName + ".map.payload");
    outputfile << "payload" << " " << intersectionMapName << " " << mapPayload << endl;
    outputfile.close();
}

/*
	- Obtain system time and calculte current Minute of the Year. This time will be used for calculation of mapPayload deletion method.
		--get the number of years passed (today's number of day in the year -1)
		--get number of hours and minutes elapsed today
*/
int MapManager::getMapPayloadReceivedTime()
{
    int minuteOfYear{};
    time_t t = time(NULL);
    tm *timePtr = gmtime(&t);

    int dayOfYear = timePtr->tm_yday;
    int currentHour = timePtr->tm_hour;
    int currentMinute = timePtr->tm_min;

    minuteOfYear = (dayOfYear - 1) * HOURSINADAY * MINUTESINAHOUR + currentHour * MINUTESINAHOUR + currentMinute;

    return minuteOfYear;
}

/*
	- Obtain current time in second;
*/
int MapManager::getMapPayloadReceivedSecondOfMinute()
{
    int secondOfMinute{};
    time_t t = time(NULL);
    tm *timePtr = gmtime(&t);

    secondOfMinute = timePtr->tm_sec;

    return secondOfMinute;
}

/*
    - Check whether mapPayload has to be added in the availableMapList or updated the received time of mapPayload
*/
void MapManager::maintainAvailableMapList() //check Map.h
{
    Map::AvailableMap availableMap;

    if (addToMapInList())
    {
        string mapName = intersectionMapName;
        string mapFileDirectory = "./map/" + mapName + ".map.payload";

        writeMAPPayloadInFile();
        availableMap.availableMapPayload = mapPayload;
        availableMap.availableMapFileName = mapName;
        availableMap.availableMapFileDirectory = mapFileDirectory;
        availableMap.mapIntersectionID = intersectinID;
        availableMap.mapAge = 1;
        availableMap.minuteOfYear = getMapPayloadReceivedTime();
        availableMap.secondOfMinute = getMapPayloadReceivedSecondOfMinute();
        availableMap.activeMapStatus = "False";
        availableMapList.insert(availableMapList.begin(), availableMap);
    }

    else if (updateMapPayLoadList())
    {
        vector<Map::AvailableMap>::iterator findMapPayLoad = std::find_if(std::begin(availableMapList), std::end(availableMapList),
                                                                               [&](Map::AvailableMap const &p) { return p.availableMapPayload == mapPayload; });

        findMapPayLoad->availableMapPayload = mapPayload;
        findMapPayLoad->mapAge = 1.0;
        findMapPayLoad->minuteOfYear = getMapPayloadReceivedTime();
        findMapPayLoad->secondOfMinute = getMapPayloadReceivedSecondOfMinute();
    }
}

/*
	- This function is for deleting map from availableMapList and also delete map file from the map folder.
*/
void MapManager::deleteMap()
{
    if (deleteMapPayLoadFromList() == true)
    {
       vector<Map::AvailableMap>::iterator findMapPayLoad = std::find_if(std::begin(availableMapList), std::end(availableMapList),
                                                                               [&](Map::AvailableMap const &p) { return p.availableMapPayload == getTimedOutMapPayLoad(); });

        string deleteFileName = findMapPayLoad->availableMapFileDirectory;
        remove(deleteFileName.c_str());
        availableMapList.erase(findMapPayLoad);

    }
}

/*
	- This function is for printing availableMapList.
*/
void MapManager::printAvailableMapList()
{
    for (size_t i = 0; i < availableMapList.size(); i++)
        cout << availableMapList[i].availableMapFileName << " " << availableMapList[i].availableMapFileDirectory << " " << availableMapList[i].activeMapStatus << endl;
}

/*
	- This function is for maintaining activemaplist based on the availableMapList.
*/
void MapManager::createActiveMapList(BasicVehicle basicVehicle)
{
    Map::ActiveMap activeMap;
    string fmap{};
    string intersectionName{};

    if (activeMapList.empty() && !availableMapList.empty())
    {
        for (size_t i = 0; i < availableMapList.size(); i++)
        {
            fmap = availableMapList[i].availableMapFileDirectory;
            intersectionName = availableMapList[i].availableMapFileName;
            bool singleFrame{false}; /// TRUE to encode speed limit in lane, FALSE to encode in approach
            //Initialize mapengine library.
            LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
            //Obtain vehicle information from bsm
            double vehicle_Latitude = basicVehicle.getLatitude_DecimalDegree();
            double vehicle_Longitude = basicVehicle.getLongitude_DecimalDegree();
            double vehicle_Elevation = basicVehicle.getLongitude_DecimalDegree();
            double vehicle_Speed = basicVehicle.getSpeed_MeterPerSecond();
            double vehicle_Heading = basicVehicle.getHeading_Degree();
            //Initialize all struct require to locate vehicle in map by mapengine library.
            struct geoPoint_t geoPoint_t_1 = {vehicle_Latitude, vehicle_Longitude, vehicle_Elevation};
            struct motion_t motion_t_1 = {vehicle_Speed, vehicle_Heading};
            struct intersectionTracking_t intersectionTracking_t_1 = {mapLocType::onInbound, 0, 0, 0};
            struct projection_t projection_t_1 = {0.0, 0.0, 0.0};
            struct laneProjection_t laneProjection_t_1 = {0, projection_t_1};
            struct vehicleTracking_t vehicleTracking_t_1 = {intersectionTracking_t_1, laneProjection_t_1};
            std::bitset<4> maneuvers;
            struct dist2go_t dist2go_t_1 = {0.0, 0.0};
            struct connectTo_t connectTo_t_1 = {0, 0, 0, maneuverType::straightAhead};
            std::vector<connectTo_t> connect2go1;
            connect2go1.push_back(connectTo_t_1);
            struct locationAware_t locationAware_t_1 = {0, 0, 0, 0, 0.0, maneuvers, dist2go_t_1, connect2go1};
            struct signalAware_t signalAware_t_1 = {phaseColor::dark, phaseState::redLight, unknown_timeDetail, unknown_timeDetail, unknown_timeDetail};
            struct connectedVehicle_t connectedVehicle_t_1 = {0, 0, 0, geoPoint_t_1, motion_t_1, vehicleTracking_t_1, locationAware_t_1, signalAware_t_1};

            if (plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1) == true && unsigned(vehicleTracking_t_1.intsectionTrackingState.vehicleIntersectionStatus) == static_cast<int>(MsgEnum::mapLocType::onInbound))
            {
                activeMap.activeMapFileName = intersectionName;
                activeMap.activeMapFileDirectory = fmap;
                activeMapList.push_back(activeMap);
                break;
            }
            delete plocAwareLib;
        }
    }
}

/*
	-If vehicle is out of the intersection, activeMapList has to cleared.
*/
void MapManager::deleteActiveMapfromList()
{
    activeMapList.clear();
}

void MapManager::updateMapAge()
{
    for (size_t i = 0; i < availableMapList.size(); i++)
    {
        if (getMapPayloadReceivedSecondOfMinute() >= availableMapList[i].secondOfMinute)
        {

            availableMapList[i].mapAge = availableMapList[i].mapAge + (getMapPayloadReceivedSecondOfMinute() - availableMapList[i].secondOfMinute);
            availableMapList[i].minuteOfYear = getMapPayloadReceivedTime();
            availableMapList[i].secondOfMinute = getMapPayloadReceivedSecondOfMinute();
        }

        else if (getMapPayloadReceivedSecondOfMinute() < availableMapList[i].secondOfMinute)
        {
            availableMapList[i].mapAge = availableMapList[i].mapAge + (getMapPayloadReceivedSecondOfMinute() + SECONDSINAMINUTE - availableMapList[i].secondOfMinute);
            availableMapList[i].minuteOfYear = getMapPayloadReceivedTime();
            availableMapList[i].secondOfMinute = getMapPayloadReceivedSecondOfMinute();
        }
    }
}

/*
    - Method for obtaining reference point of the received Map using map engine library
*/
void MapManager::getReferencePoint()
{
    std::string fmap{};
    bool singleFrame{false};
    int intersectionIndex{};
    struct geoRefPoint_t geoRefPoint_t_1 = {0, 0, 0};
    fmap = "./map/" + intersectionMapName + ".map.payload";

    LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
    intersectionIndex = plocAwareLib->getIndexByIntersectionId(static_cast<int16_t>(regionalID), static_cast<int16_t>(intersectinID));
    geoRefPoint_t_1 = plocAwareLib->getIntersectionRefPoint(static_cast<int8_t>(intersectionIndex));
    
    delete plocAwareLib;
    
    mapReferenceLatitude = geoRefPoint_t_1.latitude / 10000000.0;
    mapReferenceLongitude = geoRefPoint_t_1.longitude / 10000000.0;
    mapReferenceElevation = geoRefPoint_t_1.elevation * 0.1;
}

/*
    - Getters for reference point(latitude) of received Map
*/
double MapManager::getMapReferenceLatitude()
{
    return mapReferenceLatitude;
}

/*
    - Getters for reference point(longitude) of received Map
*/
double MapManager::getMapReferenceLongitude()
{
    return mapReferenceLongitude;
}

/*
    - Getters for reference point(elevation) of received Map
*/
double MapManager::getMapReferenceElevation()
{
    return mapReferenceElevation;
}

/*
    - Getters for Active map List
*/
    vector<Map::ActiveMap> MapManager::getActiveMapList()
{
    return activeMapList;
}

/*
	- Getters for Available map List
*/
vector<Map::AvailableMap> MapManager::getAvailableMapList()
{
    return availableMapList;
}

MapManager::~MapManager()
{
}