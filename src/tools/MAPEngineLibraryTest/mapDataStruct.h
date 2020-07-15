//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _MAP_DATA_STRUCT_H
#define _MAP_DATA_STRUCT_H

#include <cstdint>
#include <bitset>
#include <string>
#include <vector>

#include "msgEnum.h"
#include "geoUtils.h"

namespace NmapData
{
	static const double lowSpeedThreshold = 0.2;    // in m/s; speed lower than this heading is questionable
	static const double headingErrorBoundNormal = 45.0;     // in degree
	static const double headingErrorBoundLowSpeed = 200.0;  // in degree
	static const double laneWidthRatio = 1.5;       // for geofencing

	struct ConnectStruct
	{
		uint16_t  regionalId;
		uint16_t  intersectionId;
		uint8_t   laneId;
		MsgEnum::maneuverType laneManeuver;
	};

	struct NodeStruct
	{ // A lane is described by a sequence of nodes along the center of the lane.
	  // Order of nodes starts from the intersection box and moves away from the intersection box.
		GeoUtils::geoRefPoint_t geoNode;
		GeoUtils::point2D_t ptNode;  // in centimeter, reference to intersection reference point
		uint32_t dTo1stNode;         // in centimeter, reference to the first node on the same lane
		uint16_t heading;            // in decidegree
	};

	struct LaneStruct
	{ // An approach consists of multiple lane objectives, including traffic lanes and crosswalks.
		// Order of lane array starts at the curb lane and moves towards the center of the road.
		// Lanes within an approach could be controlled by different traffic lights
		// (e.g., left-turn, through, and right-turn movements).
		uint8_t   id;                // global laneId at an intersection
		MsgEnum::laneType type;      // traffic vs. crosswalk
		std::bitset<20> attributes;  // traffic lane vs. crosswalk
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
		uint16_t width;              // in centimeter
		uint8_t  controlPhase;       // 1 - 8
		size_t   numpoints;          // sizeof(mpNodes)
		std::vector<NmapData::ConnectStruct> mpConnectTo;
		std::vector<NmapData::NodeStruct> mpNodes;
	};

	struct ApproachStruct
	{ // A typical intersection has 12 approach objects: 8 for motor vehicles and 4 crosswalks.
		// Order of approach array starts at west-inbound and goes clockwise to north-outbound.
		uint8_t id;                  // global approachId at an intersection
		uint8_t speed_limit;         // in mph
		MsgEnum::approachType type;  // motor vehicles vs. crosswalk
		std::vector<NmapData::LaneStruct> mpLanes;
		std::vector<GeoUtils::point2D_t> mpPolygon;
		MsgEnum::polygonType mpPolygonType;
		uint32_t mindist2intsectionCentralLine; // in centimeter
	};

	struct IntersectionStruct
	{
		uint8_t       mapVersion;
		std::string   name;
		uint16_t      regionalId;
		uint16_t      id;
		std::bitset<8> attributes;
			/*  with bits as defined:
			 *    Elevation data is included            (0)
			 *    Geometric data is included            (1)
			 *    Speed limit data is included          (2)
			 *    Navigational data is included         (3)
			 *    Reserved                              (4-7)
			 */
		GeoUtils::geoRefPoint_t geoRef;
		GeoUtils::enuCoord_t    enuCoord;
		uint32_t  radius;             // in centimeter
		std::vector<uint8_t>  speeds; // in mph
		std::vector<NmapData::ApproachStruct> mpApproaches;
		std::vector<GeoUtils::point2D_t> mpPolygon;
		MsgEnum::polygonType  mpPolygonType;
		std::vector<uint8_t>  mapPayload;
		std::vector<uint32_t> mpConnIntersections;
	};
};

#endif
