//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
/* testDecoder.cpp
 * testDecoder tests the DSRC message encoder and decoder with hard-coded data elements.
 * DSRC messages can be tested include: BSM, SRM, SPaT, and SSM.
 *
 * Usage: testDecoder -s <BSM|SRM|SPaT|SSM>
 *
 */


#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdexcept>
#include <cstdint>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> 

#include "json.h"
#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"

	

int main()
{
	Json::Value jsonObject;
	Json::FastWriter fastWriter;
	std::string jsonString;
		
	/// buffer to hold message payload
	size_t bufSize = DsrcConstants::maxMsgSize;
	std::vector<uint8_t> buf(bufSize, 0);
	/// dsrcFrameOut to store UPER decoding result
	Frame_element_t dsrcFrameOut;
	/// dsrcFrameIn to store input to UPER encoding function
	/// output log file
	std::string fout = "bsmDecoder.out";
	std::ofstream OS_OUT(fout);		
	int j=1;

	char receiveBuffer[1024]="";
	int recvDataN = 0;

	int socketName;
    struct sockaddr_in socketIdentifier;
	struct sockaddr_in senderIdentifier;
	socklen_t senderAddrLen;
	struct sockaddr_in receiverIdentifier;
	receiverIdentifier.sin_family = AF_INET; // IPv4 family
    receiverIdentifier.sin_addr.s_addr = INADDR_ANY; // Any available address 
    receiverIdentifier.sin_port = htons(5100); 
	socklen_t len = sizeof(struct sockaddr_in);

	if((socketName = socket(AF_INET, SOCK_DGRAM, 0))< 0) // If socket creation is unsuccessful, socket() function returns '-1'.
	{
		std::cout << "Create a UDP socket: Unsuccessful!" << std::endl;
 	}
    else
    {
        std::cout << "Create a UDP socket: Successful!" << std::endl; 
        memset(&socketIdentifier, 0, sizeof(socketIdentifier)); // Fills zeros in the socketIdentifier variable.
              
        // Filling socket information 
        socketIdentifier.sin_family = AF_INET; // IPv4 family
        socketIdentifier.sin_addr.s_addr = INADDR_ANY; // Any available address 
        socketIdentifier.sin_port = htons(6666); 
        
        // Bind the socket with the server address 
        if ( bind(socketName, (const struct sockaddr *)&socketIdentifier, sizeof(socketIdentifier)) < 0 ) // Bind functions return '-1' if the binding is unsuccessful.
        { 
            std::cout << "Bind UDP socket to IP and Port: Unsuccessful!" << std::endl;
        }
        else 
            std::cout << "Bind UDP socket to IP and Port: Successful!" << std::endl; 
    }

	while(true)
	{
		memset(&senderIdentifier, 0, sizeof(senderIdentifier)); // Fills zeros in the clientIdentifier variable.
		recvDataN = recvfrom(socketName, receiveBuffer, sizeof(receiveBuffer), MSG_WAITALL, (struct sockaddr *) &senderIdentifier, (socklen_t *) &senderAddrLen);
		std::cout << "Received Encoded BSM" << std::endl;
		
		receiveBuffer[recvDataN] = '\0';
		
		std::string in(receiveBuffer);
		std::cout << "<==" << in << std::endl;
		std::string output;
		size_t cnt = in.length() / 2;

		for (size_t i = 0; cnt > i; ++i) 
		{
			uint32_t s = 0;
			std::stringstream ss;
			ss << std::hex << in.substr(i * 2, 2);
			ss >> s;
			output.push_back(static_cast<unsigned char>(s));
		}

		int index = 0;
		for (std::vector<uint8_t>::iterator it = buf.begin(); it != buf.end() && index< output.size(); ++it)
		{
			*it = output[index];
			index++;
		}
		size_t payload_size = output.size();
		if (payload_size > 0)
		{
			/// decode BSM payload
			if ((AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0)
				&& (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_bsm))
			{
				BSM_element_t& bsmOut = dsrcFrameOut.bsm;
				std::cout << "Decoded BSM: " <<j<< std::endl;
				j++;
						
				jsonObject["MmitssBasicVehicle"]["temporaryID"] = bsmOut.id;
				jsonObject["MmitssBasicVehicle"]["secMark_Second"] = bsmOut.timeStampSec;
				jsonObject["MmitssBasicVehicle"]["position"]["latitude_DecimalDegree"] = DsrcConstants::damega2unit<int32_t>(bsmOut.latitude);
				jsonObject["MmitssBasicVehicle"]["position"]["longitude_DecimalDegree"] = DsrcConstants::damega2unit<int32_t>(bsmOut.longitude);
				jsonObject["MmitssBasicVehicle"]["position"]["elevation_Meter"] = DsrcConstants::deca2unit<int32_t>(bsmOut.elevation);
				jsonObject["MmitssBasicVehicle"]["speed_MeterPerSecond"] = round(DsrcConstants::unit2kph<uint16_t>(bsmOut.speed));
				jsonObject["MmitssBasicVehicle"]["heading_Degree"] = round(DsrcConstants::unit2heading<uint16_t>(bsmOut.heading));
				jsonObject["MmitssBasicVehicle"]["type"] = 0;

				jsonString = fastWriter.write(jsonObject);
				std::cout << jsonString << std::endl;  
				sendto(socketName, jsonString.c_str(), strlen(jsonString.c_str()), MSG_CONFIRM, (struct sockaddr *) &receiverIdentifier, len);
				std::cout << "Forwarded!" << std::endl;
			}
		}
	}
	OS_OUT.close();
	return(0);
}
