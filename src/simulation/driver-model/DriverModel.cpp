/***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  DriverModel.cpp  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  This code is built upon the sample code for the DriverModel provided with VISSIM2020
  distribution (version of 2018-12-11, developed by Lukas Kautzsch). 

***************************************************************************************/

#pragma comment(lib, "ws2_32.lib")
# define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <fstream>
#include <cstring>
#include <string>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "json/json.h"
#include "json/json-forwards.h"
#include "geoCoord.h"
#include "DriverModel.h"
#include "MsgBlob.h"

constexpr auto PI = 3.14159265;

/*==========================================================================*/
// Define MMITSS DLL Parameters:
/*==========================================================================*/

/****************************************************************************/
// REPLACE THIS WITH THE DESIRED VEHICLE TYPE
/****************************************************************************/
// 2: EmergencyVehicle, 6: Transit, 9: Truck, 4: Car
#define VEHICLE_TYPE 4
#define CONFIG_FILE_NAME "mmitss_simulation/mmitss_driver_model_config.json"

/*==========================================================================*/
// These variables are required by Vissim. NOTE: Without these variables the
// simulation may not run.
/*==========================================================================*/

double desired_acceleration{};
double desired_lane_angle{};
long active_lane_change{};
long rel_target_lane{};
double desired_velocity{};
long turning_indicator{};
long vehicle_color = RGB(0, 0, 0);

/*==========================================================================*/
// These variables are required calculate heading.
/*==========================================================================*/
// Time-variant attributes of vehicle (internal):
double veh_x{};
double veh_y{};
double veh_rear_x{};
double veh_rear_y{};

/*==========================================================================*/
// Variables representing the time-persistent attributes of each vehicle
/*==========================================================================*/

long veh_id{};
long veh_type{};
double length_cm{};
double width_cm{};

/*==========================================================================*/
// Timestamp related variables
/*==========================================================================*/

double veh_timestamp{};
long intTimestamp{};
long veh_secmark{};
uint32_t msgCount_in{}; //TBD
/*==========================================================================*/
// Vehicle speed
/*==========================================================================*/

double veh_speed{};

/*==========================================================================*/
// Variables related to UDP socket
/*==========================================================================*/

std::string clientComputerIP{};
int clientComputerPort{};
SOCKET socketC;
WSADATA wsaData;
struct sockaddr_in clientInfo;
int len = sizeof(clientInfo);

/*==========================================================================*/
// geoPoint: an object of GeoCoord class
/*==========================================================================*/

geoCoord geoPoint;

/*==========================================================================*/

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

/*==========================================================================*/

