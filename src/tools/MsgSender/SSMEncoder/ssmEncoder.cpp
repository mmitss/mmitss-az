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

#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"
#include "json/json.h"

void logMsgHex(std::ofstream& OS, const uint8_t* buf, size_t size)
{
	OS << std::hex;
	for (size_t i = 0; i < size; i++)
		OS << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buf[i]);
	OS << std::dec << std::endl;
}

void print_ssmdata(std::ofstream& OS, const SSM_element_t& ssm)
{
	OS << "SSM UPER (Unaligned Packed Encoding Rules) decoding result:" << std::endl << std::endl;
	OS << "minuteOfYear: " << ssm.timeStampMinute << std::endl;
	OS << "msOfMinute: " << ssm.timeStampSec << std::endl;
	OS << "sequenceNumber: " << static_cast<unsigned int>(ssm.msgCnt) << std::endl;
	OS << "SignalStatusList: 1" << std::endl;
	OS << "\t" << "updateCnt: " << static_cast<unsigned int>(ssm.updateCnt) << std::endl;
	OS << "\t" << "id" << std::endl;
	OS << "\t\t" << "regionalId: " << ssm.regionalId << std::endl;
	OS << "\t\t" << "intersectionId: " << ssm.id << std::endl;
	OS << "\t" << "SignalRequest: " << ssm.mpSignalRequetStatus.size() << std::endl;
	size_t reqNo = 1;
	for (const auto& RequetStatus : ssm.mpSignalRequetStatus)
	{
		OS << "\t\t" << "request #: " << reqNo++ << std::endl;
		OS << "\t\t\t" << "requester" << std::endl;
		OS << "\t\t\t\t" << "VehicleID: " << RequetStatus.vehId << std::endl;
		OS << "\t\t\t\t" << "RequestID: " << static_cast<unsigned int>(RequetStatus.reqId) << std::endl;
		OS << "\t\t\t\t" << "sequenceNumber: " << static_cast<unsigned int>(RequetStatus.sequenceNumber) << std::endl;
		OS << "\t\t\t\t" << "role: " << static_cast<unsigned int>(RequetStatus.vehRole) << std::endl;
		if (RequetStatus.inApprochId != 0)
			OS << "\t\t\t" << "inBoundLane(approach): " << static_cast<unsigned int>(RequetStatus.inApprochId) << std::endl;
		else
			OS << "\t\t\t" << "inBoundLane(lane): " << static_cast<unsigned int>(RequetStatus.inLaneId) << std::endl;
		if (RequetStatus.outApproachId != 0)
			OS << "\t\t\t" << "outBoundLane(approach): " << static_cast<unsigned int>(RequetStatus.outApproachId) << std::endl;
		else
			OS << "\t\t\t" << "outBoundLane(lane): " << static_cast<unsigned int>(RequetStatus.outLaneId) << std::endl;
		OS << "\t\t\t" << "ETAminute: " << RequetStatus.ETAminute << std::endl;
		OS << "\t\t\t" << "ETAsec: " << RequetStatus.ETAsec << std::endl;
		OS << "\t\t\t" << "duration: " << RequetStatus.duration << std::endl;
		OS << "\t\t\t" << "PRS status: " << static_cast<unsigned int>(RequetStatus.status) << std::endl;
	}
}


