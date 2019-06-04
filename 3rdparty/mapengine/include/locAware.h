//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _MRPLOCAWARE_H
#define _MRPLOCAWARE_H

#include <map>
#include <string>
#include <vector>

#include "dsrcMapData.h"
#include "mapDataStruct.h"

class LocAware
{
	private:
		bool initiated;
		bool speedLimitInLane;
		bool saveNewMap2nmap;
		// store intersection MAP data
		std::vector<NmapData::IntersectionStruct> mpIntersection;
		// map between (intersectionId, laneId) and (intersection, approach, lane)
		// indexes (start from 0) in mpIntersection.
		// key:   (regionalId << 24) | (intersectionId << 8) | laneId
		// value: (intIndx << 16) | (appIndx << 8) | laneIndx
		std::map<uint64_t, uint32_t> IndexMap;
		// for saving updated MapData into file
		std::string mapFilePath;

		// processing intersection MAP file
		bool readNmap(const std::string& fname);
		// processing intersection encodeed MAP payload file
		bool readPayload(const std::string& fname);
		void setIntersectionName(const std::string& name, const uint16_t& regionalId, const uint16_t& intersectionId);
		void saveNmap(const NmapData::IntersectionStruct& intObj) const;
		void setOutbond2InboundWaypoints(void);
		void setOutbond2InboundWaypoints(NmapData::IntersectionStruct& intObj);
		void getOutbond2InboundWaypoints(NmapData::IntersectionStruct& intObj);
		void setLocalOffsetAndHeading(void);
		void setLocalOffsetAndHeading(NmapData::IntersectionStruct& intObj);
		void buildPolygons(void);
		void buildPolygons(NmapData::IntersectionStruct& intObj);
		// UPER encoding MapData
		size_t encode_mapdata_payload(void);
		// add new MAP
		void addIntersection(const NmapData::IntersectionStruct& intObj);
		// get static map data elements
		uint8_t getMapVersion(const uint16_t& regionalId, const uint16_t& intersectionId) const;
		std::vector<uint8_t> getIndexesByIds(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId) const;
		uint8_t getControlPhaseByLaneId(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId) const;
		uint8_t getControlPhaseByAprochId(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& approachId) const;
		uint8_t getIndexByApproachId(const uint8_t& intersectionIndx, const uint8_t& approachId) const;
		uint8_t getLaneIdByIndexes(const uint8_t& intersectionIndx, const uint8_t& approachIndx, const uint8_t& laneIndx) const;
		// locating vehicle BSM on intersection Map
		std::vector<uint8_t> nearedIntersections(const GeoUtils::geoPoint_t& geoPoint) const;
		bool isOutboundConnect2Inbound(const NmapData::ConnectStruct& connObj, const GeoUtils::geoPoint_t& geoPoint,
			const GeoUtils::motion_t& motionState, GeoUtils::vehicleTracking_t& vehicleTrackingState) const;

	public:
		LocAware(const std::string& fname, bool isSingleFrame=false);
		~LocAware(void);

		// set option for saving new MAP into namp file
		void setSaveNewMap2nmap(const bool& option);
		// save intersection object into nmap file
		void saveNmap(const uint16_t& regionalId, const uint16_t& intersectionId) const;
		// check MAP update based on encoded MAP payload
		uint32_t checkMapUpdate(const uint8_t* buf, size_t size);
		// get static map data elements
		bool isInitiated(void) const;
		std::vector<uint32_t> getIntersectionIds(void) const;
		std::string getIntersectionNameById(const uint16_t& regionalId, const uint16_t& intersectionId) const;
		uint32_t getIntersectionIdByName(const std::string& name) const;
		uint8_t  getIndexByIntersectionId(const uint16_t& regionalId, const uint16_t& intersectionId) const;
		uint8_t  getControlPhaseByIds(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& approachId, const uint8_t& laneId) const;
		uint8_t  getApproachIdByLaneId(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId) const;
		uint32_t getLaneLength(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId) const;
		GeoUtils::geoRefPoint_t getIntersectionRefPoint(const uint8_t& intersectionIndx) const;
		std::string getIntersectionNameByIndex(const uint8_t& intersectionIndex) const;
		std::vector<uint8_t> getMapdataPayload(const uint16_t& regionalId, const uint16_t& intersectionId) const;
		bool getSpeedLimits(std::vector<uint8_t>& speedLimits, const uint16_t& regionalId, const uint16_t& intersectionId) const;
		// locating vehicle BSM on intersection Map
		bool locateVehicleInMap(const GeoUtils::connectedVehicle_t& cv, GeoUtils::vehicleTracking_t& cvTrackingState) const;
		void updateLocationAware(const GeoUtils::vehicleTracking_t& vehicleTrackingState, GeoUtils::locationAware_t& vehicleLocationAware) const;
		void getPtDist2D(const GeoUtils::vehicleTracking_t& vehicleTrackingState, GeoUtils::point2D_t& pt) const;
};

#endif
