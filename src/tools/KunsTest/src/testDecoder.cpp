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

void logMsgHex(std::ofstream& OS, const uint8_t* buf, size_t size)
{
	OS << std::hex;
	for (size_t i = 0; i < size; i++)
		OS << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buf[i]);
	OS << std::dec << std::endl;
}

void print_bsmdata(std::ofstream& OS, const BSM_element_t& bsm)
{
	OS << "BSM UPER (Unaligned Packed Encoding Rules) decoding result:" << std::endl << std::endl;
	OS << "BSMcoreData" << std::endl;
	OS << "\t" << "msgCnt: " << static_cast<unsigned int>(bsm.msgCnt) << std::endl;
	OS << "\t" << "id: " << bsm.id << std::endl;
	OS << "\t" << "secMark: " << bsm.timeStampSec << std::endl;
	OS << "\t" << "lat: " << std::fixed << std::setprecision(7) << DsrcConstants::damega2unit<int32_t>(bsm.latitude) << std::endl;
	OS << "\t" << "long: " << std::fixed << std::setprecision(7) << DsrcConstants::damega2unit<int32_t>(bsm.longitude) << std::endl;
	OS << "\t" << "elev: " << std::fixed << std::setprecision(1) << DsrcConstants::deca2unit<int32_t>(bsm.elevation) << std::endl;
	OS << "\t" << "PositionalAccuracy" << std::endl;
	OS << "\t\t" << "semiMajor: " << static_cast<unsigned int>(bsm.semiMajor) << std::endl;
	OS << "\t\t" << "semiMinor: " << static_cast<unsigned int>(bsm.semiMinor) << std::endl;
	OS << "\t\t" << "orientation: " << bsm.orientation << std::endl;
	OS << "\t" << "transmission : " << static_cast<unsigned int>(bsm.transState) << std::endl;
	OS << "\t" << "speed : " << round(DsrcConstants::unit2kph<uint16_t>(bsm.speed)) << std::endl;
	OS << "\t" << "heading : " << round(DsrcConstants::unit2heading<uint16_t>(bsm.heading)) << std::endl;
	OS << "\t" << "SteeringWheelAngle: " << static_cast<int>(bsm.steeringAngle) << std::endl;
	OS << "\t" << "AccelerationSet4Way" << std::endl;
	OS << "\t\t" << "accLong: " << bsm.accelLon << std::endl;
	OS << "\t\t" << "accLat: " << bsm.accelLat << std::endl;
	OS << "\t\t" << "accVert: " << static_cast<int>(bsm.accelVert) << std::endl;
	OS << "\t\t" << "yaw: " << bsm.yawRate << std::endl;
	OS << "\t" << "BrakeSystemStatus" << std::endl;
	OS << "\t\t" << "wheelBrakes: " << bsm.brakeAppliedStatus.to_string() << std::endl;
	OS << "\t\t" << "traction: " << static_cast<unsigned int>(bsm.tractionControlStatus) << std::endl;
	OS << "\t\t" << "abs: " << static_cast<unsigned int>(bsm.absStatus) << std::endl;
	OS << "\t\t" << "scs: " << static_cast<unsigned int>(bsm.stabilityControlStatus) << std::endl;
	OS << "\t\t" << "brakeBoost: " << static_cast<unsigned int>(bsm.brakeBoostApplied) << std::endl;
	OS << "\t\t" << "auxBrakes: " << static_cast<unsigned int>(bsm.auxiliaryBrakeStatus) << std::endl;
	OS << "\t" << "VehicleSize" << std::endl;
	OS << "\t\t" << "width: " << bsm.vehWidth << std::endl;
	OS << "\t\t" << "length: " << bsm.vehLen << std::endl << std::endl;
}

