//************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _DSRC_FRAME_H
#define _DSRC_FRAME_H

#include <cstdint>

#include "dsrcBSM.h"
#include "dsrcRTCM.h"
#include "dsrcSPAT.h"
#include "dsrcSRM.h"
#include "dsrcSSM.h"
#include "dsrcMapData.h"
#include "msgEnum.h"

struct Frame_element_t
{
	uint16_t dsrcMsgId;
	MapData_element_t mapData;
	SPAT_element_t    spat;
	BSM_element_t     bsm;
	RTCM_element_t    rtcm;
	SRM_element_t     srm;
	SSM_element_t     ssm;
	void reset(void)
	{
		dsrcMsgId = MsgEnum::DSRCmsgID_unknown;
		mapData.reset();
		spat.reset();
		bsm.reset();
		rtcm.reset();
		srm.reset();
		ssm.reset();
	};
};

#endif