DRIVERMODEL_API int DriverModelSetValue(long type,
                                        long index1,
                                        long index2,
                                        long long_value,
                                        double double_value,
                                        char *string_value)
{
  /* Sets the value of a data object of type <type>, selected by <index1> */
  /* and possibly <index2>, to <long_value>, <double_value> or            */
  /* <*string_value> (object and value selection depending on <type>).    */
  /* Return value is 1 on success, otherwise 0.                           */

  switch (type)
  {
  case DRIVER_DATA_PATH:
  case DRIVER_DATA_TIMESTEP:
  case DRIVER_DATA_TIME:
    veh_timestamp = double_value;           // This is timestamp from vissim. Resolution: 0.1 simulation second.
    intTimestamp = static_cast<long>(10 * veh_timestamp);      // This converts the original timestamp into integer for the use of modulo operator (%) later.
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
  case DRIVER_DATA_VEH_TYPE:
  case DRIVER_DATA_VEH_X_COORDINATE:
    veh_x = double_value;
    return 1;
  case DRIVER_DATA_VEH_Y_COORDINATE:
    veh_y = double_value;
    return 1;

  case DRIVER_DATA_VEH_Z_COORDINATE:
  case DRIVER_DATA_VEH_REAR_X_COORDINATE:
    veh_rear_x = double_value;
    return 1;
  case DRIVER_DATA_VEH_REAR_Y_COORDINATE:
       {
          veh_rear_y = double_value;
          double dx = veh_x - veh_rear_x;
          double dy = veh_y - veh_rear_y;
          double veh_heading = atan2(dy, dx) * 180.0 / PI;
          double y_origin = 0.0;
          double x_origin = 0.0;
          double veh_x_cal = ((veh_x + veh_rear_x) / 2) - x_origin;
          double veh_y_cal = ((veh_y + veh_rear_y) / 2) - y_origin;
          veh_heading = 90.0 - veh_heading;
          if (veh_heading < 0)
          {
              veh_heading = veh_heading + 360.0;
          }

          double x_grid{};
          double y_grid{};
          double z_grid{};
          geoPoint.local2ecef(veh_y_cal, veh_x_cal, 0.0, &x_grid, &y_grid, &z_grid); // Convert the current vehicle position from local to earth-center-earth-fixed co-ordinates.

          // Tranformed/calculated time-variant attributes:
          double longitude_DecimalDegree{};
          double latitude_DecimalDegree{};
          double elevation_Meter{};
          geoPoint.ecef2lla(x_grid, y_grid, z_grid, &longitude_DecimalDegree, &latitude_DecimalDegree, &elevation_Meter); // Convert the current vehicle position from earth-center-earth-fixed to latitude-longitude-altitude co-ordinates.

          uint64_t temporaryId_in = static_cast<uint64_t>(veh_id);
          uint16_t secMark_in = static_cast<uint16_t>(veh_secmark);
          int32_t latitude_DecimalDegree_in = static_cast<int32_t>(latitude_DecimalDegree * MULTIPLIER_LATITUDE);
          int32_t longitude_DecimalDegree_in = static_cast<int32_t>(longitude_DecimalDegree * MULTIPLIER_LONGITUDE);
          int32_t elevation_Meter_in = static_cast<int32_t>(elevation_Meter * MULTIPLIER_ELEVATION);
          uint16_t speed_MetersPerSecond_in = static_cast<uint16_t>(veh_speed * MULTIPLIER_SPEED);
          uint16_t heading_Degree_in = static_cast<uint16_t>(veh_heading * MULTIPLIER_HEADING);
          uint16_t length_cm_in = static_cast<uint16_t>(length_cm * MULTIPLIER_LENGTH);
          uint16_t width_cm_in = static_cast<uint16_t>(width_cm * MULTIPLIER_WIDTH);
          uint8_t vehicle_Type_in = static_cast<uint8_t>(VEHICLE_TYPE);

          if (msgCount_in > MAX_MSG_COUNT)
              msgCount_in = 0;
          msgCount_in++;
          MsgBlob msgBlobObject(msgCount_in, temporaryId_in, secMark_in, latitude_DecimalDegree_in, longitude_DecimalDegree_in, elevation_Meter_in, speed_MetersPerSecond_in, heading_Degree_in, length_cm_in, width_cm_in, vehicle_Type_in);
          sendto(socketC, msgBlobObject.CreateBSMForVissimVehicle(), BSM_BlOB_SIZE, 0, (sockaddr*)&clientInfo, len);
          return 1;
      }
  case DRIVER_DATA_VEH_REAR_Z_COORDINATE:
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
  case DRIVER_DATA_PRIO_RULE_DISTANCE:
  case DRIVER_DATA_PRIO_RULE_STATE:
  case DRIVER_DATA_ROUTE_SIGNAL_DISTANCE:
  case DRIVER_DATA_ROUTE_SIGNAL_STATE:
  case DRIVER_DATA_ROUTE_SIGNAL_CYCLE:
    return 1;
  case DRIVER_DATA_ROUTE_SIGNAL_SWITCH:
    return 0; /* (to avoid getting sent lots of signal switch data) */
  case DRIVER_DATA_CONFL_AREAS_COUNT:
    return 0; /* (to avoid getting sent lots of conflict area data) */
  case DRIVER_DATA_CONFL_AREA_TYPE:
  case DRIVER_DATA_CONFL_AREA_YIELD:
  case DRIVER_DATA_CONFL_AREA_DISTANCE:
  case DRIVER_DATA_CONFL_AREA_LENGTH:
  case DRIVER_DATA_CONFL_AREA_VEHICLES:
  case DRIVER_DATA_CONFL_AREA_TIME_ENTER:
  case DRIVER_DATA_CONFL_AREA_TIME_IN:
  case DRIVER_DATA_CONFL_AREA_TIME_EXIT:
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

DRIVERMODEL_API int DriverModelSetValue3(long type,
                                         long index1,
                                         long index2,
                                         long index3,
                                         long long_value,
                                         double double_value,
                                         char *string_value)
{
  /* Sets the value of a data object of type <type>, selected by <index1>, */
  /* <index2> and <index3>, to <long_value>, <double_value> or             */
  /* <*string_value> (object and value selection depending on <type>).     */
  /* Return value is 1 on success, otherwise 0.                            */
  /* DriverModelGetValue (DRIVER_DATA_MAX_NUM_INDICES, ...) needs to set   */
  /* *long_value to 3 or greater in order to activate this function!       */

  switch (type)
  {
  case DRIVER_DATA_ROUTE_SIGNAL_SWITCH:
    return 0; /* don't send any more switch values */
  default:
    return 0;
  }
}

/*--------------------------------------------------------------------------*/

DRIVERMODEL_API int DriverModelGetValue(long type,
                                        long index1,
                                        long index2,
                                        long *long_value,
                                        double *double_value,
                                        char **string_value)
{
  /* Gets the value of a data object of type <type>, selected by <index1> */
  /* and possibly <index2>, and writes that value to <*long_value>,       */
  /* <*double_value> or <**string_value> (object and value selection      */
  /* depending on <type>).                                                */
  /* Return value is 1 on success, otherwise 0.                           */

  switch (type)
  {
  case DRIVER_DATA_STATUS:
    *long_value = 0;
    return 1;
  case DRIVER_DATA_WANTS_ALL_SIGNALS:
    *long_value = 1; /* needs to be set to zero if no global signal data is required */
    return 1;
  case DRIVER_DATA_MAX_NUM_INDICES:
    *long_value = 3; /* because DriverModelSetValue3() and DriverModelSetValue3() exist in this DLL */
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

/*--------------------------------------------------------------------------*/

DRIVERMODEL_API int DriverModelGetValue3(long type,
                                         long index1,
                                         long index2,
                                         long index3,
                                         long *long_value,
                                         double *double_value,
                                         char **string_value)
{
  /* Gets the value of a data object of type <type>, selected by <index1>, */
  /* <index2> and <index3>, and writes that value to <*long_value>,        */
  /* <*double_value> or <**string_value> (object and value selection       */
  /* depending on <type>).                                                 */
  /* Return value is 1 on success, otherwise 0.                            */
  /* DriverModelGetValue (DRIVER_DATA_MAX_NUM_INDICES, ...) needs to set   */
  /* *long_value to 3 or greater in order to activate this function!       */

/* NOTE: THIS FUNCTION CAUSES A WARNINGC4065: switch statement contains 'default' 
but no 'case' labels. However, no change is planned to be made since this 
is VISSIM's original source code.*/

  switch (type)
  {
  default:
    return 0;
  }
}

/*==========================================================================*/

DRIVERMODEL_API int DriverModelExecuteCommand(long number)
{
  /* Executes the command <number> if that is available in the driver */
  /* module. Return value is 1 on success, otherwise 0.               */

  switch (number)
  {
  case DRIVER_COMMAND_INIT:
  {
      Json::Value jsonObject;
      std::ifstream configJson(CONFIG_FILE_NAME);
      string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
      Json::CharReaderBuilder builder;
      Json::CharReader* reader = builder.newCharReader();
      string errors{};
      reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
      delete reader;

      double ref_lon_degree = (jsonObject["vissim_origin_position"]["longitude"]["Degree"]).asDouble(); // Parse longitude (degrees part) of vissim (0,0) from the configuration file
      double ref_lon_minutes = (jsonObject["vissim_origin_position"]["longitude"]["Minute"]).asDouble(); // Parse longitude (minutes part)  of vissim (0,0) from the configuration file
      double ref_lon_seconds = (jsonObject["vissim_origin_position"]["longitude"]["Second"]).asDouble(); // Parse longitude (seconds part) of vissim (0,0) from the configuration file
      double ref_lat_degree = (jsonObject["vissim_origin_position"]["latitude"]["Degree"]).asDouble(); // Parse latiitude (degrees part) of vissim (0,0) from the configuration file
      double ref_lat_minutes = (jsonObject["vissim_origin_position"]["latitude"]["Minute"]).asDouble(); // Parse latitude (minutes part) of vissim (0,0) from the configuration file
      double ref_lat_seconds = (jsonObject["vissim_origin_position"]["latitude"]["Second"]).asDouble(); // Parse latitude (secods part) of vissim (0,0) from the configuration file
      double ref_elevation_Meter = (jsonObject["vissim_origin_position"]["elevation_Meter"]).asDouble(); // Parse latitude (secods part) of vissim (0,0) from the configuration file

      double ref_longitude_DecimalDegree = geoPoint.dms2d(ref_lon_degree, ref_lon_minutes, ref_lon_seconds); // Convert the longitude of VISSIM (0,0) parsed from the configuration file into decimal format.
      double ref_latitude_DecimalDegree = geoPoint.dms2d(ref_lat_degree, ref_lat_minutes, ref_lat_seconds); // Convert the latitude of VISSIM (0,0) parsed from the configuration file into decimal format.


      geoPoint.init(ref_longitude_DecimalDegree, ref_latitude_DecimalDegree, ref_elevation_Meter);                                         // Initialize a geo-point as VISSIM (0,0) point.

      clientComputerIP = (jsonObject["client_ip"]).asString(); // Parse client computer IP from the configuration file.
      clientComputerPort = (jsonObject["client_port"]).asInt(); // Parse client computer IUSP port from the configuration file.

      // Fill up client information:
      clientInfo.sin_family = AF_INET;                                  // Fillup client information: family
      clientInfo.sin_port = htons(clientComputerPort);                  // // Fillup client information: port
      clientInfo.sin_addr.s_addr = inet_addr(clientComputerIP.c_str()); // // Fillup client information: IP_address

      WSAStartup(MAKEWORD(2, 2), &wsaData);     // Startup WSA.
      socketC = socket(AF_INET, SOCK_DGRAM, 0); // Create and open a udp socket

      return 1;
  }
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
