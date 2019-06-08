//************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _DSRC_RTCM_H
#define _DSRC_RTCM_H

#include <cstdint>
#include <vector>
#include "msgEnum.h"

struct RTCM_element_t
{
	uint8_t  msgCnt;            // (0..127)
	uint8_t  rev;               // (0..3)
	uint32_t timeStampMinute;   // minute of the year
	std::vector<uint8_t> payload;
	void reset(void)
	{
		timeStampMinute = MsgEnum::invalid_timeStampMinute;
		payload.clear();
	};
};

#endif
