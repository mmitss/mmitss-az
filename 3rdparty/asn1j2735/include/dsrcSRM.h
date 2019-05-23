//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _DSRC_SRM_H
#define _DSRC_SRM_H

#include <cstdint>
#include "msgEnum.h"

struct SRM_element_t
{
	uint32_t  timeStampMinute; //  minute of the year
	uint16_t  timeStampSec;    //  millisecond of the current minute
	uint8_t   msgCnt;          // (0..127), set to 0xFF when EXCL
	// intersection data for request
	uint16_t  regionalId;
	uint16_t  intId;
	uint8_t   reqId;
	uint8_t   inApprochId;    // 0 = unknown
	uint8_t   inLaneId;       // 0 = unknown
	uint8_t   outApproachId;  // 0 = unknown
	uint8_t   outLaneId;      // 0 = unknown
	uint32_t  ETAminute;      // minute of the year
	uint16_t  ETAsec;         // millisecond of the minute
	uint16_t  duration;       // in milliseconds
	// vehicle data
	uint32_t  vehId;
	int32_t   latitude;       // in 1/10th micro degrees
	int32_t   longitude;      // in 1/10th micro degrees
	int32_t   elevation;      // in decimeters, -4096 = Unknown
	uint16_t  heading;        // LSB of 0.0125 degrees
	uint16_t  speed;          // units of 0.02 m/s, (0..8191), 0x1FFF(8191) = unavailable
	// enum
	MsgEnum::requestType  reqType;
	MsgEnum::basicRole    vehRole;
	MsgEnum::vehicleType  vehType;
	MsgEnum::transGear    transState;
	void reset(void)
	{
		timeStampMinute = MsgEnum::invalid_timeStampMinute;
		msgCnt          = 0xFF;
		regionalId      = 0;
		ETAminute       = MsgEnum::invalid_timeStampMinute;
		ETAsec          = 0xFFFF;
		duration        = 0xFFFF;
		inApprochId     = 0;
		inLaneId        = 0;
		outApproachId   = 0;
		outLaneId       = 0;
		elevation       = MsgEnum::unknown_elevation;
		speed           = MsgEnum::unknown_speed;
		transState      = MsgEnum::transGear::unavailable;
		reqType         = MsgEnum::requestType::reserved;
		vehRole         = MsgEnum::basicRole::unavailable;
		vehType         = MsgEnum::vehicleType::unavailable;
	};
};

#endif
