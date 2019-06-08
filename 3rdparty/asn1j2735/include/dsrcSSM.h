//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _DSRC_SSM_H
#define _DSRC_SSM_H

#include <cstdint>
#include <vector>
#include "msgEnum.h"

struct SignalRequetStatus_t
{
	uint32_t  vehId;
	uint8_t   reqId;
	uint8_t   sequenceNumber;    // msgCnt in SRM
	uint8_t   inApprochId;       // 0 = unknown
	uint8_t   inLaneId;          // 0 = unknown
	uint8_t   outApproachId;     // 0 = unknown
	uint8_t   outLaneId;         // 0 = unknown
	uint32_t  ETAminute;         // minute of the year
	uint16_t  ETAsec;            // millisecond of the minute
	uint16_t  duration;          // in milliseconds
	MsgEnum::basicRole     vehRole;
	MsgEnum::requestStatus status;
	void reset(void)
	{
		sequenceNumber = 0xFF;
		inApprochId    = 0;
		inLaneId       = 0;
		outApproachId  = 0;
		outLaneId      = 0;
		ETAminute      = MsgEnum::invalid_timeStampMinute;
		ETAsec         = 0xFFFF;
		duration       = 0xFFFF;
		vehRole        = MsgEnum::basicRole::unavailable;
		status         = MsgEnum::requestStatus::unavailable;
	};
};

struct SSM_element_t
{
	uint32_t  timeStampMinute;   //	 minute of the year, (0..527040), 527040 = invalid
	uint16_t  timeStampSec;      //  millisecond of the minute
	uint8_t   msgCnt;            // (0..127), set to 0xFF when EXCL
	uint8_t   updateCnt;         // (0..127), change whenever mpSignalRequetStatus has changed
	uint16_t  regionalId;
	uint16_t  id;                // intersection ID
	std::vector<SignalRequetStatus_t> mpSignalRequetStatus;  // maximum entries: 32
	void reset(void)
	{
		timeStampMinute = MsgEnum::invalid_timeStampMinute;
		msgCnt = 0xFF;
		regionalId = 0;
		mpSignalRequetStatus.clear();
	};
};

#endif