void print_srmdata(std::ofstream& OS, const SRM_element_t& srm)
{
	OS << "SRM UPER (Unaligned Packed Encoding Rules) decoding result:" << std::endl << std::endl;
	OS << "minuteOfYear: " << srm.timeStampMinute << std::endl;
	OS << "msOfMinute: " << srm.timeStampSec << std::endl;
	OS << "sequenceNumber: " << static_cast<unsigned int>(srm.msgCnt) << std::endl;
	OS << "SignalRequestList: 1" << std::endl;
	OS << "\t" << "request" << std::endl;
	OS << "\t\t" << "id" << std::endl;
	OS << "\t\t\t" << "regionalId: " << srm.regionalId << std::endl;
	OS << "\t\t\t" << "intersectionId: " << srm.intId << std::endl;
	OS << "\t\t" << "requestID: " << static_cast<unsigned int>(srm.reqId) << std::endl;
	OS << "\t\t" << "requestType: " << static_cast<unsigned int>(srm.reqType) << std::endl;
	if (srm.inApprochId != 0)
		OS << "\t\t" << "inBoundLane(approach): " << static_cast<unsigned int>(srm.inApprochId) << std::endl;
	else
		OS << "\t\t" << "inBoundLane(lane): " << static_cast<unsigned int>(srm.inLaneId) << std::endl;
	if (srm.outApproachId != 0)
		OS << "\t\t" << "outBoundLane(approach): " << static_cast<unsigned int>(srm.outApproachId) << std::endl;
	else
		OS << "\t\t" << "outBoundLane(lane): " << static_cast<unsigned int>(srm.outLaneId) << std::endl;
	OS << "\t\t" << "ETAminute: " << srm.ETAminute << std::endl;
	OS << "\t\t" << "ETAsec: " << srm.ETAsec << std::endl;
	OS << "\t\t" << "duration: " << srm.duration << std::endl;
	OS << "\t" << "requestor" << std::endl;
	OS << "\t\t" << "VehicleID: " << srm.vehId << std::endl;
	OS << "\t\t" << "RequestorType" << std::endl;
	OS << "\t\t\t" << "BasicVehicleRole: " << static_cast<unsigned int>(srm.vehRole) << std::endl;
	OS << "\t\t\t" << "VehicleType: " << static_cast<unsigned int>(srm.vehType) << std::endl;
	OS << "\t\t" << "position" << std::endl;
	OS << "\t\t\t" << "Position3D: " << std::endl;
	OS << "\t\t\t\t" << "lat: " << std::fixed << std::setprecision(7) << DsrcConstants::damega2unit<int32_t>(srm.latitude) << std::endl;
	OS << "\t\t\t\t" << "long: " << std::fixed << std::setprecision(7) << DsrcConstants::damega2unit<int32_t>(srm.longitude) << std::endl;
	OS << "\t\t\t\t" << "elevation: " << std::fixed << std::setprecision(1) << DsrcConstants::deca2unit<int32_t>(srm.elevation) << std::endl;
	OS << "\t\t\t" << "heading: " << round(DsrcConstants::unit2heading<uint16_t>(srm.heading)) << std::endl;
	OS << "\t\t\t" << "TransmissionAndSpeed" << std::endl;
	OS << "\t\t\t\t" << "transmission: " << static_cast<unsigned int>(srm.transState) << std::endl;
	OS << "\t\t\t\t" << "speed: " << round(DsrcConstants::unit2kph<uint16_t>(srm.speed)) << std::endl << std::endl;
}

