/*
**********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  testMEL.cpp
  Created by: Debashis Das
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. This script is the demonstration of conversion of GPS coordinates into local coordinates.
*/

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <unistd.h>
#include <cstring>

#include "AsnJ2735Lib.h"
#include "locAware.h"
#include "geoCoord.h"
#include "json/json.h"

using namespace GeoUtils;
using namespace MsgEnum;

int main()
{
	double vehicle_Speed{};
	double vehicle_Heading{};
	double vehicle_Latitude{};
	double vehicle_Longitude{};
	double vehicle_Elevation{};
	// double MAPpoints_Latitude{};
	// double MAPpoints_Longitude{};
	// double MAPpoints_Elevation{};
	double referenceLatitude{};
	double referenceLongitude{};
	double referenceElevation{};
	double ecef_x, ecef_y, ecef_z;
	double local_x, local_y, local_z;
	bool singleFrame = false; /// TRUE to encode speed limit in lane, FALSE to encode in approach
	std::ifstream infile;
	std::string lineread{};
	geoCoord geoPoint;
	std::string mapPayload{};
	std::string intersectionName{};
	std::string fmap{};

	//Reading configuration file to get map payload and intersection name
	Json::Value jsonObject_config;
	Json::Reader reader;
	std::ifstream configJson("mapEngineLibraryTestConfig.json");
	std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject_config);
	mapPayload = (jsonObject_config["MapPayload"]).asString();
	intersectionName = (jsonObject_config["IntersectionName"]).asString();
	referenceLatitude =(jsonObject_config["ReferencePoint"]["Latitude"]).asDouble();
	referenceLongitude = (jsonObject_config["ReferencePoint"]["Longitude"]).asDouble();
	referenceElevation = (jsonObject_config["ReferencePoint"]["Elevation"]).asDouble();
	vehicle_Speed = (jsonObject_config["VehicleSpeed"]).asDouble();

	fmap = "./map/" + intersectionName + ".map.payload";

	// creating mapPayload file
	const char *path = "./map";
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

	/// instance class LocAware (Map Engine)
	LocAware *plocAwareLib = new LocAware(fmap, singleFrame);
	// uint32_t referenceId = plocAwareLib->getIntersectionIdByName(intersectionName);
	// uint16_t regionalId = static_cast<uint16_t>((referenceId >> 16) & 0xFFFF);
	// uint16_t intersectionId = static_cast<uint16_t>(referenceId & 0xFFFF);

	geoPoint.init(referenceLongitude, referenceLatitude, referenceElevation); //Initializing geoPoint

	/*
    	Converting Vehicle Coordinates into local coordinates and locate vehicle in MAP.
    	Define the file name to read vehicle GPS Coordinates and heading
    	*/
	infile.open("GPSCoordinates.txt");
	if (infile.fail())
	{
		std::cout << "fail to open the file" << std::endl;
	}

	else
	{
		std::ofstream outfile("Local_GPS_Coordinates.txt"); //Define the file name to store the output
		
		outputfile.open("Local_GPS_Coordinates.txt");
		while (getline(infile, lineread))
		{
			sscanf(lineread.c_str(), "%lf %lf %lf %lf", &vehicle_Latitude, &vehicle_Longitude, &vehicle_Elevation, &vehicle_Heading);

			//Declaring parameters for locating vehicles in MAP
			std::bitset<4> maneuvers;
			std::vector<connectTo_t> connect2go;
			struct geoPoint_t geoPoint_t_1 = {vehicle_Latitude, vehicle_Longitude, vehicle_Elevation};
			struct motion_t motion_t_1 = {vehicle_Speed, vehicle_Heading};
			struct intersectionTracking_t intersectionTracking_t_1 = {mapLocType::onInbound, 0, 0, 0};
			struct projection_t projection_t_1 = {0.0, 0.0, 0.0};
			struct laneProjection_t laneProjection_t_1 = {0, projection_t_1};
			struct vehicleTracking_t vehicleTracking_t_1 = {intersectionTracking_t_1, laneProjection_t_1};
			struct dist2go_t dist2go_t_1 = {0.0, 0.0};
			struct locationAware_t locationAware_t_1 = {0, 0, 0, 0, 0.0, maneuvers, dist2go_t_1, connect2go};
			struct signalAware_t signalAware_t_1 = {phaseColor::dark, phaseState::redLight, unknown_timeDetail, unknown_timeDetail, unknown_timeDetail};
			struct connectedVehicle_t connectedVehicle_t_1 = {0, 0, 0, geoPoint_t_1, motion_t_1, vehicleTracking_t_1, locationAware_t_1, signalAware_t_1};

			//coverting vehicle gps points into ecef coordinates
			geoPoint.lla2ecef(vehicle_Longitude, vehicle_Latitude, vehicle_Elevation, &ecef_x, &ecef_y, &ecef_z);
			//coverting ecef coordinates into local coordinates
			geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);

			//Write the output in file
			outputfile << local_x << "," << local_y << "," << local_z << "," << vehicle_Heading << "," << plocAwareLib->locateVehicleInMap(connectedVehicle_t_1, vehicleTracking_t_1) << std::endl;
		}

		outputfile.close();
		infile.close();
	}

	// /*
	// 	Converting MAP node points into local coordinates obtain from decoding the payload.
	// */
	// infile.open("Map_GPSCoordinates.txt");
	// if (infile.fail())
	// {
	// 	std::cout << "fail to open the file" << std::endl;
	// 	;
	// }

	// else
	// {
	// 	std::ofstream outfile("Local_Map_GPSCoordinates.txt");
	// 	std::ofstream outputfile;
	// 	outputfile.open("Local_Map_GPSCoordinates.txt");
	// 	while (getline(infile, lineread))
	// 	{
	// 		sscanf(lineread.c_str(), "%lf %lf %lf", &MAPpoints_Latitude, &MAPpoints_Longitude, &MAPpoints_Elevation);

	// 		geoPoint.lla2ecef(MAPpoints_Longitude, MAPpoints_Latitude, MAPpoints_Elevation, &ecef_x, &ecef_y, &ecef_z);
	// 		geoPoint.ecef2local(ecef_x, ecef_y, ecef_z, &local_x, &local_y, &local_z);

	// 		outputfile << local_x << "," << local_y << "," << local_z << std::endl;
	// 	}

	// 	outputfile.close();
	// 	infile.close();
	// }

	delete plocAwareLib;
	return (0);
}
