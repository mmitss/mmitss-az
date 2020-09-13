//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#include <algorithm>

#include "dsrcConsts.h"
#include "geoUtils.h"

void GeoUtils::geoPoint2geoRefPoint(const GeoUtils::geoPoint_t& geoPoint, GeoUtils::geoRefPoint_t & geoRef)
{
	geoRef.latitude  = DsrcConstants::unit2damega<int32_t>(geoPoint.latitude);
	geoRef.longitude = DsrcConstants::unit2damega<int32_t>(geoPoint.longitude);
	geoRef.elevation = DsrcConstants::unit2deca<int32_t>(geoPoint.elevation);
}

void GeoUtils::geoRefPoint2geoPoint(const GeoUtils::geoRefPoint_t & geoRef, GeoUtils::geoPoint_t& geoPoint)
{
	geoPoint.latitude  = DsrcConstants::damega2unit<int32_t>(geoRef.latitude);
	geoPoint.longitude = DsrcConstants::damega2unit<int32_t>(geoRef.longitude);
	geoPoint.elevation = DsrcConstants::deca2unit<int32_t>(geoRef.elevation);
}

void GeoUtils::setEnuCoord(const GeoUtils::geoPoint_t& geoPoint, GeoUtils::enuCoord_t& enuCoord)
{
	double latitude_r = DsrcConstants::deg2rad(geoPoint.latitude);
	double longitude_r = DsrcConstants::deg2rad(geoPoint.longitude);
	enuCoord.transMatrix.dSinLat  = std::sin(latitude_r);
	enuCoord.transMatrix.dCosLat  = std::cos(latitude_r);
	enuCoord.transMatrix.dSinLong = std::sin(longitude_r);
	enuCoord.transMatrix.dCosLong = std::cos(longitude_r);
	// ENU origin reference at ECEF
	GeoUtils::lla2ecef(geoPoint,enuCoord.pointECEF);
}

void GeoUtils::setEnuCoord(const GeoUtils::geoRefPoint_t& geoRef, GeoUtils::enuCoord_t& enuCoord)
{
	GeoUtils::geoPoint_t geoPoint;
	GeoUtils::geoRefPoint2geoPoint(geoRef, geoPoint);
	GeoUtils::setEnuCoord(geoPoint, enuCoord);
}

void GeoUtils::lla2ecef(const GeoUtils::geoPoint_t& geoPoint, GeoUtils::point3D_t& ptECEF)
{ //convert degrees to radians
	double latitude_r = DsrcConstants::deg2rad(geoPoint.latitude);
	double longitude_r = DsrcConstants::deg2rad(geoPoint.longitude);
	// Radius of Curvature
	double N = DsrcConstants::ellipsoid_a / std::sqrt(1.0 - std::pow(DsrcConstants::ellipsoid_e * std::sin(latitude_r), 2));
	// ECEF coordinates
	ptECEF.x = (N + geoPoint.elevation) * std::cos(latitude_r) * std::cos(longitude_r);
	ptECEF.y = (N + geoPoint.elevation) * std::cos(latitude_r) * std::sin(longitude_r);
	ptECEF.z = (N * (1 - std::pow(DsrcConstants::ellipsoid_e, 2)) + geoPoint.elevation) * std::sin(latitude_r);
}

void GeoUtils::ecef2enu(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point3D_t& ptECEF, GeoUtils::point3D_t& ptENU)
{
	ptENU.x = (-enuCoord.transMatrix.dSinLong) * (ptECEF.x - enuCoord.pointECEF.x)
		+ (enuCoord.transMatrix.dCosLong) * (ptECEF.y - enuCoord.pointECEF.y);
	ptENU.y = (-enuCoord.transMatrix.dSinLat * enuCoord.transMatrix.dCosLong) * (ptECEF.x - enuCoord.pointECEF.x)
		+ (-enuCoord.transMatrix.dSinLat * enuCoord.transMatrix.dSinLong) * (ptECEF.y - enuCoord.pointECEF.y)
		+ (enuCoord.transMatrix.dCosLat) * (ptECEF.z - enuCoord.pointECEF.z);
	ptENU.z = (enuCoord.transMatrix.dCosLat * enuCoord.transMatrix.dCosLong) * (ptECEF.x - enuCoord.pointECEF.x)
		+ (enuCoord.transMatrix.dCosLat * enuCoord.transMatrix.dSinLong) * (ptECEF.y - enuCoord.pointECEF.y)
		+ (enuCoord.transMatrix.dSinLong) * (ptECEF.z - enuCoord.pointECEF.z);
}

void GeoUtils::lla2enu(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::geoPoint_t& geoPoint, GeoUtils::point3D_t& ptENU)
{
	GeoUtils::point3D_t ptECEF;
	GeoUtils::lla2ecef(geoPoint,ptECEF);
	GeoUtils::ecef2enu(enuCoord,ptECEF,ptENU);
}

