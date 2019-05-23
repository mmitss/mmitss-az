//************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _DSRC_MAPDATA_H
#define _DSRC_MAPDATA_H

#include <cstddef>
#include <cstdint>
#include <bitset>
#include <vector>

#include "msgEnum.h"

struct conn_element_t
{
	uint16_t  regionalId;
	uint16_t  intersectionId;
	uint8_t   laneId;
	MsgEnum::maneuverType laneManeuver;
};

struct node_element_t
{
		bool useXY;
	// unit of centimeters
	int32_t offset_x;
	int32_t offset_y;
	// in 1/10th micro degrees
	int32_t latitude;
	int32_t longitude;
};

struct lane_element_t
{ // an approach can have variable lane objectives.
	// order of lane array starts at the curb lane.
	// lanes within an approach can be controlled by different traffic lights
	// (e.g., left-turn, through, and right-turn movements on one approach).
	uint8_t id;     // starts from 1
	MsgEnum::laneType type;
	std::bitset<20> attributes;
		/*  vehicular traffic lane with bits as defined:
		 *    isVehicleRevocableLane                (0)
		 *    isVehicleFlyOverLane                  (1)
		 *    hovLaneUseOnly                        (2)
		 *    restrictedToBusUse                    (3)
		 *    restrictedToTaxiUse                   (4)
		 *    restrictedFromPublicUse               (5)
		 *    hasIRbeaconCoverage                   (6)
		 *    permissionOnRequest                   (7)
		 *    maneuverStraightAllowed               (8)
		 *    maneuverLeftAllowed                   (9)
		 *    maneuverRightAllowed                  (10)
		 *    maneuverUTurnAllowed                  (11)
		 *    maneuverLeftTurnOnRedAllowed          (12)
		 *    maneuverRightTurnOnRedAllowed         (13)
		 *    maneuverLaneChangeAllowed             (14)
		 *    maneuverNoStoppingAllowed             (15)
		 *    yieldAllwaysRequired                  (16)
		 *    goWithHalt                            (17)
		 *    caution                               (18)
		 *    reserved                              (19)
		 *
		 *  crosswalk with bits as defined:
		 *    crosswalkRevocableLane                (0)
		 *    bicyleUseAllowed                      (1)
		 *    isXwalkFlyOverLane                    (2)
		 *    fixedCycleTime                        (3)
		 *    biDirectionalCycleTimes               (4)
		 *    hasPushToWalkButton                   (5)
		 *    audioSupport                          (6)
		 *    rfSignalRequestPresent                (7)
		 *    unsignalizedSegmentsPresent           (8)
		 *    reserved                              (9-19)
		 */
	uint16_t width;        // in centimeter
	uint8_t  controlPhase; // 1 - 8
	std::vector<conn_element_t> mpConnectTo;
	std::vector<node_element_t> mpNodes;
};

struct approach_element_t
{ // an regular intersection has 12 approach objects: 8 for motor vehicles and 4 crosswalks.
	// an approach with 0 number of lanes does not physically exist, and is not sent.
	// order of approach array starts at westbound inbound and goes clockwise to northbound outbound.
	uint8_t  id;           // starts from 1
	uint16_t speed_limit;  // in units of 0.02 m/s
	MsgEnum::approachType type;
	std::vector<lane_element_t> mpLanes;
};

struct geo_refPoint_t
{
	int32_t latitude;   // in 1/10th micro degrees
	int32_t longitude;  // in 1/10th micro degrees
	int32_t elevation;  // in decimeters
};

struct MapData_element_t
{
	bool isSingleFrame;
	uint16_t regionalId;
	uint16_t id;
	uint8_t  mapVersion;
	std::bitset<8> attributes;
		/*  with bits as defined:
		 *    Elevation data is included            (0)
		 *    Geometric data is included            (1)
		 *    Speed limit data is included          (2)
		 *    Navigational data is included         (3)
		 *    Reserved                              (4-7)
		 */
	geo_refPoint_t geoRef;
	std::vector<uint16_t> speeds;
	std::vector<approach_element_t> mpApproaches;
	void reset(void)
	{
		regionalId = 0;
		attributes.reset();
		speeds.clear();
		mpApproaches.clear();
	};
};

#endif
