//************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _DSRC_SPAT_H
#define _DSRC_SPAT_H

#include <cstdint>
#include <bitset>
#include "msgEnum.h"

struct PhaseState_element_t
{
	MsgEnum::phaseState  currState;
	// following time marks are in tenths of a second in the current or next hour
	uint16_t startTime;          // when this phase 1st started
	uint16_t minEndTime;         // expected shortest end time
	uint16_t maxEndTime;         // expected longest end time
	void reset(void)
	{
		currState  = MsgEnum::phaseState::unavailable;
		startTime  = MsgEnum::unknown_timeDetail;
		minEndTime = MsgEnum::unknown_timeDetail;
		maxEndTime = MsgEnum::unknown_timeDetail;
	};
};

struct SPAT_element_t
{
	uint16_t regionalId;
	uint16_t id;                // intersection ID
	uint8_t  msgCnt;            // (0..127)
	uint32_t timeStampMinute;   // minute of the year
	uint16_t timeStampSec;      // millisecond of the minute
	std::bitset<8>  permittedPhases;
	std::bitset<8>  permittedPedPhases;
	std::bitset<16> status;
		/*  general status of the controller, with bits as defined:
		 *    manualControlIsEnabled                (0)
		 *    stopTimeIsActivated                   (1)
		 *    failureFlash                          (2)
		 *    preemptIsActive                       (3)
		 *    signalPriorityIsActive                (4)
		 *    fixedTimeOperation                    (5)
		 *    trafficDependentOperation             (6)
		 *    standbyOperation                      (7)
		 *    failureMode                           (8)
		 *    off                                   (9)
		 *    recentMAPmessageUpdate                (10)
		 *    recentChangeInMAPassignedLanesIDsUsed (11)
		 *    noValidMAPisAvailableAtThisTime       (12)
		 *    noValidSPATisAvailableAtThisTime      (13)
		 *    reserved                              (14,15)
		 */
	PhaseState_element_t phaseState[8];
	PhaseState_element_t pedPhaseState[8];
	void reset(void)
	{
		regionalId = 0;
		timeStampMinute = MsgEnum::invalid_timeStampMinute;
		timeStampSec    = 0xFFFF;
		permittedPhases.reset();
		permittedPedPhases.reset();
		for (int i = 0; i < 8; i++)
		{
			phaseState[i].reset();
			pedPhaseState[i].reset();
		}
	};
};

#endif
