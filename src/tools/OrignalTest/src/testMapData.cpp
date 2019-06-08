//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
/* testMapData.cpp
 * testMapData tests encoding and decoding of the MAP payload.
 * It reads an nmap file and encodes MAP payload or reads an payload file, and then decodes the encoded payload.
 *
 * Usage: testMapData -f <nmap> -s <intersection name>
 *
 * Input: .nmap file
 * Output: encoded MAP payload in hex
 *
 * Input: .payload file
 * Output: decoded MAP payload in nmap format
 * 
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

#include "AsnJ2735Lib.h"
#include "locAware.h"

void logMapPayload(const std::string& intersectionName, const std::vector<uint8_t>& payload);

void do_usage(const char* progname)
{
	std::cerr << "Usage" << progname << std::endl;
	std::cerr << "\t-f full path to intersection map file" << std::endl;
	std::cerr << "\t-s intersection name" << std::endl;
	std::cerr << "\t-i turn on encoding speed limit in lane" << std::endl;
	std::cerr << "\t-? print this message" << std::endl;
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
	int option;
	std::string fmap;
	std::string intersectionName;
	bool singleFrame = false;  /// TRUE to encode speed limit in lane, FALSE to encode in approach

	while ((option = getopt(argc, argv, "f:s:i?")) != EOF)
	{
		switch(option)
		{
		case 'f':
			fmap = std::string(optarg);
			break;
		case 'i':
				singleFrame = true;
			break;
		case 's':
			intersectionName = std::string(optarg);
			break;
		case '?':
		default:
			do_usage(argv[0]);
			break;
		}
	}
	if (fmap.empty() || intersectionName.empty())
		do_usage(argv[0]);

	/// instance class LocAware (Map Engine)
	LocAware* plocAwareLib = new LocAware(fmap, singleFrame);
	/// get intersection reference id
	uint32_t referenceId = plocAwareLib->getIntersectionIdByName(intersectionName);
	if (referenceId == 0)
	{
		std::cerr << "Failed initiating locAwareLib " << fmap << std::endl;
		delete plocAwareLib;
		return(-1);
	}
	uint16_t regionalId = static_cast<uint16_t>((referenceId >> 16) & 0xFFFF);
	uint16_t intersectionId = static_cast<uint16_t>(referenceId & 0xFFFF);
	std::vector<uint8_t> mapPayload = plocAwareLib->getMapdataPayload(regionalId, intersectionId);
	if (fmap.find(std::string(".nmap")) != std::string::npos)
	{ /// log hex payload
		std::cout << "Log encoded MAP payload." << std::endl;
		logMapPayload(intersectionName, mapPayload);
	}
	/// decode MAP
	Frame_element_t dsrcFrameOut;
	if ((AsnJ2735Lib::decode_msgFrame(&mapPayload[0], mapPayload.size(), dsrcFrameOut) > 0)
		&& (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_map))
	{
		MapData_element_t& mapData = dsrcFrameOut.mapData;
		std::cout << "IntersectionId = " << mapData.id << ", version = " << static_cast<unsigned int>(mapData.mapVersion) << std::endl;
		if (fmap.find(std::string(".payload")) != std::string::npos)
		{ /// log decoded MAP into nmap file
			std::cout << "Log namp file." << std::endl;
			plocAwareLib->saveNmap(regionalId, intersectionId);
		}
	}
	else
		std::cerr << "Failed decode_msgFrame for MAP" << std::endl;
	delete plocAwareLib;
	return(0);
}

void logMsgHex(std::ofstream& OS, const uint8_t* buf, size_t size)
{
	OS << std::hex;
	for (size_t i = 0; i < size; i++)
		OS << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buf[i]);
	OS << std::dec << std::endl;
}

void logMapPayload(const std::string& intersectionName, const std::vector<uint8_t>& payload)
{
	std::string fout = std::string("testMapData_") + intersectionName + std::string(".map.payload");
	std::ofstream OS_OUT(fout);
	OS_OUT << "Intersection: " << intersectionName << std::endl << std::endl;
	OS_OUT << "MapData payload size " << payload.size() << std::endl;
	OS_OUT << "payload " << intersectionName << " ";
	logMsgHex(OS_OUT, &payload[0], payload.size());
	OS_OUT << std::endl;
	OS_OUT.close();
}
