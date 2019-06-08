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
#include <string.h> 
#include <arpa/inet.h> 

#include "json.h"
#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"



void logMsgHex(std::ofstream& OS, const uint8_t* buf, size_t size)
{
	OS << std::hex;
	for (size_t i = 0; i < size; i++)
		OS << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buf[i]);
	OS << std::dec << std::endl;
}

int main()
{

	Json::Value jsonObject;
	Json::Reader reader;
	/// parameters
	uint32_t vehId = 0;
	double   heading = 0.0;
	double   speed = 0.0;       /// kph
	double   latitude = 0.0;
	double   longitude = 0.0;
	double   elevation = 0.0;   /// meters
	uint16_t msOfMinute = 0;
	size_t bufSize = DsrcConstants::maxMsgSize;
	std::vector<uint8_t> buf(bufSize, 0);
	/// dsrcFrameOut to store UPER decoding result
	Frame_element_t dsrcFrameOut;
	/// dsrcFrameIn to store input to UPER encoding function
	Frame_element_t dsrcFrameIn;
	dsrcFrameIn.reset();
	/// output log file
	std::string fout = "bsmEncoder.out";
	std::ofstream OS_OUT(fout);
	char receiveBuffer[512]="";
	int recvDataN = 0;

	int socketName;
    struct sockaddr_in socketIdentifier;
	struct sockaddr_in senderIdentifier;
	struct sockaddr_in receiverIdentifier;
	receiverIdentifier.sin_family = AF_INET; // IPv4 family
    receiverIdentifier.sin_addr.s_addr = INADDR_ANY; // Any available address 
    receiverIdentifier.sin_port = htons(6666); 
	socklen_t senderAddrLen;
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
        socketIdentifier.sin_port = htons(5555); 
        
        // Bind the socket with the server address 
        if ( bind(socketName, (const struct sockaddr *)&socketIdentifier, sizeof(socketIdentifier)) < 0 ) // Bind functions return '-1' if the binding is unsuccessful.
        { 
            std::cout << "Bind UDP socket to IP and Port: Unsuccessful!" << std::endl;
        }
        else 
            std::cout << "Bind UDP socket to IP and Port: Successful!" << std::endl; 
    }
	int j=1;
    std::string sendbufString;
	while(true)
	{
	
		memset(&senderIdentifier, 0, sizeof(senderIdentifier)); // Fills zeros in the clientIdentifier variable.

		recvDataN = recvfrom(socketName, receiveBuffer, sizeof(receiveBuffer), MSG_WAITALL, (struct sockaddr *) &senderIdentifier, (socklen_t *) &senderAddrLen);
		std::cout << "Received BSM: "<< j << std::endl;
		j++;
		std::cout << "<==" << receiveBuffer << std::endl;
		receiveBuffer[recvDataN] = '\0';
		std::string recvBufferString(receiveBuffer);
		
		reader.parse(recvBufferString.c_str(), jsonObject);
		vehId = (jsonObject["MmitssBasicVehicle"]["temporaryID"]).asInt();
		std::cout << vehId << std::endl;
		
		//type = (jsonObject["MmitssBasicVehicle"]["type"]).asInt();
		speed = (jsonObject["MmitssBasicVehicle"]["speed_MeterPerSecond"]).asFloat();
		msOfMinute = (jsonObject["MmitssBasicVehicle"]["secMark_Second"]).asInt();
		heading  = (jsonObject["MmitssBasicVehicle"]["heading_Degree"]).asFloat();
		latitude = (jsonObject["MmitssBasicVehicle"]["position"]["latitude_DecimalDegree"]).asDouble();
		longitude = (jsonObject["MmitssBasicVehicle"]["position"]["longitude_DecimalDegree"]).asDouble();
		elevation = (jsonObject["MmitssBasicVehicle"]["position"]["elevation_Meter"]).asFloat();

	

		/// manual input bsmIn
		dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_bsm;
		BSM_element_t& bsmIn = dsrcFrameIn.bsm;
		bsmIn.msgCnt = 1;
		bsmIn.id = vehId;
		bsmIn.timeStampSec = msOfMinute;
		bsmIn.latitude  = DsrcConstants::unit2damega<int32_t>(latitude);
		bsmIn.longitude = DsrcConstants::unit2damega<int32_t>(longitude);
		bsmIn.elevation = DsrcConstants::unit2deca<int32_t>(elevation);
		bsmIn.yawRate   = 0;
		bsmIn.vehLen    = 1200;
		bsmIn.vehWidth  = 300;
		bsmIn.speed     = DsrcConstants::kph2unit<uint16_t>(speed);
		bsmIn.heading   = DsrcConstants::heading2unit<uint16_t>(heading);
		/// encode BSM payload
		size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);
		if (payload_size > 0)
		{
			logMsgHex(OS_OUT, &buf[0], payload_size);
			char sendbuf[1024];
			std::stringstream ss;
            ss << std::hex;
            for(int i=0;i<payload_size;i++)
                ss << std::setw(2) << std::setfill('0') << (int)buf[i];
            sendbufString = ss.str();                
            std::cout << "Encoded into UPER-HEX: " << sendbufString << std::endl;
    		sendto(socketName, sendbufString.c_str(), strlen(sendbufString.c_str()), MSG_CONFIRM, (struct sockaddr *) &receiverIdentifier, len);
		}
	}
	OS_OUT.close();
	return(0);
}