int main(int argc, char** argv)
{

// //Decode Json
// 	Json::Value root;               // Json root
//     Json::Reader parser;            // Json parser
//     std::ifstream ifs;
//     ifs.open("output.json");
//     //assert(ifs.is_open());
//     int noOfRequest{};
//     // std::string laneID = "inBoundLaneID";
//  //    basicVehicleRole: 9
// 	// inBoundLaneID: 12
// 	// msgCount: 6
// 	// priorityRequestStatus: 4
// 	// requestID: 5
// 	// vehicleETA: 82.000999450683594
// 	// vehicleID
    
 
    
    
//     // Parse the json
//     bool bIsParsed = parser.parse( ifs, root );
//     if (bIsParsed == true)
//     {   
//         std::cout<<"parsed successfully"<<std::endl;
//         noOfRequest = (root["noOfRequest"]).asInt();
//         int laneID[noOfRequest];
//         // Get the values
//         const Json::Value values = root["MmitssSignalStatus"]["requestorInfo"];

//         // Print the objects
//         //for ( int i = 0; i < values.size(); i++ )
//         for ( int i = 0; i < noOfRequest; i++ )
//         {
//             // Print the values
//             //cout << values[i] << endl;

//             // Print the member names and values individually of an object
//             for(int j = 0; j < values[i].getMemberNames().size(); j++)
//             {
//                 // Member name and value
//                 std::cout << values[i].getMemberNames()[j] << ": " << values[i][values[i].getMemberNames()[j]].asString() << std::endl;
                
//                  if (values[i].getMemberNames()[j]=="basicVehicleRole")
//                     laneID[i] = values[i][values[i].getMemberNames()[j]].asInt();

// 	// inBoundLaneID: 12
// 	// msgCount: 6
// 	// priorityRequestStatus: 4
// 	// requestID: 5
// 	// vehicleETA: 82.000999450683594
// 	// vehicleID

//                 if (values[i].getMemberNames()[j]=="inBoundLaneID")
//                     laneID[i] = values[i][values[i].getMemberNames()[j]].asInt();
//             }
//             std::cout<<"LaneID: "<<laneID[0]<<std::endl;
//             std::cout<<"LaneID: "<<laneID[1]<<std::endl;
//             std::cout<<"LaneID: "<<laneID[2]<<std::endl;
//         }
//     }
//     else
//     {
//         std::cout << "Cannot parse the json content!" << std::endl;
//     }




	/// parameters
	const uint16_t regionalId = 0;
	const uint16_t intersectionId = 1003;
	const uint32_t vehId = 601;
	const uint8_t  priorityLevel = 5;
	const uint8_t  inLaneId = 8;
	const uint8_t  outLaneId = 30;
	const double   heading = 60.0;
	const double   speed = 35.0;       /// kph
	const double   latitude = 37.4230638;
	const double   longitude = -122.1420467;
	const double   elevation = 12.6;   /// meters
	const uint32_t time2go = 29000;    /// 30 seconds
	const uint16_t duration = 2000;    /// 2 seconds
	const uint16_t msOfMinute = 50001;
	const uint32_t minuteOfYear = 120001;
	uint32_t ETAsec = msOfMinute + time2go;
	/// buffer to hold message payload
	size_t bufSize = DsrcConstants::maxMsgSize;
	std::vector<uint8_t> buf(bufSize, 0);
	/// dsrcFrameOut to store UPER decoding result
	Frame_element_t dsrcFrameOut;
	/// dsrcFrameIn to store input to UPER encoding function
	Frame_element_t dsrcFrameIn;
	dsrcFrameIn.reset();
	/// output log file
	std::string fout = "SSM.out";
	std::ofstream OS_OUT(fout);

	//second ssm input

	const uint32_t vehId2 = 605;
	const uint8_t  priorityLevel2 = 5;
	const uint8_t  inLaneId2 = 11;
	const uint8_t  outLaneId2 = 26;
	const double   heading2 = 150.0;
	const double   speed2 = 35.0;       /// kph
	const double   latitude2 = 37.4232238;
	const double   longitude2 = -122.1422688;
	const double   elevation2 = 12.6;   /// meters
	const uint32_t time2go2 = 39000;    /// 30 seconds
	const uint16_t duration2 = 2000;    /// 2 seconds
	const uint16_t msOfMinute2 = 50002;
	const uint32_t minuteOfYear2 = 120002;
	uint32_t ETAsec2 = msOfMinute2 + time2go2;


	/// manual input ssmIn
		dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_ssm;
		SSM_element_t& ssmIn = dsrcFrameIn.ssm;
		ssmIn.timeStampMinute = minuteOfYear;
		ssmIn.timeStampSec = msOfMinute;
		ssmIn.msgCnt = 4;
		ssmIn.updateCnt = 1;
		ssmIn.regionalId = regionalId;
		ssmIn.id = intersectionId;
		SignalRequetStatus_t requestStatus;
		requestStatus.reset();
		requestStatus.vehId = vehId;
		requestStatus.reqId = priorityLevel;
		requestStatus.sequenceNumber = 2;
		requestStatus.vehRole   = MsgEnum::basicRole::transit;
		requestStatus.inLaneId  = inLaneId;
		requestStatus.outLaneId = outLaneId;
		requestStatus.ETAminute = static_cast<uint32_t>(minuteOfYear + std::floor(ETAsec / 60000));
		requestStatus.ETAsec    = static_cast<uint16_t>((ETAsec % 60000) & 0xFFFF);
		requestStatus.duration  = duration;
		requestStatus.status    = MsgEnum::requestStatus::granted;
		ssmIn.mpSignalRequetStatus.push_back(requestStatus);

		// SignalRequetStatus_t requestStatus2;
		// requestStatus.reset();
		requestStatus.vehId = vehId2;
		requestStatus.reqId = priorityLevel2;
		requestStatus.sequenceNumber = 3;
		requestStatus.vehRole   = MsgEnum::basicRole::truck;
		requestStatus.inLaneId  = inLaneId2;
		requestStatus.outLaneId = outLaneId2;
		requestStatus.ETAminute = static_cast<uint32_t>(minuteOfYear2 + std::floor(ETAsec2 / 60000));
		requestStatus.ETAsec    = static_cast<uint16_t>((ETAsec2 % 60000) & 0xFFFF);
		requestStatus.duration  = duration2;
		requestStatus.status    = MsgEnum::requestStatus::requested;
		ssmIn.mpSignalRequetStatus.push_back(requestStatus);		

		/// encode SSM payload
		size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);
		if (payload_size > 0)
		{
			std::cout << "encode_ssm_payload succeed" << std::endl;
			/// decode SSM payload
			if ((AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0)
				&& (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_ssm))
			{
				SSM_element_t& ssmOut = dsrcFrameOut.ssm;
				std::cout << "decode_msgFrame for SSM succeed" << std::endl;
				OS_OUT << "Payload size " << payload_size << std::endl;
				logMsgHex(OS_OUT, &buf[0], payload_size);
				OS_OUT << std::endl;
				print_ssmdata(OS_OUT, ssmOut);
			}
			else
				std::cout << "Failed decode_msgFrame for SSM" << std::endl;
		}
		else
			std::cout << "Failed encode_msgFrame for SSM" << std::endl;
		std::cout << "Done test SSM encode and decode" << std::endl << std::endl;


	
	OS_OUT.close();
	return(0);
}