void GeoUtils::lla2enu(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::geoPoint_t& geoPoint, GeoUtils::point2D_t& ptENU)
{
	GeoUtils::point3D_t pt3DENU;
	GeoUtils::lla2enu(enuCoord,geoPoint,pt3DENU);
	ptENU.x = DsrcConstants::unit2hecto<int32_t>(pt3DENU.x);
	ptENU.y = DsrcConstants::unit2hecto<int32_t>(pt3DENU.y);
}

void GeoUtils::lla2enu(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::geoRefPoint_t& geoRef, GeoUtils::point2D_t& ptENU)
{
	GeoUtils::geoPoint_t geoPoint;
	GeoUtils::geoRefPoint2geoPoint(geoRef,geoPoint);
	GeoUtils::lla2enu(enuCoord,geoPoint,ptENU);
}

void GeoUtils::enu2ecef(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point3D_t& ptENU, GeoUtils::point3D_t& ptECEF)
{
	ptECEF.x = enuCoord.pointECEF.x + (-enuCoord.transMatrix.dSinLong) * ptENU.x
		+ (-enuCoord.transMatrix.dSinLat * enuCoord.transMatrix.dCosLong) * ptENU.y
		+ (enuCoord.transMatrix.dCosLat * enuCoord.transMatrix.dCosLong) * ptENU.z;
	ptECEF.y = enuCoord.pointECEF.y + (enuCoord.transMatrix.dCosLong) * ptENU.x
		+ (-enuCoord.transMatrix.dSinLat * enuCoord.transMatrix.dSinLong) * ptENU.y
		+ (enuCoord.transMatrix.dCosLat * enuCoord.transMatrix.dSinLong) * ptENU.z;
	ptECEF.z = enuCoord.pointECEF.z + (enuCoord.transMatrix.dCosLat) * ptENU.y
		+ (enuCoord.transMatrix.dSinLat) * ptENU.z;
}

void GeoUtils::enu2ecef(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point2D_t& ptENU, GeoUtils::point3D_t& ptECEF)
{
	point3D_t pt;
	pt.x = DsrcConstants::hecto2unit<int32_t>(ptENU.x);
	pt.y = DsrcConstants::hecto2unit<int32_t>(ptENU.y);
	pt.z = 0.0;
	GeoUtils::enu2ecef(enuCoord,pt,ptECEF);
}

void GeoUtils::ecef2lla(const GeoUtils::point3D_t& ptECEF, GeoUtils::geoPoint_t& geoPoint)
{ // initial
	double p = std::sqrt(std::pow(ptECEF.x,2) + std::pow(ptECEF.y,2));
	double elevation = 0;
	double latitude = atan( ptECEF.z / (p * (1.0 - std::pow(DsrcConstants::ellipsoid_e,2))));
	double prev_latitude = latitude;

	for (int i=0; i<10; i++)
	{
		double N = DsrcConstants::ellipsoid_a / std::sqrt(1.0 - (std::pow(DsrcConstants::ellipsoid_e,2) * std::pow(std::sin(prev_latitude),2)));
		elevation = p / std::cos(prev_latitude) - N ;
		latitude = atan( ptECEF.z / (p * (1.0 - std::pow(DsrcConstants::ellipsoid_e,2) * N / (N + elevation))));
		if (std::abs(latitude - prev_latitude) < 1.e-8)
			break;
		else
			prev_latitude = latitude;
	}
	geoPoint.latitude = DsrcConstants::rad2deg(latitude);
	geoPoint.longitude = DsrcConstants::rad2deg(atan2(ptECEF.y, ptECEF.x));
	geoPoint.elevation = elevation;
}

void GeoUtils::enu2lla(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point3D_t& ptENU, GeoUtils::geoPoint_t& geoPoint)
{
	GeoUtils::point3D_t ptECEF;
	GeoUtils::enu2ecef(enuCoord,ptENU,ptECEF);
	GeoUtils::ecef2lla(ptECEF,geoPoint);
}

void GeoUtils::enu2lla(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point2D_t& ptENU, GeoUtils::geoPoint_t& geoPoint)
{
	GeoUtils::point3D_t ptECEF;
	GeoUtils::enu2ecef(enuCoord,ptENU,ptECEF);
	GeoUtils::ecef2lla(ptECEF,geoPoint);
}

void GeoUtils::enu2lla(const GeoUtils::enuCoord_t& enuCoord, const GeoUtils::point2D_t& ptENU, GeoUtils::geoRefPoint_t& geoRef)
{
	GeoUtils::geoPoint_t geoPoint;
	GeoUtils::enu2lla(enuCoord,ptENU,geoPoint);
	GeoUtils::geoPoint2geoRefPoint(geoPoint,geoRef);
}

double GeoUtils::distlla2lla(const GeoUtils::geoPoint_t& p1, const GeoUtils::geoPoint_t& p2)
{ // distance between lat/Long points using Haversine formula
	double dLat = DsrcConstants::deg2rad(p2.latitude - p1.latitude);
	double dLon = DsrcConstants::deg2rad(p2.longitude - p1.longitude);
	double a = std::sin(dLat/2.0) * std::sin(dLat/2.0) + std::sin(dLon/2.0) * std::sin(dLon/2.0)
		* std::cos(DsrcConstants::deg2rad(p1.latitude)) * std::cos(DsrcConstants::deg2rad(p2.latitude));
	double c = 2.0 * atan2(sqrt(a),sqrt(1-a));
	return (DsrcConstants::ellipsoid_r * c);
}

