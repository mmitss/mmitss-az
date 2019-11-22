//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _GEO_UTILS_H
#define _GEO_UTILS_H

#include <bitset>
#include <cmath>
#include <cstdint>
#include <vector>

#include "dsrcConsts.h"
#include "msgEnum.h"

namespace GeoUtils
{
	struct geoPoint_t
	{
		double latitude;    // in degree
		double longitude;   // in degree
		double elevation;   // in meters
	};

	struct geoRefPoint_t
	{
		int32_t latitude;   // in 1/10th micro degrees
		int32_t longitude;  // in 1/10th micro degrees
		int32_t elevation;  // in decimeters
		bool operator==(const GeoUtils::geoRefPoint_t& p) const
		{ // node elevation is the same as intersection ref elevation
			return ((latitude == p.latitude) && (longitude == p.longitude));
		};
	};

	struct point3D_t
	{ // unit of meters
		double x;
		double y;
		double z;
	};

	struct transMatrix_t
	{
		double dSinLat;
		double dCosLat;
		double dSinLong;
		double dCosLong;
	};

	struct enuCoord_t
	{
		GeoUtils::point3D_t     pointECEF;        // ENU origin reference in ECEF
		GeoUtils::transMatrix_t transMatrix;
	};

	struct point2D_t
	{ // unit of centimeters
		int32_t x;
		int32_t y;
		bool operator==(const GeoUtils::point2D_t& p) const
		{
			return ((x == p.x) && (y == p.y));
		};
		bool operator<(const GeoUtils::point2D_t& p) const
		{
			return (x < p.x || (x == p.x && y < p.y));
		};
		uint32_t length() const
		{
			return (static_cast<uint32_t>(round(std::sqrt(x  * x + y * y))));
		};
		uint32_t distance2pt(const GeoUtils::point2D_t& p) const
		{
			return (static_cast<uint32_t>(round(std::sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y)))));
		};
		uint16_t direction2pt(const GeoUtils::point2D_t& p) const
		{
			double heading = DsrcConstants::rad2deg(atan2(p.x - x, p.y - y)); // [-180..180] degree, relative to true north
			if (heading < 0)
				heading += 360.0; // [0..360]
			return (DsrcConstants::unit2deca<uint16_t>(heading));
		}
	};

	struct vector2D_t
	{
		int32_t X;
		int32_t Y;
		bool operator==(const GeoUtils::vector2D_t& P) const
		{
			return ((X == P.X) && (Y == P.Y));
		};
		void set(const GeoUtils::point2D_t& startPoint, const GeoUtils::point2D_t& endPoint)
		{
			X = endPoint.x - startPoint.x;
			Y = endPoint.y - startPoint.y;
		};
	};

	struct motion_t
	{
		double speed;   // in m/s
		double heading; // in degree, (0..360)
	};

	struct projection_t
	{
		double t;       // along the line (unit-less)
		double d;       // distance away from the line, positive means on the right-side of travel
		double length;  // length of the line segment
		void reset(void)
		{
			t = 0.0;
			d = 0.0;
			length = 0.0;
		}
	};

	struct laneProjection_t
	{
		uint8_t nodeIndex;
		GeoUtils::projection_t proj2segment;
		void reset(void)
		{
			nodeIndex = 0;
			proj2segment.reset();
		}
	};

	struct intersectionTracking_t
	{
		MsgEnum::mapLocType vehicleIntersectionStatus;
		uint8_t  intersectionIndex;  // meaningful when vehicleIntersectionStatus != outside
		uint8_t  approachIndex;      // meaningful when vehicleIntersectionStatus != outside & insideIntersectionBox
		uint8_t  laneIndex;          // meaningful when vehicleIntersectionStatus != outside & insideIntersectionBox
		bool operator==(const GeoUtils::intersectionTracking_t& p) const
		{
			return ((vehicleIntersectionStatus == p.vehicleIntersectionStatus)
				&& (intersectionIndex == p.intersectionIndex)
				&& (approachIndex == p.approachIndex)
				&& (laneIndex == p.laneIndex));
		};
		void reset(void)
		{
			vehicleIntersectionStatus = MsgEnum::mapLocType::outside;
			intersectionIndex = 0;
			approachIndex = 0;
			laneIndex = 0;
		};
	};

	struct laneTracking_t
	{
		MsgEnum::laneLocType vehicleLaneStatus;
		GeoUtils::laneProjection_t laneProj;
	};

	struct vehicleTracking_t
	{
		GeoUtils::intersectionTracking_t  intsectionTrackingState;
		GeoUtils::laneProjection_t        laneProj;
		void reset(void)
		{
			intsectionTrackingState.reset();
			laneProj.reset();
		};
	};

	struct dist2go_t
	{ // unit of meters
		double distLong;    // dist2stopbar when approaching (positive) or inside intersection box (negative);
												// dist2entrance when leaving the intersection (positive)
		double distLat;     // distance away from lane center, positive/negative value means on the right/left side of travel
		void reset(void)
		{
			distLong = 0.0;
			distLat = 0.0;
		};
	};

	struct connectTo_t
	{
		uint16_t  regionalId;
		uint16_t  intersectionId;
		uint8_t   laneId;
		MsgEnum::maneuverType laneManeuver;
		void reset(void)
		{
			regionalId = 0;
			intersectionId = 0;
			laneId = 0;
			laneManeuver = MsgEnum::maneuverType::unavailable;
		}
	};

	struct locationAware_t
	{
		uint16_t  regionalId;
		uint16_t  intersectionId;
		uint8_t   laneId;
		uint8_t   controlPhase;
		double    speed_limit;  // in mps
		std::bitset<4> maneuvers;
		GeoUtils::dist2go_t  dist2go;
		std::vector<GeoUtils::connectTo_t> connect2go;
		void reset(void)
		{
			regionalId = 0;
			intersectionId = 0;
			laneId = 0;
			controlPhase = 0;
			speed_limit = 0.0;
			maneuvers.reset();
			dist2go.reset();
			connect2go.clear();
		};
	};

	struct signalAware_t
	{
		MsgEnum::phaseColor currColor;
		MsgEnum::phaseState currState;
		uint16_t startTime;    // when this phase 1st started, tenths of a second in the current or next hour
		uint16_t minEndTime;   // expected shortest end time, tenths of a second in the current or next hour
		uint16_t maxEndTime;   // expected longest end time, tenths of a second in the current or next hour
		void reset(void)
		{
			currColor  = MsgEnum::phaseColor::dark;
			currState  = MsgEnum::phaseState::unavailable;
			startTime  = MsgEnum::unknown_timeDetail;
			minEndTime = MsgEnum::unknown_timeDetail;
			maxEndTime = MsgEnum::unknown_timeDetail;
		};
	};

	struct connectedVehicle_t
	{
		uint64_t msec;
		uint32_t id;
		bool isVehicleInMap;
		GeoUtils::geoPoint_t geoPoint;
		GeoUtils::motion_t   motionState;
		GeoUtils::vehicleTracking_t vehicleTrackingState;
		GeoUtils::locationAware_t   vehicleLocationAware;
		GeoUtils::signalAware_t     vehicleSignalAware;
		void reset(void)
		{
			isVehicleInMap = false;
			vehicleTrackingState.reset();
			vehicleLocationAware.reset();
			vehicleSignalAware.reset();
		};
	};

	// methods
	void geoPoint2geoRefPoint(const GeoUtils::geoPoint_t& geoPoint, GeoUtils::geoRefPoint_t& geoRef);
	void geoRefPoint2geoPoint(const GeoUtils::geoRefPoint_t& geoRef, GeoUtils::geoPoint_t& geoPoint);
	void setEnuCoord(const GeoUtils::geoPoint_t& geoPoint, GeoUtils::enuCoord_t& enuCoord);
	void setEnuCoord(const GeoUtils::geoRefPoint_t& geoRef, GeoUtils::enuCoord_t& enuCoord);
	void lla2ecef(const GeoUtils::geoPoint_t& geoPoint, GeoUtils::point3D_t& ptECEF);
	void ecef2enu(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point3D_t& ptECEF, GeoUtils::point3D_t& ptENU);
	void lla2enu(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::geoPoint_t& geoPoint, GeoUtils::point3D_t& ptENU);
	void lla2enu(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::geoPoint_t& geoPoint, GeoUtils::point2D_t& ptENU);
	void lla2enu(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::geoRefPoint_t& geoRef, GeoUtils::point2D_t& ptENU);
	void enu2ecef(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point3D_t& ptENU, GeoUtils::point3D_t& ptECEF);
	void enu2ecef(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point2D_t& ptENU, GeoUtils::point3D_t& ptECEF);
	void ecef2lla(const GeoUtils::point3D_t& ptECEF, GeoUtils::geoPoint_t& geoPoint);
	void enu2lla(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point3D_t& ptENU, GeoUtils::geoPoint_t& geoPoint);
	void enu2lla(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point2D_t& ptENU, GeoUtils::geoPoint_t& geoPoint);
	void enu2lla(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point2D_t& ptENU, GeoUtils::geoRefPoint_t& geoRef);
	double distlla2lla(const GeoUtils::geoPoint_t& p1,const GeoUtils::geoPoint_t& p2);
	long dotProduct(const GeoUtils::vector2D_t& v1, const GeoUtils::vector2D_t& v2);
	long crossProduct(const GeoUtils::vector2D_t& v1, const GeoUtils::vector2D_t& v2);
	long cross(const GeoUtils::point2D_t& O, const GeoUtils::point2D_t& A, const GeoUtils::point2D_t& B);
	void projectPt2Line(const GeoUtils::point2D_t& startPoint, const GeoUtils::point2D_t& endPoint,
		const GeoUtils::point2D_t& pt, GeoUtils::projection_t& proj2line);
	MsgEnum::polygonType convexcave(const std::vector<GeoUtils::point2D_t>& p);
	bool isPointInsidePolygon(const std::vector<GeoUtils::point2D_t>& polygon, const GeoUtils::point2D_t& waypoint);
	int isLeft(const GeoUtils::point2D_t& p0,const GeoUtils::point2D_t& p1,const GeoUtils::point2D_t& p2);
	std::vector<GeoUtils::point2D_t> convexHullAndrew(std::vector<GeoUtils::point2D_t>& P);
	uint16_t getTime2Go(const double& dist2go, const double& speed_1, const double speed_2, const double& alpha);
};

#endif
