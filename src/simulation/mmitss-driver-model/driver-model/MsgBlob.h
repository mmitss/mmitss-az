/***************************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
	   granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  MsgBlob.h
  Created by: Donald Cunningham / Debashis Das / Niraj Altekar
  University of Arizona
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  This code is built upon the sample code for the DriverModel provided with VISSIM2020
  distribution (version of 2018-12-11, developed by Lukas Kautzsch).

***************************************************************************************/

#include <cstdint>
#include <iostream>

#define MAX_MSG_COUNT 999999999
#define BSM_BlOB_SIZE 35
#define MULTIPLIER_LATITUDE 10000000
#define MULTIPLIER_LONGITUDE 10000000
#define MULTIPLIER_ELEVATION 100
#define MULTIPLIER_SPEED 100
#define MULTIPLIER_HEADING 100
#define MULTIPLIER_LENGTH 10
#define MULTIPLIER_WIDTH 10


using std::cout;
using std::endl;
using std::string;

class MsgBlob
{
public:

	uint32_t msgCount{}; 
	uint64_t temporaryId{};
	uint16_t secMark{};
	int32_t latitude_DecimalDegree{};
	int32_t longitude_DecimalDegree{};
	int32_t elevation_Meter{};
	uint16_t speed_MetersPerSecond{};
	uint16_t heading_Degree{};
	uint16_t length_cm{};
	uint16_t width_cm{};
	uint8_t vehicle_Type{};

    char msgBuffer[BSM_BlOB_SIZE]{};
	
    public:
    
	MsgBlob(uint32_t  msgCount,
            uint64_t temporaryId, 
            uint16_t secMark, 
            int32_t  latitude_DecimalDegree, 
            int32_t  longitude_DecimalDegree, 
            int32_t  elevation_Meter,
            uint16_t speed_MetersPerSecond,
            uint16_t heading_Degree,
            uint16_t length_cm,
            uint16_t width_cm,
            uint8_t vehicle_Type);

	~MsgBlob(void);

	char* CreateBSMForVissimVehicle(void);   //this function creates a BSM blob payload from a vehicle
	void FillMsgBuffer(int counter, char* msg_Buffer, char* dataPointer, int dataSize);

};
