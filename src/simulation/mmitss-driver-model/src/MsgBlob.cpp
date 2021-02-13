/***************************************************************************************

 © 2021 Arizona Board of Regents on behalf of the University of Arizona with rights
	   granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************

  MsgBlob.cpp
  Created by: Donald Cunningham / Debashis Das / Niraj Altekar
  University of Arizona
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  This code is built upon the sample code for the DriverModel provided with VISSIM2020
  distribution (version of 2018-12-11, developed by Lukas Kautzsch).

***************************************************************************************/

#include "MsgBlob.h"

MsgBlob::MsgBlob(uint32_t  msgCount1,
                 uint64_t temporaryId1, 
                 uint16_t secMark1, 
                 int32_t  latitude_DecimalDegree1, 
                 int32_t  longitude_DecimalDegree1, 
                 int32_t  elevation_Meter1,
                 uint16_t speed_MetersPerSecond1,
                 uint16_t heading_Degree1,
                 uint16_t length_cm1,
                 uint16_t width_cm1,
                 uint8_t vehicle_Type1)
{
    msgCount = msgCount1;
    temporaryId = temporaryId1; 
    secMark = secMark1;
    latitude_DecimalDegree = latitude_DecimalDegree1; 
    longitude_DecimalDegree = longitude_DecimalDegree1; 
    elevation_Meter = elevation_Meter1;
    speed_MetersPerSecond = speed_MetersPerSecond1;
    heading_Degree = heading_Degree1;
    length_cm = length_cm1;
    width_cm = width_cm1;
    vehicle_Type = vehicle_Type1;
}

MsgBlob::~MsgBlob()
{
}


// This function creates a BSM payload from a vehicle data come from VISSIM, 
// BSM blob payload would be located in a octet string stream
char* MsgBlob::CreateBSMForVissimVehicle(void)
{

	int offset_counter{};         // a counter 	
	
	// do msgCount   (4 byte)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&msgCount), 4);
	offset_counter += 4; // move past to next item

	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&temporaryId), 8);
	offset_counter += 8;

	// do the secMark  (2 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&secMark), 2);
	offset_counter += 2;

	// do the latitude_DecimalDegree (4 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&latitude_DecimalDegree), 4);
	offset_counter += 4;

	// do the longitude_DecimalDegree (4 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&longitude_DecimalDegree), 4);
	offset_counter += 4;

	// do the elevation_Meter (4 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&elevation_Meter), 4);
	offset_counter += 4;

	// do the speed_MetersPerSecond (2 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&speed_MetersPerSecond), 2);
	offset_counter += 2;

	// do the heading_Degree (2 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&heading_Degree), 2);
	offset_counter += 2;

	// do the length_cm (2 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&length_cm), 2);
	offset_counter += 2;

	// do the width_cm (2 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&width_cm), 2);
	offset_counter += 2;

	// do the vehicle_Type (1 bytes)
	FillMsgBuffer(offset_counter, msgBuffer, reinterpret_cast<char*>(&vehicle_Type), 1);
	// offset_counter += 1;

	// msgBuffer[offset_counter] = '\0';
	// assert(offset_counter == BSM_BlOB_SIZE); // check if we exactlly used 38 byte of blob
	return msgBuffer;
}


void MsgBlob::FillMsgBuffer(int counter, char* msg_Buffer, char* dataPointer, int dataSize)
{
	for (int i = 0; i < dataSize; i++)
		msg_Buffer[counter + i] = *(dataPointer + (dataSize - i - 1));
}