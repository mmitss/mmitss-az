//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _DSRCMSGENUM_H
#define _DSRCMSGENUM_H

#include <cstdint>
#include <bitset>
#include <string>

namespace MsgEnum
{
	static const uint16_t DSRCmsgID_unknown  = 0;
	static const uint16_t DSRCmsgID_map  = 18;
	static const uint16_t DSRCmsgID_spat = 19;
	static const uint16_t DSRCmsgID_bsm  = 20;
	static const uint16_t DSRCmsgID_rtcm = 28;
	static const uint16_t DSRCmsgID_srm  = 29;
	static const uint16_t DSRCmsgID_ssm  = 30;

	static const uint8_t  patternFlashing = 0xFE;
	static const uint8_t  patternFree = 0xFF;
	static const uint64_t mapInterval = 1000;  /// in milliseconds
	static const uint64_t ssmInterval = 1000;  /// in milliseconds
	static const uint64_t isdInterval = 2000;  /// in milliseconds
	static const uint32_t invalid_timeStampMinute = 527040;
	static const int32_t  unknown_elevation = -4096;
	static const uint16_t unknown_timeDetail = 36001;
	static const uint16_t unknown_speed = 8191;

	static const uint8_t  rswz_roadwork = 3;

	enum class approachType    : uint8_t {inbound = 1, outbound, crosswalk = 4};
	enum class laneType        : uint8_t {traffic = 1, crosswalk = 4};
	enum class maneuverType    : uint8_t {unavailable, uTurn, leftTurn, rightTurn, straightAhead, straight};
	enum class polygonType     : uint8_t {colinear, concave, convex};
	enum class phaseColor      : uint8_t {dark, green, yellow, red, flashingRed};
	enum class phaseState      : uint8_t {unavailable, dark, flashingRed, redLight, preMovement,
			permissiveGreen, protectedGreen, permissiveYellow, protectedYellow, flashingYellow};
	enum class requestType     : uint8_t {reserved, priorityRequest, requestUpdate, priorityCancellation};
	enum class requestStatus   : uint8_t {unavailable, requested, processing, watchOtherTraffic, granted,
			rejected, maxPresence, reserviceLocked};
	enum class basicRole       : uint8_t {unavailable = 8, truck, motorcycle, roadsideSource, police, fire, ambulance,
			DOT, transit, slowMoving, stopNgo, cyclist, pedestrian, nonMotorized, military};
	enum class vehicleType     : uint8_t {unavailable, notApply, special, moto, car, carOther, bus, axleCnt2, axleCnt3, axleCnt4,
			axleCnt4Trailer, axleCnt5Trailer, axleCnt6Trailer, axleCnt5MultiTrailer, axleCnt6MultiTrailer, axleCnt7MultiTrailer};
	enum class engageStatus    : uint8_t {unavailable, off, on, engaged};
	enum class transGear       : uint8_t {neutral, park, forward, reverse, unavailable = 7};
	enum class mapLocType      : uint8_t {outside, insideIntersectionBox, onInbound, atIntersectionBox, onOutbound};
	enum class laneLocType     : uint8_t {outside, approaching, inside, leaving};
	enum class controlMode     : uint8_t {unavailable, flashing, preemption, runningFree, coordination};
	enum class phaseCallType   : uint8_t {none, vehicle, ped, bike};
	enum class phaseRecallType : uint8_t {none, minimum, maximum, ped, bike};
	enum class softCallObj     : uint8_t {none, ped, vehicle, priority};
	enum class softCallType    : uint8_t {none, call, extension, cancel};

	static inline MsgEnum::phaseColor getLightColor(const MsgEnum::phaseState& state)
	{
		if (state == MsgEnum::phaseState::flashingRed)
			return(MsgEnum::phaseColor::flashingRed);
		if (state == MsgEnum::phaseState::redLight)
			return(MsgEnum::phaseColor::red);
		if ((state == MsgEnum::phaseState::preMovement) || (state == MsgEnum::phaseState::permissiveGreen) || (state == MsgEnum::phaseState::protectedGreen))
			return(MsgEnum::phaseColor::green);
		if ((state == MsgEnum::phaseState::permissiveYellow) || (state == MsgEnum::phaseState::protectedYellow) || (state == MsgEnum::phaseState::flashingYellow))
			return(MsgEnum::phaseColor::yellow);
		return(MsgEnum::phaseColor::dark);
	};
}

#endif