long GeoUtils::dotProduct(const GeoUtils::vector2D_t& v1, const GeoUtils::vector2D_t& v2)
{
	return ((long)v1.X * v2.X + (long)v1.Y * v2.Y);
}

long GeoUtils::crossProduct(const GeoUtils::vector2D_t& v1, const GeoUtils::vector2D_t& v2)
{
	return ((long)v1.X * v2.Y - (long)v1.Y * v2.X);
}

long GeoUtils::cross(const GeoUtils::point2D_t& O, const GeoUtils::point2D_t& A, const GeoUtils::point2D_t& B)
{
	return ((long)(A.x - O.x) * (B.y - O.y) - (long)(A.y - O.y) * (B.x - O.x));
}

void GeoUtils::projectPt2Line(const GeoUtils::point2D_t& startPoint, const GeoUtils::point2D_t& endPoint,
	const GeoUtils::point2D_t& pt, GeoUtils::projection_t& proj2line)
{
	GeoUtils::vector2D_t va, vb;
	va.set(startPoint,endPoint);
	vb.set(startPoint,pt);
	double d = std::abs((double)dotProduct(va,va));
	proj2line.t = (double)dotProduct(vb,va) / d;
	proj2line.d = (double)crossProduct(vb,va) / std::sqrt(d);
	proj2line.length = std::sqrt(d);
}

MsgEnum::polygonType GeoUtils::convexcave(const std::vector<point2D_t>& p)
{ // for a convex polygon all the cross products of adjacent edges must
	// have the same sign, while concave polygon will change sign
	int size = (int)p.size();
	if (size < 3)
		return(MsgEnum::polygonType::colinear);

	GeoUtils::vector2D_t v1,v2;
	int flag = 0;
	for (int i = 0; i < size; i++)
	{
		int j = (i+1) % size;
		int k = (i+2) % size;
		v1.set(p[i],p[j]);
		v2.set(p[j],p[k]);
		long d = crossProduct(v1,v2);
		if (d < 0)
			flag |= 1;
		else if (d > 0)
			flag |= 2;
		if (flag == 3)
			return(MsgEnum::polygonType::concave);
	}
	return((flag == 0) ? (MsgEnum::polygonType::colinear) : (MsgEnum::polygonType::convex));
}

int GeoUtils::isLeft(const GeoUtils::point2D_t& p0, const GeoUtils::point2D_t& p1, const GeoUtils::point2D_t& p2)
{ // check whether p2 is on the left (>0), right (<0) or on (=0) the line through p0,p1
	return ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
}

std::vector<GeoUtils::point2D_t> GeoUtils::convexHullAndrew(std::vector<GeoUtils::point2D_t>& P)
{ // constructs the convex hull of a set of 2-dimensional points (Andrew's algorithm)
	int n = (int)P.size();
	int k = 0;
	if (n == 1)
		return(P);
	std::vector<GeoUtils::point2D_t> H(2*n);
	// sort points lexicographically
	std::sort(P.begin(), P.end());
	// build lower hull
	for (int i = 0; i < n; ++i)
	{
		while ((k >= 2) && (cross(H[k-2], H[k-1], P[i]) <= 0)) k--;
		H[k++] = P[i];
	}
	// build upper hull
	for (int i = n - 2, t = k + 1; i >= 0; i--)
	{
		while ((k >= t) && (cross(H[k-2], H[k-1], P[i]) <= 0)) k--;
		H[k++] = P[i];
	}
	H.resize(k-1);
	return(H);
}

bool GeoUtils::isPointInsidePolygon(const std::vector<GeoUtils::point2D_t>& polygon, const GeoUtils::point2D_t& waypoint)
{ // for convex polygon only: cross product between the vector (way-point->vertex)
	// and the vertex (vertex->next_vertex) must have the same sign when moving
	// counter-clockwise or clockwise along the edges
	int flag = 0;
	int size = (int)polygon.size();
	GeoUtils::vector2D_t v1,v2;

	for (int i = 0; i < size; i++)
	{
		int j = (i+1) % size;
		v1.set(waypoint,polygon[i]);
		v2.set(polygon[i],polygon[j]);
		long d = crossProduct(v1,v2);
		if (d < 0)
			flag |= 1;
		else if (d > 0)
			flag |= 2;
		if (flag == 3)
			return(false);
	}
	return(true);
}

/// time-to-go (in tenths of a second) with given dist2go and speed
uint16_t GeoUtils::getTime2Go(const double& dist2go, const double& speed_1, const double speed_2, const double& alpha)
{ /// dist2go in meters, speed_1 & speed_2 in mps, alpha in [0, 1]
	double speed2go = alpha * speed_1 + (1.0 - alpha) * speed_2;
	return((uint16_t)(std::ceil(std::abs(dist2go) / speed2go * 10)));
}
