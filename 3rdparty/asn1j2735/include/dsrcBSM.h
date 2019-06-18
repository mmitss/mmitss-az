//************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _DSRC_BSM_H
#define _DSRC_BSM_H

#include <cstdint>
#include <bitset>
#include "msgEnum.h"

struct BSM_element_t
{ // BSMcoreData
	uint8_t   msgCnt;          // INTEGER (0..127)
	uint32_t  id;              // TemporaryID
	uint16_t  timeStampSec;    // millisecond of the minute
	int32_t   latitude;        // in 1/10th micro degrees
	int32_t   longitude;       // in 1/10th micro degrees
	int32_t   elevation;       // in decimeters, -4096 = Unknown
	uint8_t   semiMajor;       // LSB = .05m, 0xFF = unavailable
	uint8_t   semiMinor;       // LSB = .05m, 0xFF = unavailable
	uint16_t  orientation;     // LSB units of 360/65535 deg  = 0.0054932479, 0xFFFF = unavailable
	uint16_t  vehLen;          // LSB units of 1 cm, (0.. 4095)
	uint16_t  vehWidth;        // LSB units of 1 cm, (0..1023)
	uint16_t  speed;           // units of 0.02 m/s, (0..8191), 0x1FFF(8191) = unavailable
	uint16_t  heading;         // LSB of 0.0125 degrees
	int16_t   accelLon;        // LSB units are 0.01 m/s^2, (-2000..2001), 2001 = Unavailable
	int16_t   accelLat;        // LSB units are 0.01 m/s^2, (-2000..2001), 2001 = Unavailable
	int8_t    accelVert;       // LSB units of 0.02G, (-127..127), -127 = unavailable
	int8_t    steeringAngle;   // LSB units of 1.5 degrees, (-126..127), 0x7F(127) = unavailable
	int16_t   yawRate;         // LSB units of 0.01 degrees per second, (-32767..32767),
	std::bitset<5> brakeAppliedStatus;
		/*  with bit defined as
		 *    unavailable (0) -- When set, the brake applied status is unavailable
		 *    leftFront   (1) -- Left Front Active
		 *    leftRear    (2) -- Left Rear Active
		 *    rightFront  (3) -- Right Front Active
		 *    rightRear   (4) -- Right Rear Active
		 */
	MsgEnum::transGear     transState;
	MsgEnum::engageStatus  tractionControlStatus;
	MsgEnum::engageStatus  absStatus;
	MsgEnum::engageStatus  stabilityControlStatus;
	MsgEnum::engageStatus  brakeBoostApplied;
	MsgEnum::engageStatus  auxiliaryBrakeStatus;
	void reset(void)
	{
		elevation              = MsgEnum::unknown_elevation;
		semiMajor              = 0xFF;
		semiMinor              = 0xFF;
		orientation            = 0xFFFF;
		speed                  = MsgEnum::unknown_speed;
		accelLon               = 2001;
		accelLat               = 2001;
		accelVert              = -127;
		steeringAngle          = 0x7F;
		brakeAppliedStatus.set(0);
		transState             = MsgEnum::transGear::unavailable;
		tractionControlStatus  = MsgEnum::engageStatus::unavailable;
		absStatus              = MsgEnum::engageStatus::unavailable;
		stabilityControlStatus = MsgEnum::engageStatus::unavailable;
		brakeBoostApplied      = MsgEnum::engageStatus::unavailable;
		auxiliaryBrakeStatus   = MsgEnum::engageStatus::unavailable;
	};
};

#endif
