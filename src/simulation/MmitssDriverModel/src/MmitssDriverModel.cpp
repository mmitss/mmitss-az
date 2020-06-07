#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <fstream>
#include <cstring>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include "MmitssDriverModel.h"
#include "geoCoord.h"
#include "json/json.h"
#include "json/json-forwards.h"
#define PI 3.142

//Following two functions are needed for establishing a socket connection
int inet_pton(int af, const char *src, void *dst)
{
	struct sockaddr_storage ss;
	int size = sizeof(ss);
	char src_copy[INET6_ADDRSTRLEN + 1];

	ZeroMemory(&ss, sizeof(ss));
	/* stupid non-const API */
	strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
	src_copy[INET6_ADDRSTRLEN] = 0;

	if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
		switch (af) {
		case AF_INET:
			*(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
			return 1;
		case AF_INET6:
			*(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
			return 1;
		}
	}
	return 0;
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
	struct sockaddr_storage ss;
	unsigned long s = size;

	ZeroMemory(&ss, sizeof(ss));
	ss.ss_family = af;

	switch (af) {
	case AF_INET:
		((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
		break;
	case AF_INET6:
		((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
		break;
	default:
		return NULL;
	}
	/* cannot direclty use &size because of strict aliasing rules */
	return (WSAAddressToString((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0) ?
		dst : NULL;
}
/*==========================================================================*/
//Default Variables: Do not remove these. Simulation will not work!
double  desired_acceleration = 0.0;
double  desired_lane_angle = 0.0;
long    active_lane_change = 0;
long    rel_target_lane = 0;
double  desired_velocity = 0.0;
long    turning_indicator = 0;
long    vehicle_color = RGB(0, 0, 0);
/*==========================================================================*/
//Variables for JsonCPP
Json::Value jsonObject;
Json::FastWriter fastWriter;
std::string jsonString;
Json::StyledStreamWriter styledStreamWriter;
Json::Value jsonObject_config;
Json::Reader reader;
std::ifstream configJson("MmitssDriverModelConfig.json");
std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
/*==========================================================================*/
// Network Config:
std::string clientComputerIP;
int clientComputerPort = 0;
SOCKET socketC;
WSADATA wsaData;
struct sockaddr_in clientInfo;
int len = sizeof(clientInfo);

//Mmitss Variables:
double	lon_degree = 0.0;
double	lon_minutes = 0.0;
double	lon_seconds = 0.0;
double	lat_degree = 0.0;
double	lat_minutes = 0.0;
double	lat_seconds = 0.0;
double	altitude = 0.0; 
double y_origin = 0.0;
double x_origin = 0.0;

double veh_x = 0.0;
double veh_y = 0.0;
double veh_rear_x = 0.0;
double veh_rear_y = 0.0;
double dx = 0.0;
double dy = 0.0;
double veh_x_cal = 0.0;
double veh_y_cal = 0.0;
double veh_heading = 0.0;
double x_grid = 0.0;
double y_grid = 0.0;
double z_grid = 0.0;

double g_long = 0.0;
double g_lat = 0.0;
double g_elev = 0.0;

double longitude = 0.0;
double latitude = 0.0;

double veh_timestamp = 0.0;
double veh_speed = 0.0;
long veh_id = 0;
long veh_type = 0;
geoCoord geoPoint;

long veh_secmark = 0;
int intTimestamp = 0;

double length_cm = 0.0;
double width_cm = 0.0;
/*==========================================================================*/




BOOL APIENTRY DllMain(HANDLE  hModule,
	DWORD   ul_reason_for_call,
	LPVOID  lpReserved)
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
/*==========================================================================*/

DRIVERMODEL_API  int  DriverModelSetValue(long   type,
	long   index1,
	long   index2,
	long   long_value,
	double double_value,
	char   *string_value)
{
	/* Sets the value of a data object of type <type>, selected by <index1> */
	/* and possibly <index2>, to <long_value>, <double_value> or            */
	/* <*string_value> (object and value selection depending on <type>).    */
	/* Return value is 1 on success, otherwise 0.                           */

	switch (type) {
	case DRIVER_DATA_PATH:
	case DRIVER_DATA_TIMESTEP:
	case DRIVER_DATA_TIME:
		veh_timestamp = double_value; // This is timestamp from vissim. Resolution: 0.1 simulation second.
		intTimestamp = 10 * veh_timestamp; // This converts the original timestamp into integer for the use of modulo operator (%) later.
		veh_secmark = intTimestamp % 600 * 100; // secMark follows the units from the SAE J2735 standard.
		return 1;
	case DRIVER_DATA_USE_UDA:
		return 0; /* doesn't use any UDAs */
				  /* must return 1 for desired values of index1 if UDA values are to be sent from/to Vissim */
	case DRIVER_DATA_VEH_ID:
		veh_id = long_value;
		return 1;
	case DRIVER_DATA_VEH_LANE:
	case DRIVER_DATA_VEH_ODOMETER:
	case DRIVER_DATA_VEH_LANE_ANGLE:
	case DRIVER_DATA_VEH_LATERAL_POSITION:
	case DRIVER_DATA_VEH_VELOCITY:
		veh_speed = double_value;
		return 1;
	case DRIVER_DATA_VEH_ACCELERATION:
	case DRIVER_DATA_VEH_LENGTH:
		length_cm = double_value * 100;
		return 1;
	case DRIVER_DATA_VEH_WIDTH:
		width_cm = double_value * 100;
		return 1;
	case DRIVER_DATA_VEH_WEIGHT:
	case DRIVER_DATA_VEH_MAX_ACCELERATION:
		return 1;
	case DRIVER_DATA_VEH_TURNING_INDICATOR:
		turning_indicator = long_value;
		return 1;
	case DRIVER_DATA_VEH_CATEGORY:
	case DRIVER_DATA_VEH_PREFERRED_REL_LANE:
	case DRIVER_DATA_VEH_USE_PREFERRED_LANE:
		return 1;
	case DRIVER_DATA_VEH_DESIRED_VELOCITY:
		desired_velocity = double_value;
		return 1;
	case DRIVER_DATA_VEH_X_COORDINATE:
		veh_x = double_value;
		return 1;
	case DRIVER_DATA_VEH_Y_COORDINATE:
		veh_y = double_value;
		return 1;
	case DRIVER_DATA_VEH_TYPE:
		veh_type = long_value;
		return 1;
	case DRIVER_DATA_VEH_Z_COORDINATE:
	case DRIVER_DATA_VEH_REAR_X_COORDINATE:
		veh_rear_x = double_value;
		return 1;
	case DRIVER_DATA_VEH_REAR_Y_COORDINATE:
		veh_rear_y = double_value;
        //Begin: Calculation of vehicle heading		
        dx = veh_x - veh_rear_x;
		dy = veh_y - veh_rear_y;
		veh_heading = atan2(dy, dx)*180.0 / PI;
		veh_x_cal = ((veh_x + veh_rear_x) / 2) - x_origin;
		veh_y_cal = ((veh_y + veh_rear_y) / 2) - y_origin;
		veh_heading = 90.0 - veh_heading;
		if (veh_heading<0)
		{
			veh_heading = veh_heading + 360.0;
		}
        //End: Calculation of vehicle heading.

        //Begin: Convert VISSIM X,Y co-ordinates into GPS co-ordinates using geo-coord class.
		geoPoint.init(longitude, latitude, altitude); // Initialize a geo-point as VISSIM (0,0) point.
		geoPoint.local2ecef(veh_y_cal, veh_x_cal, 0.0, &x_grid, &y_grid, &z_grid); // Convert the current vehicle position from local to earth-center-earth-fixed co-ordinates.
		geoPoint.ecef2lla(x_grid, y_grid, z_grid, &g_long, &g_lat, &g_elev); // Convert the current vehicle position from earth-center-earth-fixed to latitude-longitude-altitude co-ordinates.
        //End: Calculation of GPS co-ordinates. Now GPS coordinates are stored in g_long, g_lat, and g_elev.
		return 1;
	case DRIVER_DATA_VEH_REAR_Z_COORDINATE:
		/*Begin: Creation of JSON object using available information. This activity is done uder this (vehicle colour) case because,
		all the required data is available before the execution of this case. This activity can be moved any where else, but it needs to be made sure that all the required data is
		available prior to creation of json object.*/
		jsonObject["MsgType"] = "BSM";
		jsonObject["BasicVehicle"]["temporaryID"] = veh_id;
		jsonObject["BasicVehicle"]["secMark_Second"] = veh_secmark;
		jsonObject["BasicVehicle"]["position"]["latitude_DecimalDegree"] = g_lat;
		jsonObject["BasicVehicle"]["position"]["longitude_DecimalDegree"] = g_long;
		jsonObject["BasicVehicle"]["position"]["elevation_Meter"] = g_elev;
		jsonObject["BasicVehicle"]["speed_MeterPerSecond"] = veh_speed;
		jsonObject["BasicVehicle"]["heading_Degree"] = veh_heading;
		jsonObject["BasicVehicle"]["type"] = veh_type;
		jsonObject["BasicVehicle"]["size"]["length_cm"] = length_cm;
		jsonObject["BasicVehicle"]["size"]["width_cm"] = width_cm;
		//END: Creation of JSON object

		jsonString = fastWriter.write(jsonObject); // Information from JSON object is now stored in a c++ styled string jsonString.
		/* For once every second: (intTimestamp % 10 == 0)
		   For 10 times every second: (intTimestamp % 1 == 0)
		   in general, for 'n' times per second: (intTimestamp % (10/n) == 0)
		*/
	
			
		sendto(socketC, jsonString.c_str(), strlen(jsonString.c_str()), 0, (sockaddr*)&clientInfo, len); // Send message over created socket.
		
		
		return 1;
	case DRIVER_DATA_VEH_COLOR:
		vehicle_color = long_value; 
        return 1;
	case DRIVER_DATA_VEH_CURRENT_LINK:
		return 0; /* (To avoid getting sent lots of DRIVER_DATA_VEH_NEXT_LINKS messages) */
				  /* Must return 1 if these messages are to be sent from VISSIM!         */
	case DRIVER_DATA_VEH_NEXT_LINKS:
	case DRIVER_DATA_VEH_ACTIVE_LANE_CHANGE:
	case DRIVER_DATA_VEH_REL_TARGET_LANE:
	case DRIVER_DATA_VEH_INTAC_STATE:
	case DRIVER_DATA_VEH_INTAC_TARGET_TYPE:
	case DRIVER_DATA_VEH_INTAC_TARGET_ID:
	case DRIVER_DATA_VEH_INTAC_HEADWAY:
	case DRIVER_DATA_VEH_UDA:
	case DRIVER_DATA_NVEH_ID:
	case DRIVER_DATA_NVEH_LANE_ANGLE:
	case DRIVER_DATA_NVEH_LATERAL_POSITION:
	case DRIVER_DATA_NVEH_DISTANCE:
	case DRIVER_DATA_NVEH_REL_VELOCITY:
	case DRIVER_DATA_NVEH_ACCELERATION:
	case DRIVER_DATA_NVEH_LENGTH:
	case DRIVER_DATA_NVEH_WIDTH:
	case DRIVER_DATA_NVEH_WEIGHT:
	case DRIVER_DATA_NVEH_TURNING_INDICATOR:
	case DRIVER_DATA_NVEH_CATEGORY:
	case DRIVER_DATA_NVEH_LANE_CHANGE:
	case DRIVER_DATA_NVEH_TYPE:
	case DRIVER_DATA_NVEH_UDA:
	case DRIVER_DATA_NVEH_X_COORDINATE:
	case DRIVER_DATA_NVEH_Y_COORDINATE:
	case DRIVER_DATA_NVEH_Z_COORDINATE:
	case DRIVER_DATA_NVEH_REAR_X_COORDINATE:
	case DRIVER_DATA_NVEH_REAR_Y_COORDINATE:
	case DRIVER_DATA_NVEH_REAR_Z_COORDINATE:
	case DRIVER_DATA_NO_OF_LANES:
	case DRIVER_DATA_LANE_WIDTH:
	case DRIVER_DATA_LANE_END_DISTANCE:
	case DRIVER_DATA_CURRENT_LANE_POLY_N:
	case DRIVER_DATA_CURRENT_LANE_POLY_X:
	case DRIVER_DATA_CURRENT_LANE_POLY_Y:
	case DRIVER_DATA_CURRENT_LANE_POLY_Z:
	case DRIVER_DATA_RADIUS:
	case DRIVER_DATA_MIN_RADIUS:
	case DRIVER_DATA_DIST_TO_MIN_RADIUS:
	case DRIVER_DATA_SLOPE:
	case DRIVER_DATA_SLOPE_AHEAD:
	case DRIVER_DATA_SIGNAL_DISTANCE:
	case DRIVER_DATA_SIGNAL_STATE:
	case DRIVER_DATA_SIGNAL_STATE_START:
	case DRIVER_DATA_SPEED_LIMIT_DISTANCE:
	case DRIVER_DATA_SPEED_LIMIT_VALUE:
		return 1;
	case DRIVER_DATA_DESIRED_ACCELERATION:
		desired_acceleration = double_value;
		return 1;
	case DRIVER_DATA_DESIRED_LANE_ANGLE:
		desired_lane_angle = double_value;
		return 1;
	case DRIVER_DATA_ACTIVE_LANE_CHANGE:
		active_lane_change = long_value;
		return 1;
	case DRIVER_DATA_REL_TARGET_LANE:
		rel_target_lane = long_value;
		return 1;
	default:
		return 0;
	}
}

/*--------------------------------------------------------------------------*/

DRIVERMODEL_API  int  DriverModelGetValue(long   type,
	long   index1,
	long   index2,
	long   *long_value,
	double *double_value,
	char   **string_value)
{
	/* Gets the value of a data object of type <type>, selected by <index1> */
	/* and possibly <index2>, and writes that value to <*double_value>,     */
	/* <*float_value> or <**string_value> (object and value selection       */
	/* depending on <type>).                                                */
	/* Return value is 1 on success, otherwise 0.                           */

	switch (type) {
	case DRIVER_DATA_STATUS:
		*long_value = 0;
		return 1;
	case DRIVER_DATA_VEH_TURNING_INDICATOR:
		*long_value = turning_indicator;
		return 1;
	case DRIVER_DATA_VEH_DESIRED_VELOCITY:
		*double_value = desired_velocity;
		return 1;
	case DRIVER_DATA_VEH_COLOR:
		*long_value = vehicle_color;
		return 1;
	case DRIVER_DATA_VEH_UDA:
		return 0; /* doesn't set any UDA values */
	case DRIVER_DATA_WANTS_SUGGESTION:
		*long_value = 1;
		return 1;
	case DRIVER_DATA_DESIRED_ACCELERATION:
		*double_value = desired_acceleration;
		return 1;
	case DRIVER_DATA_DESIRED_LANE_ANGLE:
		*double_value = desired_lane_angle;
		return 1;
	case DRIVER_DATA_ACTIVE_LANE_CHANGE:
		*long_value = active_lane_change;
		return 1;
	case DRIVER_DATA_REL_TARGET_LANE:
		*long_value = rel_target_lane;
		return 1;
	case DRIVER_DATA_SIMPLE_LANECHANGE:
		*long_value = 1;
		return 1;
	case DRIVER_DATA_USE_INTERNAL_MODEL:
		*long_value = 1; /* must be set to 0 if external model is to be applied */
		return 1;
	case DRIVER_DATA_WANTS_ALL_NVEHS:
		*long_value = 0; /* must be set to 1 if data for more than 2 nearby vehicles per lane and upstream/downstream is to be passed from Vissim */
		return 1;
	case DRIVER_DATA_ALLOW_MULTITHREADING:
		*long_value = 0; /* must be set to 1 to allow a simulation run to be started with multiple cores used in the simulation parameters */
		return 1;
	default:
		return 0;
	}
}

/*==========================================================================*/

DRIVERMODEL_API  int  DriverModelExecuteCommand(long number) 
{
	/* Executes the command <number> if that is available in the driver */
	/* module. Return value is 1 on success, otherwise 0.               */

	switch (number) {
	case DRIVER_COMMAND_INIT: // This function is executed only once at the start of the simulation.
		reader.parse(configJsonString.c_str(), jsonObject_config); // Read the configuration file into jsonObject_config 
		lon_degree = (jsonObject_config["MmitssDriverModelConfig"]["CorridorOrigin"]["Longitude"]["Degree"]).asDouble(); // Parse longitude (degrees part) of vissim (0,0) from the configuration file
		lon_minutes = (jsonObject_config["MmitssDriverModelConfig"]["CorridorOrigin"]["Longitude"]["Minute"]).asDouble(); // Parse longitude (minutes part)  of vissim (0,0) from the configuration file
		lon_seconds = (jsonObject_config["MmitssDriverModelConfig"]["CorridorOrigin"]["Longitude"]["Second"]).asDouble(); // Parse longitude (seconds part) of vissim (0,0) from the configuration file
		lat_degree = (jsonObject_config["MmitssDriverModelConfig"]["CorridorOrigin"]["Latitude"]["Degree"]).asDouble(); // Parse latiitude (degrees part) of vissim (0,0) from the configuration file
		lat_minutes = (jsonObject_config["MmitssDriverModelConfig"]["CorridorOrigin"]["Latitude"]["Minute"]).asDouble(); // Parse latitude (minutes part) of vissim (0,0) from the configuration file
		lat_seconds = (jsonObject_config["MmitssDriverModelConfig"]["CorridorOrigin"]["Latitude"]["Second"]).asDouble(); // Parse latitude (secods part) of vissim (0,0) from the configuration file
		altitude = (jsonObject_config["MmitssDriverModelConfig"]["CorridorOrigin"]["Altitude_Meter"]).asDouble(); // Parse altitude of vissim (0,0) from the configuration file
		clientComputerIP = (jsonObject_config["MmitssDriverModelConfig"]["ClientComputer"]["IP"]).asString(); // Parse client computer IP from the configuration file. 
		clientComputerPort = (jsonObject_config["MmitssDriverModelConfig"]["ClientComputer"]["Port"]).asInt(); // Parse client computer IUSP port from the configuration file.

        // This client will receive the generated json string over UDP socket.
   		longitude = geoPoint.dms2d(lon_degree, lon_minutes, lon_seconds); // Convert the longitude of VISSIM (0,0) parsed from the configuration file into decimal format.
		latitude = geoPoint.dms2d(lat_degree, lat_minutes, lat_seconds); // Convert the latitude of VISSIM (0,0) parsed from the configuration file into decimal format.
        
        // Fill up client information:
        clientInfo.sin_family = AF_INET; // Fillup client information: family
		clientInfo.sin_port = htons(clientComputerPort); // // Fillup client information: port
		clientInfo.sin_addr.s_addr = inet_addr(clientComputerIP.c_str()); // // Fillup client information: IP_address

		WSAStartup(MAKEWORD(2, 2), &wsaData); // Startup WSA.
		socketC = socket(AF_INET, SOCK_DGRAM, 0); // Create and open a udp socket

    	return 1;
	case DRIVER_COMMAND_CREATE_DRIVER:
		return 1;
	case DRIVER_COMMAND_KILL_DRIVER:
		return 1;
	case DRIVER_COMMAND_MOVE_DRIVER:
		return 1;
	default:
		return 0;
	}
}

/*==========================================================================*/
/*  End of DriverModel.cpp                                                  */
/*==========================================================================*/ 