void print_spatdata(std::ofstream& OS, const SPAT_element_t& spat)
{
	OS << "SPaT UPER (Unaligned Packed Encoding Rules) decoding result:" << std::endl << std::endl;
	OS << "intersections: 1" << std::endl;
	OS << "\t" << "IntersectionState" << std::endl;
	OS << "\t\t" << "id" << std::endl;
	OS << "\t\t\t" << "regionalId: " << spat.regionalId << std::endl;
	OS << "\t\t\t" << "intersectionId: " << spat.id << std::endl;
	OS << "\t" << "msgCnt: " << static_cast<unsigned int>(spat.msgCnt) << std::endl;
	OS << "\t" << "minuteOfYear: " << spat.timeStampMinute << std::endl;
	OS << "\t" << "msOfMinute: " << spat.timeStampSec << std::endl;
	OS << "\t" << "status: " << spat.status.to_string() << std::endl;
	OS << "\t" << "phaseState" << std::endl;
	for (int i = 0; i < 8; i++)
	{
		if (spat.permittedPhases.test(i))
		{
			const auto& phaseState = spat.phaseState[i];
			OS << "\t\t" << "phase: " << (i+1) << std::endl;
			OS << "\t\t" << "currState: " << static_cast<unsigned int>(phaseState.currState) << std::endl;
			OS << "\t\t" << "startTime: " << phaseState.startTime << std::endl;
			OS << "\t\t" << "minEndTime: " << phaseState.minEndTime << std::endl;
			OS << "\t\t" << "maxEndTime: " << phaseState.maxEndTime << std::endl;
		}
	}
	OS << "\t" << "pedPhaseState" << std::endl;
	for (int i = 0; i < 8; i++)
	{
		if (spat.permittedPedPhases.test(i))
		{
			const auto& phaseState = spat.pedPhaseState[i];
			OS << "\t\t" << "phase: " << (i+1) << std::endl;
			OS << "\t\t" << "currState: " << static_cast<unsigned int>(phaseState.currState) << std::endl;
			OS << "\t\t" << "startTime: " << phaseState.startTime << std::endl;
			OS << "\t\t" << "minEndTime: " << phaseState.minEndTime << std::endl;
			OS << "\t\t" << "maxEndTime: " << phaseState.maxEndTime << std::endl;
		}
	}
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

void do_usage(const char* progname)
{
	std::cerr << "Usage" << progname << std::endl;
	std::cerr << "\t-s scenario: BSM, SRM, SPaT, SSM" << std::endl;
	std::cerr << "\t-? print this message" << std::endl;
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
	int option;
	std::string scenario;

	while ((option = getopt(argc, argv, "s:?")) != EOF)
	{
		switch(option)
		{
		case 's':
			scenario = std::string(optarg);
			break;
		case '?':
		default:
			do_usage(argv[0]);
			break;
		}
	}
	if (scenario.empty() || ((scenario.compare("BSM") != 0) && (scenario.compare("SRM") != 0)
			&& (scenario.compare("SPaT") != 0) && (scenario.compare("SSM") != 0)))
		do_usage(argv[0]);

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
	const uint32_t time2go = 30000;    /// 30 seconds
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
	std::string fout = "testDecoder_" + scenario + ".out";
	std::ofstream OS_OUT(fout);

	std::cout << "Test " << scenario << " encode and decode" << std::endl;

	if (scenario.compare("BSM") == 0)
	{	/// manual input bsmIn
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
			std::cout << "encode_bsm_payload succeed" << std::endl;
			/// decode BSM payload
			if ((AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0)
				&& (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_bsm))
			{
				BSM_element_t& bsmOut = dsrcFrameOut.bsm;
				std::cout << "decode_msgFrame for BSM succeed" << std::endl;
				OS_OUT << "Payload size " << payload_size << std::endl;
				logMsgHex(OS_OUT, &buf[0], payload_size);
				OS_OUT << std::endl;
				print_bsmdata(OS_OUT, bsmOut);
			}
			else
				std::cout << "Failed decode_msgFrame for BSM" << std::endl;
		}
		else
			std::cout << "Failed encode_msgFrame for BSM" << std::endl;
		std::cout << "Done test BSM encode and decode" << std::endl << std::endl;
	}
	else if (scenario.compare("SRM") == 0)
	{	/// manual input srmIn
		dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_srm;
		SRM_element_t& srmIn = dsrcFrameIn.srm;
		srmIn.timeStampMinute = minuteOfYear;
		srmIn.timeStampSec = msOfMinute;
		srmIn.msgCnt = 2;
		srmIn.regionalId = regionalId;
		srmIn.intId = intersectionId;
		srmIn.reqId = priorityLevel;
		srmIn.inApprochId = 0;
		srmIn.inLaneId = inLaneId;
		srmIn.outApproachId = 0;
		srmIn.outLaneId = outLaneId;
		srmIn.ETAsec    = static_cast<uint16_t>((ETAsec % 60000) & 0xFFFF);
		srmIn.ETAminute = static_cast<uint32_t>(minuteOfYear + std::floor(ETAsec / 60000));
		srmIn.duration  = duration;
		srmIn.vehId     = vehId;
		srmIn.latitude  = DsrcConstants::unit2damega<int32_t>(latitude);
		srmIn.longitude = DsrcConstants::unit2damega<int32_t>(longitude);
		srmIn.elevation = DsrcConstants::unit2deca<int32_t>(elevation);
		srmIn.heading   = DsrcConstants::heading2unit<uint16_t>(heading);
		srmIn.speed     = DsrcConstants::kph2unit<uint16_t>(speed);
		srmIn.reqType   = MsgEnum::requestType::priorityRequest;
		srmIn.vehRole   = MsgEnum::basicRole::transit;
		srmIn.vehType   = MsgEnum::vehicleType::bus;
		/// encode SRM payload
		size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);
		if (payload_size > 0)
		{
			std::cout << "encode_msgFrame for SRM succeed" << std::endl;
			/// decode SRM payload
			if ((AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0)
				&& (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_srm))
			{
				SRM_element_t& srmOut = dsrcFrameOut.srm;
				std::cout << "decode_msgFrame for SRM succeed" << std::endl;
				OS_OUT << "Payload size " << payload_size << std::endl;
				logMsgHex(OS_OUT, &buf[0], payload_size);
				OS_OUT << std::endl;
				print_srmdata(OS_OUT, srmOut);
			}
			else
				std::cout << "Failed decode_msgFrame for SRM" << std::endl;
		}
		else
			std::cout << "Failed encode_msgFrame for SRM" << std::endl;
		std::cout << "Done test SRM encode and decode" << std::endl << std::endl;
	}
	else if (scenario.compare("SPaT") == 0)
	{	/// manual input spatIn
		dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_spat;
		SPAT_element_t& spatIn = dsrcFrameIn.spat;
		spatIn.regionalId = regionalId;
		spatIn.id = intersectionId;
		spatIn.msgCnt = 3;
		spatIn.timeStampMinute = minuteOfYear;
		spatIn.timeStampSec = msOfMinute;
		spatIn.permittedPhases.set();          /// all 8 phases permitted
		for (size_t i = 0; i < 8; i++)
		{
			if (i % 2 == 1)
				spatIn.permittedPedPhases.set(i);  /// even phase # is pedestrian phase
		}
		uint16_t baseTime = 50;
		uint16_t stepTime = 50;
		for (auto& phaseState : spatIn.phaseState)
		{
			phaseState.currState = MsgEnum::phaseState::redLight;
			phaseState.startTime = baseTime;
			phaseState.minEndTime = (uint16_t)(phaseState.startTime + stepTime);
			phaseState.maxEndTime = (uint16_t)(phaseState.minEndTime + stepTime);
			baseTime = (uint16_t)(phaseState.minEndTime +  stepTime);
		}
		spatIn.phaseState[1].currState = MsgEnum::phaseState::protectedGreen;
		spatIn.phaseState[5].currState = MsgEnum::phaseState::protectedGreen;
		for (size_t i = 0; i < 8; i++)
		{
			if (i % 2 == 0)
				continue;
			auto& phaseState = spatIn.pedPhaseState[i];
			phaseState.currState = MsgEnum::phaseState::redLight;
			phaseState.startTime = baseTime;
			phaseState.minEndTime = (uint16_t)(phaseState.startTime + stepTime);
			phaseState.maxEndTime = (uint16_t)(phaseState.minEndTime + stepTime);
			baseTime = (uint16_t)(phaseState.minEndTime +  stepTime);
		}
		spatIn.pedPhaseState[1].currState = MsgEnum::phaseState::protectedYellow;
		spatIn.pedPhaseState[5].currState = MsgEnum::phaseState::protectedYellow;
		/// encode SPaT payload
		size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &buf[0], bufSize);
		if (payload_size > 0)
		{
			std::cout << "encode_spat_payload succeed" << std::endl;
			/// decode SPaT payload
			if ((AsnJ2735Lib::decode_msgFrame(&buf[0], payload_size, dsrcFrameOut) > 0)
				&& (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_spat))
			{
				SPAT_element_t& spatOut = dsrcFrameOut.spat;
				std::cout << "decode_msgFrame for SPaT succeed" << std::endl;
				OS_OUT << "Payload size " << payload_size << std::endl;
				logMsgHex(OS_OUT, &buf[0], payload_size);
				OS_OUT << std::endl;
				print_spatdata(OS_OUT, spatOut);
			}
			else
				std::cout << "Failed decode_msgFrame for SPaT" << std::endl;
		}
		else
			std::cout << "Failed encode_msgFrame for SPaT" << std::endl;
		std::cout << "Done test SPaT encode and decode" << std::endl << std::endl;
	}
	else if (scenario.compare("SSM") == 0)
	{	/// manual input ssmIn
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
	}

	std::cout << "Done test " << scenario << " encode and decode" << std::endl;
	OS_OUT.close();
	return(0);
}
