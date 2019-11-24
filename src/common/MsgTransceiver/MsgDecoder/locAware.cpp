//*************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

#include "AsnJ2735Lib.h"
#include "dsrcConsts.h"
#include "locAware.h"

auto getFilePath = [](const std::string& str)->std::string
{
	char sep = '/';
	std::size_t found = str.rfind(sep);
	return(((found == std::string::npos) || (found == 1)) ? std::string("./") : str.substr(0, found + 1));
};

auto getFileExtension = [](const std::string& str)->std::string
{
	char sep = '.';
	std::size_t found = str.rfind(sep);
	return((found == std::string::npos) ? std::string("noExtension") : str.substr(found + 1));
};

LocAware::LocAware(const std::string& fname, bool isSingleFrame /*=false*/)
{
	saveNewMap2nmap = false;
	speedLimitInLane = isSingleFrame;
	mapFilePath = getFilePath(fname);
	std::string fileExtension = getFileExtension(fname);
	if ((fileExtension.compare("nmap") != 0) && (fileExtension.compare("payload") != 0))
	{
		initiated = true;
		std::cout << "Start without nmap file. nmap save to " << mapFilePath << std::endl;
	}
	else
	{
		initiated = false;
		if ((fileExtension.compare("nmap") == 0) && !LocAware::readNmap(fname))
		{ // read nmap file
			mpIntersection.clear();
			std::cerr << "Failed reading nmap file " << fname << std::endl;
		}
		else if ((fileExtension.compare("payload") == 0) && !LocAware::readPayload(fname))
		{ // read encoded MAP payload
			mpIntersection.clear();
			std::cerr << "Failed reading payload file " << fname << std::endl;
		}
		else
		{ // link way-points of outbound lane to way-points of inbound lane of its downstream intersection
			LocAware::setOutbond2InboundWaypoints();
			// calculate local offsets and heading for way-points
			LocAware::setLocalOffsetAndHeading();
			// build approach boxes
			LocAware::buildPolygons();
			if (fileExtension.compare("nmap") == 0)
			{ // encode MAP payload
				std::cout << "Read " << mpIntersection.size() << " intersections" << std::endl;
				size_t encoded_interections = LocAware::encode_mapdata_payload();
				if (encoded_interections != mpIntersection.size())
				{
					std::cerr << "Encoded " << encoded_interections << " out of " << mpIntersection.size() << " intersections" << std::endl;
					mpIntersection.clear();
				}
				else
				{
					std::cout << "Encoded MAP for " << mpIntersection.size() << " intersections" << std::endl;
					initiated = true;
				}
			}
			else if (fileExtension.compare("payload") == 0)
			{
				std::cout << "Loaded MAP payload for " << mpIntersection.size() << " intersections" << std::endl;
				initiated = true;
			}
		}
	}
}

LocAware::~LocAware(void)
	{mpIntersection.clear();}

auto ids2id = [](const uint16_t& regionalId, const uint16_t& intersectionId)->uint32_t
	{return((uint32_t)(regionalId << 16) | intersectionId);};

auto getIndexMapKey = [](const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId)->uint64_t
	{return((uint64_t)((regionalId << 24) | (intersectionId << 8) | laneId));};

auto getIndexMapValue = [](const uint32_t& intIndx, const uint32_t& appIndx, const uint32_t& laneIndx)->uint32_t
	{return((uint32_t)((intIndx << 16) | (appIndx << 8) | laneIndx));};

auto isEmptyStr = [](const std::string& str)->bool
	{return(str.empty() || std::all_of(str.begin(), str.end(), isspace));};

auto setApproachType = [](const std::string& str, MsgEnum::approachType& type)->bool
{
	bool retn = true;
	if (str.compare("inbound") == 0)
		type = MsgEnum::approachType::inbound;
	else if (str.compare("outbound") == 0)
		type = MsgEnum::approachType::outbound;
	else if (str.compare("crosswalk") == 0)
		type = MsgEnum::approachType::crosswalk;
	else
		retn = false;
	return(retn);
};

auto getApproachType = [](const MsgEnum::approachType& type)->std::string
{
	switch(type)
	{
	case MsgEnum::approachType::inbound:
		return(std::string("inbound"));
	case MsgEnum::approachType::outbound:
		return(std::string("outbound"));
	default:
		return(std::string("crosswalk"));
	}
};

auto setLaneType = [](const std::string& str, MsgEnum::laneType& type)->bool
{
	bool retn = true;
	if (str.compare("traffic") == 0)
		type = MsgEnum::laneType::traffic;
	else if (str.compare("crosswalk") == 0)
		type = MsgEnum::laneType::crosswalk;
	else
		retn = false;
	return(retn);
};

auto getLaneType = [](const MsgEnum::laneType& type)->std::string
{
	switch(type)
	{
	case MsgEnum::laneType::traffic:
		return(std::string("traffic"));
	default:
		return(std::string("crosswalk"));
	}
};

auto setManeuverType = [](const std::string& str, MsgEnum::maneuverType& type)->bool
{
	bool retn = true;
	if (str.compare("uTurn") == 0)
		type = MsgEnum::maneuverType::uTurn;
	else if (str.compare("leftTurn") == 0)
		type = MsgEnum::maneuverType::leftTurn;
	else if (str.compare("rightTurn") == 0)
		type = MsgEnum::maneuverType::rightTurn;
	else if (str.compare("straightAhead") == 0)
		type = MsgEnum::maneuverType::straightAhead;
	else
	{
		type = MsgEnum::maneuverType::unavailable;
		retn = false;
	}
	return(retn);
};

auto getManeuverType = [](const MsgEnum::maneuverType& type)->std::string
{
	switch(type)
	{
	case MsgEnum::maneuverType::uTurn:
		return(std::string("uTurn"));
	case MsgEnum::maneuverType::leftTurn:
		return(std::string("leftTurn"));
	case MsgEnum::maneuverType::rightTurn:
		return(std::string("rightTurn"));
	case MsgEnum::maneuverType::straightAhead:
	case MsgEnum::maneuverType::straight:
		return(std::string("straightAhead"));
	default:
		return(std::string("unavailable"));
	}
};

auto setLaneManeuver = [](const MsgEnum::maneuverType& type, std::bitset<20>& attributes)->void
{
	switch(type)
	{
	case MsgEnum::maneuverType::leftTurn:
		attributes.set(9);
		break;
	case MsgEnum::maneuverType::rightTurn:
		attributes.set(10);
		break;
	case MsgEnum::maneuverType::uTurn:
		attributes.set(11);
		break;
	case MsgEnum::maneuverType::straightAhead:
	case MsgEnum::maneuverType::straight:
		attributes.set(8);
		break;
	default:
		break;
	}
};

auto setLaneRestriction = [](const std::string& str, const MsgEnum::laneType& type, std::bitset<20>& attributes)->bool
{
	bool retn = true;
	if (type == MsgEnum::laneType::traffic)
	{
		if (str.compare("isVehicleRevocableLane") == 0)
			attributes.set(0);
		if (str.compare("flyOverLane") == 0)
			attributes.set(1);
		else if (str.compare("hovOnly") == 0)
			attributes.set(2);
		else if (str.compare("busOnly") == 0)
			attributes.set(3);
		else if (str.compare("TaxiOnly") == 0)
			attributes.set(4);
		else if (str.compare("private") == 0)
			attributes.set(5);
		else if (str.compare("hasIRbeaconCoverage") == 0)
			attributes.set(6);
		else if (str.compare("permissionOnRequest") == 0)
			attributes.set(7);
		else
			retn = false;
	}
	else
	{
		if (str.compare("crosswalkRevocableLane") == 0)
			attributes.set(0);
		else if (str.compare("bicyleUseAllowed") == 0)
			attributes.set(1);
		else if (str.compare("flyOverLane") == 0)
			attributes.set(2);
		else if (str.compare("pedRecallOn") == 0)
			attributes.set(3);
		else if (str.compare("biDirectionalCycleTimes") == 0)
			attributes.set(4);
		else if (str.compare("hasPushButton") == 0)
			attributes.set(5);
		else if (str.compare("audioSupport") == 0)
			attributes.set(6);
		else if (str.compare("rfSignalRequestPresent") == 0)
			attributes.set(7);
		else if (str.compare("unsignalizedSegmentsPresent") == 0)
			attributes.set(8);
		else
			retn = false;
	}
	return(retn);
};

auto getLaneRestriction = [](const MsgEnum::laneType& type, const std::bitset<20>& attributes)->std::vector<std::string>
{
	std::vector<std::string> retn;
	if (type == MsgEnum::laneType::traffic)
	{
		if (attributes.test(0))
			retn.push_back(std::string("isVehicleRevocableLane"));
		if (attributes.test(1))
			retn.push_back(std::string("flyOverLane"));
		if (attributes.test(2))
			retn.push_back(std::string("hovOnly"));
		if (attributes.test(3))
			retn.push_back(std::string("busOnly"));
		if (attributes.test(4))
			retn.push_back(std::string("TaxiOnly"));
		if (attributes.test(5))
			retn.push_back(std::string("private"));
		if (attributes.test(6))
			retn.push_back(std::string("hasIRbeaconCoverage"));
		if (attributes.test(7))
			retn.push_back(std::string("permissionOnRequest"));
	}
	else
	{
		if (attributes.test(0))
			retn.push_back(std::string("crosswalkRevocableLane"));
		if (attributes.test(1))
			retn.push_back(std::string("bicyleUseAllowed"));
		if (attributes.test(2))
			retn.push_back(std::string("flyOverLane"));
		if (attributes.test(3))
			retn.push_back(std::string("pedRecallOn"));
		if (attributes.test(4))
			retn.push_back(std::string("biDirectionalCycleTimes"));
		if (attributes.test(5))
			retn.push_back(std::string("hasPushButton"));
		if (attributes.test(6))
			retn.push_back(std::string("audioSupport"));
		if (attributes.test(7))
			retn.push_back(std::string("rfSignalRequestPresent"));
		if (attributes.test(8))
			retn.push_back(std::string("unsignalizedSegmentsPresent"));
	}
	return(retn);
};

auto setLaneRule = [](const std::string& str, std::bitset<20>& attributes)->bool
{
	bool retn = true;
	if (str.compare("leftTurnOnRedAllowed") == 0)
		attributes.set(12);
	else if (str.compare("rightTurnOnRedAllowed") == 0)
		attributes.set(13);
	else if (str.compare("laneChangeAllowed") == 0)
		attributes.set(14);
	else if (str.compare("noStopping") == 0)
		attributes.set(15);
	else if (str.compare("yield") == 0)
		attributes.set(16);
	else if (str.compare("goWithHalt") == 0)
		attributes.set(17);
	else if (str.compare("caution") == 0)
		attributes.set(18);
	else
		retn = false;
	return(retn);
};

auto getLaneRule = [](const std::bitset<20>& attributes)->std::vector<std::string>
{
	std::vector<std::string> retn;
	if (attributes.test(12))
		retn.push_back(std::string("leftTurnOnRedAllowed"));
	if (attributes.test(13))
		retn.push_back(std::string("rightTurnOnRedAllowed"));
	if (attributes.test(14))
		retn.push_back(std::string("noStopping"));
	if (attributes.test(15))
		retn.push_back(std::string("yield"));
	if (attributes.test(16))
		retn.push_back(std::string("private"));
	if (attributes.test(17))
		retn.push_back(std::string("goWithHalt"));
	if (attributes.test(18))
		retn.push_back(std::string("caution"));
	return(retn);
};

/// --- start of functions to process the intersection nmap file --- ///
std::vector<uint8_t> LocAware::getIndexesByIds(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId) const
{
	std::vector<uint8_t> ret;
	auto it = IndexMap.find(getIndexMapKey(regionalId, intersectionId, laneId));
	if (it != IndexMap.end())
	{
		uint32_t value = it->second;
		ret.push_back(((value >> 16) & 0xFF));
		ret.push_back(((value >> 8) & 0xFF));
		ret.push_back((value & 0xFF));
	}
	return(ret);
}

bool LocAware::readPayload(const std::string& fname)
{ // open payload file
	std::ifstream IS_PAYLOAD(fname);
	if (!IS_PAYLOAD.is_open())
	{
		std::cerr << "readPayload: failed open " << fname << std::endl;
		return(false);
	}
	// read payload file
	std::istringstream iss;
	std::string line, s;
	std::string intersectionName, payload;
	bool has_error = false;
	while (std::getline(IS_PAYLOAD, line))
	{
		if (isEmptyStr(line))
			continue;
		if (line.find("payload") != std::string::npos)
		{ // beginning of an encoded intersection MAP payload
			iss.str(line);
			iss >> std::skipws >> s >> intersectionName >> payload;
			iss.clear();
			// convert payload hex string to buffer
			std::vector<uint8_t> buf;
			for (size_t i = 0, j = payload.length(); i < j; i += 2)
			{
				std::string byteString = payload.substr(i, 2);
				buf.push_back(static_cast<uint8_t>(std::strtol(byteString.c_str(), NULL, 16)));
			}
			uint32_t referenceId = LocAware::checkMapUpdate(&buf[0], buf.size());
			if (referenceId == 0)
			{
				std::cerr << "readPayload: failed decoding MAP payload for " << intersectionName << std::endl;
				has_error = true;
				break;
			}
			else
			{	// assign intersectionName
				uint16_t regionalId = static_cast<uint16_t>((referenceId >> 16) & 0xFFFF);
				uint16_t intersectionId = static_cast<uint16_t>(referenceId & 0xFFFF);
				LocAware::setIntersectionName(intersectionName, regionalId, intersectionId);
			}
		}
	}
	IS_PAYLOAD.close();
	return(!has_error);
}

bool LocAware::readNmap(const std::string& fname)
{ // open nmap file
	std::ifstream IS_NMAP(fname);
	if (!IS_NMAP.is_open())
	{
		std::cerr << "readNmap: failed open " << fname << std::endl;
		return(false);
	}
	// read nmap
	std::istringstream iss;
	std::string line, s;
	NmapData::IntersectionStruct* pIntersection = nullptr;
	NmapData::ApproachStruct*     pApproach = nullptr;
	NmapData::LaneStruct*         pLane = nullptr;
	GeoUtils::geoPoint_t          geoPoint;
	uint32_t intersectionSeq = 0;
	uint32_t approachSeq = 0;
	uint32_t laneSeq = 0;
	uint32_t laneId = 0;
	uint32_t iTmp;

	bool has_error = false;
	while (!has_error && std::getline(IS_NMAP, line))
	{
		if (isEmptyStr(line))
			continue;
		if (line.find("MAP_Name") != std::string::npos)
		{ // beginning of a new MAP
			pIntersection = new NmapData::IntersectionStruct;
			pIntersection->attributes.set(1);  /// Geometric data
			pIntersection->attributes.set(2);  /// Speed limit is required
			iss.str(line);
			iss >> std::skipws >> s >> pIntersection->name;
			iss.clear();
		}
		else if (line.find("MAP_Version") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> iTmp;
			iss.clear();
			pIntersection->mapVersion = static_cast<uint8_t>(iTmp & 0xFF);
			if (pIntersection->mapVersion > 127)
			{
				std::cerr << "readNmap: MAP_Version should be between 0 - 127 for intersection " << pIntersection->name << std::endl;
				has_error = true;
			}
		}
		else if (line.find("RegionalID") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> pIntersection->regionalId;
			iss.clear();
		}
		else if (line.find("IntersectionID") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> pIntersection->id;
			iss.clear();
		}
		else if (line.find("WithElevation") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> s;
			iss.clear();
			std::transform (s.begin(), s.end(), s.begin(), ::tolower);
			if (s.compare("yes") == 0)
				pIntersection->attributes.set(0);
		}
		else if (line.find("Reference_point") != std::string::npos)
		{
			iss.str(line);
			if (pIntersection->attributes.test(0))
				iss >> std::skipws >> s >> geoPoint.latitude >> geoPoint.longitude >> geoPoint.elevation;
			else
			{
				iss >> std::skipws >> s >> geoPoint.latitude >> geoPoint.longitude;
				geoPoint.elevation = 0.0;
			}
			iss.clear();
			GeoUtils::geoPoint2geoRefPoint(geoPoint, pIntersection->geoRef);
			GeoUtils::setEnuCoord(geoPoint, pIntersection->enuCoord);
		}
		else if (line.find("ApproachID") != std::string::npos)
		{ // beginning of a new approach
			if (pLane != nullptr)
			{
				pApproach->mpLanes.push_back(*pLane);
				if (pLane->mpNodes.empty())
				{
					std::cerr << "readNmap: empty Nodes for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
					std::cerr << " lane " << (laneSeq + 1) << std::endl;
					has_error = true;
				}
				delete pLane;
				pLane = nullptr;
			}
			if (pApproach != nullptr)
			{
				pIntersection->mpApproaches.push_back(*pApproach);
				approachSeq++;
				if (pApproach->mpLanes.empty())
				{
					std::cerr << "readNmap: empty Lanes for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id) << std::endl;
					has_error = true;
				}
				delete pApproach;
				pApproach = nullptr;
			}
			if (!has_error)
			{
				pApproach = new NmapData::ApproachStruct;
				iss.str(line);
				iss >> std::skipws >> s >> iTmp;
				iss.clear();
				pApproach->id = static_cast<uint8_t>(iTmp & 0xFF);
				laneSeq = 0;
				if ((pApproach->id == 0) || (pApproach->id > 15))
				{
					std::cerr << "readNmap: ApproachID should be between 1 - 15 for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id) << std::endl;
					has_error = true;
				}
			}
		}
		else if (line.find("Approach_type") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> s;
			iss.clear();
			if (!setApproachType(s, pApproach->type))
			{
				std::cerr << "readNmap: invalid Approach_type " << s;
				std::cerr << " for intersection " << pIntersection->name;
				std::cerr << " approach " << static_cast<unsigned int>(pApproach->id) << std::endl;
				has_error = true;
			}
		}
		else if (line.find("Speed_limit") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> iTmp;
			iss.clear();
			pApproach->speed_limit = static_cast<uint8_t>(iTmp & 0xFF);
			// add distinct speed limits to speeds array
			if (std::find(pIntersection->speeds.begin(), pIntersection->speeds.end(), pApproach->speed_limit) == pIntersection->speeds.end())
				pIntersection->speeds.push_back(pApproach->speed_limit);
		}
		else if (line.find("Lane_seq") != std::string::npos)
		{ // beginning of a new lane
			if (pLane != nullptr)
			{
				pApproach->mpLanes.push_back(*pLane);
				laneSeq++;
				if (pLane->mpNodes.empty())
				{
					std::cerr << "readNmap: empty Nodes for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
					std::cerr << " lane " << (laneSeq + 1) << std::endl;
					has_error = true;
				}
				delete pLane;
				pLane = nullptr;
			}
			if (!has_error)
			{
				pLane = new NmapData::LaneStruct;
				pLane->id = static_cast<uint8_t>(++laneId);
				IndexMap[getIndexMapKey(pIntersection->regionalId, pIntersection->id, pLane->id)] = getIndexMapValue(intersectionSeq, approachSeq, laneSeq);
			}
		}
		else if (line.find("Lane_type") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> s;
			iss.clear();
			if(!setLaneType(s, pLane->type))
			{
				std::cerr << "readNmap: invalid Lane_type " << s;
				std::cerr << " for intersection " << pIntersection->name;
				std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
				std::cerr << " lane " << (laneSeq + 1) << std::endl;
				has_error = true;
			}
		}
		else if (line.find("Lane_phaseNo") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> iTmp;
			iss.clear();
			pLane->controlPhase = static_cast<uint8_t>(iTmp & 0xFF);
		}
		else if (line.find("Lane_width") != std::string::npos)
		{
			iss.str(line);
			iss >> std::skipws >> s >> pLane->width;
			iss.clear();
		}
		else if (line.find("Lane_Use") != std::string::npos)
		{ // beginning of lane use restriction
			uint32_t restrictionNums = 0;
			while (std::getline(IS_NMAP, line))
			{
				if (isEmptyStr(line))
					continue;
				if ((line.find("End_LaneUse") != std::string::npos) || (restrictionNums > 9))
					break;
				iss.str(line);
				iss >> std::skipws >> s;
				iss.clear();
				if(!setLaneRestriction(s, pLane->type, pLane->attributes))
				{
					std::cerr << "readNmap: invalid laneUseRestriction " << s;
					std::cerr << " for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
					std::cerr << " lane " << (laneSeq + 1) << std::endl;
					has_error = true;
					break;
				}
				restrictionNums++;
			}
		}
		else if (line.find("Lane_Rules") != std::string::npos)
		{ // beginning of lane use restriction
			uint32_t ruleNums = 0;
			while (std::getline(IS_NMAP, line))
			{
				if (isEmptyStr(line))
					continue;
				if ((line.find("End_LaneRules") != std::string::npos) || (ruleNums > 8))
					break;
				iss.str(line);
				iss >> std::skipws >> s;
				iss.clear();
				if((pLane->type == MsgEnum::laneType::traffic) && !setLaneRule(s, pLane->attributes))
				{
					std::cerr << "readNmap: invalid laneRule " << s;
					std::cerr << " for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
					std::cerr << " lane " << (laneSeq + 1) << std::endl;
					has_error = true;
					break;
				}
				ruleNums++;
			}
		}
		else if (line.find("Lane_Nodes") != std::string::npos)
		{ // beginning of nodes
			uint32_t nodeNums = 0;
			while (std::getline(IS_NMAP, line))
			{
				if (isEmptyStr(line))
					continue;
				if ((line.find("End_Nodes") != std::string::npos) || (nodeNums > 63))
					break;
				iss.str(line);
				iss >> std::skipws >> geoPoint.latitude >> geoPoint.longitude;
				iss.clear();
				geoPoint.elevation = DsrcConstants::deca2unit<int32_t>(pIntersection->geoRef.elevation);
				NmapData::NodeStruct* pNode = new NmapData::NodeStruct;
				GeoUtils::geoPoint2geoRefPoint(geoPoint, pNode->geoNode);
				pLane->mpNodes.push_back(*pNode);
				delete pNode;
				nodeNums++;
			}
			pLane->numpoints = (size_t)nodeNums;
			if ((nodeNums < 2) || (nodeNums > 63))
			{
				std::cerr << "readNmap: Lane_Nodes should be between 2 and 63 for intersection " << pIntersection->name;
				std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
				std::cerr << " lane " << (laneSeq + 1) << std::endl;
				has_error = true;
			}
		}
		else if (line.find("Lane_ConnectsTo") != std::string::npos)
		{ // beginning of connectTo
			uint32_t connectToNums = 0;
			unsigned int connectToRegionalId;
			unsigned int connectToIntersetionId;
			unsigned int connectToApproachId;   // start from 1
			unsigned int connectToLaneIndx;     // start from 1
			std::string connManeuver;
			MsgEnum::maneuverType type;
			while (std::getline(IS_NMAP, line))
			{
				if (isEmptyStr(line))
					continue;
				if ((line.find("End_LaneConnectsTo") != std::string::npos) || (connectToNums > 16))
					break;
				iss.str(line);
				iss >> std::skipws >> s >> connManeuver;
				iss.clear();
				if (!setManeuverType(connManeuver, type))
				{
					std::cerr << "readNmap: invalid Lane_ConnectsTo connManeuver " << connManeuver;
					std::cerr << " for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
					std::cerr << " lane " << (laneSeq + 1) << std::endl;
					has_error = true;
					break;
				}
				if (std::sscanf(s.c_str(), "%u.%u.%u.%u", &connectToRegionalId, &connectToIntersetionId, &connectToApproachId, &connectToLaneIndx) != 4)
				{
					std::cerr << "readNmap: failed parsing Lane_ConnectsTo remoteLane " << s;
					std::cerr << " for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
					std::cerr << " lane " << (laneSeq + 1) << std::endl;
					has_error = true;
					break;
				}
				if (pLane->type == MsgEnum::laneType::traffic)
					setLaneManeuver(type, pLane->attributes);
				NmapData::ConnectStruct* pConnObj = new NmapData::ConnectStruct;
				pConnObj->regionalId = static_cast<uint16_t>(connectToRegionalId);
				pConnObj->intersectionId = static_cast<uint16_t>(connectToIntersetionId);
				pConnObj->laneId = static_cast<uint8_t>(((connectToApproachId & 0x0F) << 4) | (--connectToLaneIndx & 0x0F));
				pConnObj->laneManeuver = ((type == MsgEnum::maneuverType::straightAhead) && (connectToRegionalId != pIntersection->regionalId)
					&& (connectToIntersetionId != pIntersection->id)) ? MsgEnum::maneuverType::straight : type;
				pLane->mpConnectTo.push_back(*pConnObj);
				delete pConnObj;
				connectToNums++;
			}
			if (connectToNums > 16)
			{
				std::cerr << "readNmap: Lane_ConnectsTo should be between 1 and 16 for intersection " << pIntersection->name;
				std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
				std::cerr << " lane " << (laneSeq + 1) << std::endl;
				has_error = true;
			}
		}
		else if (line.find("End_MAP") != std::string::npos)
		{ // end of intersection MAP
			if (pLane != nullptr)
			{
				pApproach->mpLanes.push_back(*pLane);
				if (pLane->mpNodes.empty())
				{
					std::cerr << "readNmap: empty Nodes for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id);
					std::cerr << " lane " << (laneSeq + 1) << std::endl;
					has_error = true;
				}
				delete pLane;
				pLane = nullptr;
			}
			if (pApproach != nullptr)
			{
				pIntersection->mpApproaches.push_back(*pApproach);
				if (pApproach->mpLanes.empty())
				{
					std::cerr << "readNmap: empty Lanes for intersection " << pIntersection->name;
					std::cerr << " approach " << static_cast<unsigned int>(pApproach->id) << std::endl;
					has_error = true;
				}
				delete pApproach;
				pApproach = nullptr;
			}
			if (pIntersection->speeds.empty())
			{
				std::cerr << "readNmap: missing speed limit for intersection " << pIntersection->name << std::endl;
				has_error = true;
			}
			mpIntersection.push_back(*pIntersection);
			delete pIntersection;
			pIntersection = nullptr;
			if (!has_error)
			{
				approachSeq = 0;
				laneSeq = 0;
				laneId = 0;
				intersectionSeq++;
			}
		}
	}
	IS_NMAP.close();
	if (pLane != nullptr)
		delete pLane;
	if (pApproach != nullptr)
		delete pApproach;
	if (pIntersection != nullptr)
		delete pIntersection;
	if (has_error)
		return(false);
	/// assign ConnectStruct::laneId
	for (auto& intObj : mpIntersection)
	{
		for (auto& appObj : intObj.mpApproaches)
		{
			for (size_t laneIndx = 0; laneIndx < appObj.mpLanes.size(); laneIndx++)
			{
				auto& laneObj = appObj.mpLanes[laneIndx];
				if (laneObj.mpConnectTo.empty() && (laneObj.type == MsgEnum::laneType::traffic))
					laneObj.attributes.set(8);
				for (auto& conn2obj : laneObj.mpConnectTo)
				{
					uint8_t conn2approachId = (conn2obj.laneId >> 4) & 0x0F;
					uint8_t conn2laneIndx = conn2obj.laneId & 0x0F;
					uint8_t conn2intersectionIndx = LocAware::getIndexByIntersectionId(conn2obj.regionalId, conn2obj.intersectionId);
					uint8_t conn2approachIndx = (conn2intersectionIndx == 0xFF) ? 0xFF : LocAware::getIndexByApproachId(conn2intersectionIndx, conn2approachId);
					uint8_t conn2LaneId = (conn2approachIndx == 0xFF) ? 0 : LocAware::getLaneIdByIndexes(conn2intersectionIndx, conn2approachIndx, conn2laneIndx);
					if ((conn2intersectionIndx == 0xFF) || (conn2approachIndx == 0xFF) || (conn2LaneId == 0))
					{
						std::cerr << "readNmap: invalid Lane_ConnectsTo " << conn2obj.regionalId << "." << conn2obj.intersectionId << ".";
						std::cerr << static_cast<unsigned int>(conn2approachId) << static_cast<unsigned int>(conn2laneIndx);
						std::cerr << " from intersection " << intObj.name << ", approach " << static_cast<unsigned int>(appObj.id);
						std::cerr << ", lane " << (laneIndx + 1) << std::endl;
						has_error = true;
					}
					conn2obj.laneId = conn2LaneId;
				}
			}
		}
	}
	return(!has_error);
}

void LocAware::saveNmap(const uint16_t& regionalId, const uint16_t& intersectionId) const
{
	uint8_t intIndex = getIndexByIntersectionId(regionalId, intersectionId);
	if (intIndex == 0xFF)
		std::cerr << "saveNmap: intersection map object not exist for " << regionalId << "." << intersectionId << std::endl;
	else
		LocAware::saveNmap(mpIntersection[intIndex]);
}

void LocAware::saveNmap(const NmapData::IntersectionStruct& intObj) const
{
	std::string fname = mapFilePath + intObj.name + std::string(".nmap");
	std::ofstream OS_NMAP(fname);
	if (!OS_NMAP.is_open())
	{
		std::cerr << "saveNmap: failed open " << fname << std::endl;
		return;
	}
	GeoUtils::geoPoint_t geoPoint;
	GeoUtils::geoRefPoint2geoPoint(intObj.geoRef, geoPoint);
	OS_NMAP << "MAP_Name " << intObj.name << std::endl;
	OS_NMAP << "MAP_Version " << static_cast<unsigned int>(intObj.mapVersion) << std::endl;
	OS_NMAP << "RegionalID " << intObj.regionalId << std::endl;
	OS_NMAP << "IntersectionID " << intObj.id << std::endl;
	OS_NMAP << "WithElevation " << (intObj.attributes.test(0) ? "yes" : "no") << std::endl;
	OS_NMAP << "Reference_point " << std::fixed << std::setprecision(7);
	OS_NMAP << geoPoint.latitude << "  " << geoPoint.longitude;
	if (intObj.attributes.test(0))
		OS_NMAP << std::setprecision(1) << "  " << geoPoint.elevation;
	OS_NMAP << std::endl;
	for (const auto& appObj : intObj.mpApproaches)
	{
		OS_NMAP << "ApproachID " << static_cast<unsigned int>(appObj.id) << std::endl;
		OS_NMAP << "\tApproach_type " << getApproachType(appObj.type) << std::endl;
		OS_NMAP << "\tSpeed_limit " << static_cast<unsigned int>(appObj.speed_limit) << std::endl;
		unsigned int laneSeq = 0;
		for (const auto& laneObj : appObj.mpLanes)
		{
			laneSeq++;
			OS_NMAP << "\tLane_seq " << laneSeq << std::endl;
			OS_NMAP << "\t\tLane_type " << getLaneType(laneObj.type) << std::endl;
			OS_NMAP << "\t\tLane_phaseNo " << static_cast<unsigned int>(laneObj.controlPhase) << std::endl;
			OS_NMAP << "\t\tLane_width " << static_cast<unsigned int>(laneObj.width) << std::endl;
			std::vector<std::string> laneUseRestrictions = getLaneRestriction(laneObj.type, laneObj.attributes);
			if (!laneUseRestrictions.empty())
			{
				OS_NMAP << "\t\tLane_Use" << std::endl;
				for (const auto& item : laneUseRestrictions)
					OS_NMAP << "\t\t\t" << item << std::endl;
				OS_NMAP << "\t\tEnd_LaneUse" << std::endl;
			}
			std::vector<std::string> laneRules = getLaneRule(laneObj.attributes);
			if (!laneRules.empty())
			{
				OS_NMAP << "\t\tLane_Rules" << std::endl;
				for (const auto& item : laneRules)
					OS_NMAP << "\t\t\t" << item << std::endl;
				OS_NMAP << "\t\tEnd_LaneRules" << std::endl;
			}
			OS_NMAP << "\t\tLane_Nodes" << std::endl;
			for (size_t indx = 0; indx < laneObj.numpoints; indx++)
			{
				const auto& nodeObj = laneObj.mpNodes[indx];
				GeoUtils::geoRefPoint2geoPoint(nodeObj.geoNode, geoPoint);
				OS_NMAP << "\t\t\t" << std::fixed << std::setprecision(7) << geoPoint.latitude;
				OS_NMAP << "  " << geoPoint.longitude << std::endl;
			}
			OS_NMAP << "\t\tEnd_Nodes" << std::endl;
			if (!laneObj.mpConnectTo.empty())
			{
				OS_NMAP << "\t\tLane_ConnectsTo" << std::endl;
				for (const auto& connObj : laneObj.mpConnectTo)
				{
					auto inds = LocAware::getIndexesByIds(connObj.regionalId, connObj.intersectionId, connObj.laneId);
					if (!inds.empty())
					{
						OS_NMAP << "\t\t\t" << connObj.regionalId << "." << connObj.intersectionId << ".";
						OS_NMAP << static_cast<unsigned int>(mpIntersection[inds[0]].mpApproaches[inds[1]].id) << ".";
						OS_NMAP << static_cast<unsigned int>(inds[2] + 1) << " " << getManeuverType(connObj.laneManeuver) << std::endl;
					}
				}
				OS_NMAP << "\t\tEnd_LaneConnectsTo" << std::endl;
			}
		}
	}
	OS_NMAP << "End_MAP" << std::endl;
	OS_NMAP.close();
}

void LocAware::setOutbond2InboundWaypoints(void)
{
	for (auto& intObj : mpIntersection)
		LocAware::setOutbond2InboundWaypoints(intObj);
}

void LocAware::setOutbond2InboundWaypoints(NmapData::IntersectionStruct& intObj)
{ // link outbound lane way-points of this intersection to its downstream connected inbound lane
	for (auto& appObj : intObj.mpApproaches)
	{
		if (appObj.type != MsgEnum::approachType::outbound)
			continue;
		for (auto& laneObj : appObj.mpLanes)
		{
			if ((laneObj.type != MsgEnum::laneType::traffic) || laneObj.mpNodes.empty()
					|| laneObj.mpConnectTo.empty() || (laneObj.mpConnectTo.size() != 1))
				continue;
			auto& connObj = laneObj.mpConnectTo[0];
			if ((connObj.regionalId == intObj.regionalId) && (connObj.intersectionId == intObj.id))
				continue;
			auto inds = LocAware::getIndexesByIds(connObj.regionalId, connObj.intersectionId, connObj.laneId);
			if (inds.empty())
				continue;
			auto& connIntObj = mpIntersection[inds[0]];
			auto& connAppObj = connIntObj.mpApproaches[inds[1]];
			auto& connLaneObj = connAppObj.mpLanes[inds[2]];
			if (connAppObj.type != MsgEnum::approachType::inbound)
				continue;
			if (connLaneObj.mpNodes.size() > connLaneObj.numpoints)
				connLaneObj.mpNodes.resize(connLaneObj.numpoints);
			uint32_t connIntId = ids2id(connIntObj.regionalId, connIntObj.id);
			if (std::find(intObj.mpConnIntersections.begin(), intObj.mpConnIntersections.end(), connIntId) == intObj.mpConnIntersections.end())
				intObj.mpConnIntersections.push_back(connIntId);
			for (auto rit = laneObj.mpNodes.rbegin(); rit != laneObj.mpNodes.rend(); ++rit)
			{
				const auto& geoNode = rit->geoNode;
				if(std::find_if(connLaneObj.mpNodes.begin(), connLaneObj.mpNodes.end(),
					[&geoNode](const NmapData::NodeStruct& obj){return(obj.geoNode == geoNode);}) == connLaneObj.mpNodes.end())
				{
					NmapData::NodeStruct node;
					node.geoNode = geoNode;
					node.geoNode.elevation = connIntObj.geoRef.elevation;
					connLaneObj.mpNodes.push_back(node);
				}
			}
		}
	}
}

void LocAware::getOutbond2InboundWaypoints(NmapData::IntersectionStruct& intObj)
{ // link inbound lane way-points of this intersection to its upstream connected outbound lane
	for (auto& appObj : intObj.mpApproaches)
	{
		if (appObj.type != MsgEnum::approachType::inbound)
			continue;
		for (auto& laneObj : appObj.mpLanes)
		{
			if (laneObj.type != MsgEnum::laneType::traffic)
				continue;
			for (auto& connIntObj : mpIntersection)
			{
				if ((connIntObj.regionalId == intObj.regionalId) && (connIntObj.id == intObj.id))
					continue;
				for (auto& connAppObj : connIntObj.mpApproaches)
				{
					if (connAppObj.type != MsgEnum::approachType::outbound)
						continue;
					for (auto& connLaneObj : connAppObj.mpLanes)
					{
						if ((connLaneObj.type != MsgEnum::laneType::traffic) || connLaneObj.mpNodes.empty()
								|| connLaneObj.mpConnectTo.empty() || (connLaneObj.mpConnectTo.size() != 1))
							continue;
						auto& connObj = connLaneObj.mpConnectTo[0];
						if ((connObj.regionalId != intObj.regionalId) || (connObj.intersectionId != intObj.id) || (connObj.laneId != laneObj.id))
							continue;
						uint32_t connIntId = ids2id(intObj.regionalId, intObj.id);
						if (std::find(connIntObj.mpConnIntersections.begin(), connIntObj.mpConnIntersections.end(), connIntId) == connIntObj.mpConnIntersections.end())
							connIntObj.mpConnIntersections.push_back(connIntId);
						for (auto rit = connLaneObj.mpNodes.rbegin(); rit!= connLaneObj.mpNodes.rend(); ++rit)
						{
							const auto& geoNode = rit->geoNode;
							if (std::find_if(laneObj.mpNodes.begin(), laneObj.mpNodes.end(),
								[&geoNode](const NmapData::NodeStruct& obj){return(obj.geoNode == geoNode);}) == laneObj.mpNodes.end())
							{
								NmapData::NodeStruct node;
								node.geoNode = geoNode;
								node.geoNode.elevation = intObj.geoRef.elevation;
								laneObj.mpNodes.push_back(node);
							}
						}
					}
				}
			}
		}
	}
}

void LocAware::setLocalOffsetAndHeading(void)
{
	for (auto& intObj : mpIntersection)
		LocAware::setLocalOffsetAndHeading(intObj);
}

void LocAware::setLocalOffsetAndHeading(NmapData::IntersectionStruct& intObj)
{
	GeoUtils::point2D_t origin{0,0};
	// set intObj.radius, appObj.mindist2intsectionCentralLine, mpNodes.ptNode & mpNodes.dTo1stNode
	uint32_t radius = 0;
	for (auto& appObj : intObj.mpApproaches)
	{
		for (auto& laneObj : appObj.mpLanes)
		{
			uint32_t dTo1stNode = 0;
			for (size_t i = 0, j = laneObj.mpNodes.size(); i < j; i++)
			{
				auto& nodeObj = laneObj.mpNodes[i];
				GeoUtils::lla2enu(intObj.enuCoord, nodeObj.geoNode, nodeObj.ptNode);
				if (i > 0)
				{
					auto& prevNodeObj = laneObj.mpNodes[i-1];
					dTo1stNode += nodeObj.ptNode.distance2pt(prevNodeObj.ptNode);
				}
				nodeObj.dTo1stNode = dTo1stNode;
				uint32_t ptLength = nodeObj.ptNode.length();
				if (ptLength > radius)
					radius = ptLength;
			}
			// reset dTo1stNode for the first node:
			// For traffic lanes, project intersection ref point (i.e., origin) onto the closed segment on lane,
			//  set distance from the closed way-point to the projected point as dTo1stNode for the closed way-point
			//  this is helpful for applications that crossing the stop-bar will be an event trigger, such as to cancel TSP request.
			//  GPS overshot at stop-bar could cause wrong cancel request. This projected distance to intersection center can be used
			//  to ensure the vehicle has crossed the stop-bar.
			//  This value won't affect calculation of distance to the stop-bar.
			GeoUtils::projection_t proj2segment;
			switch (appObj.type)
			{
			case MsgEnum::approachType::inbound:
				GeoUtils::projectPt2Line(laneObj.mpNodes[1].ptNode, laneObj.mpNodes[0].ptNode, origin, proj2segment);
				laneObj.mpNodes[0].dTo1stNode  = static_cast<uint32_t>(round(std::abs((proj2segment.t - 1.0)* proj2segment.length)));
				break;
			case MsgEnum::approachType::outbound:
				GeoUtils::projectPt2Line(laneObj.mpNodes[0].ptNode, laneObj.mpNodes[1].ptNode, origin, proj2segment);
				laneObj.mpNodes[0].dTo1stNode  = static_cast<uint32_t>(round(std::abs((proj2segment.t) * proj2segment.length)));
				break;
			case MsgEnum::approachType::crosswalk:
				break;
			}
		}
		// get mindist2intsectionCentralLine in centimeter
		appObj.mindist2intsectionCentralLine = (appObj.mpLanes.empty()) ? 2000 : appObj.mpLanes[0].mpNodes[0].dTo1stNode;
		for (auto& laneObj : appObj.mpLanes)
		{
			if (laneObj.mpNodes[0].dTo1stNode < appObj.mindist2intsectionCentralLine)
				appObj.mindist2intsectionCentralLine = laneObj.mpNodes[0].dTo1stNode;
		}
	}
	intObj.radius = radius;
	// set mpNodes.heading
	for (auto& appObj : intObj.mpApproaches)
	{
		if (appObj.type != MsgEnum::approachType::outbound)
		{ // order of node sequence on inbound lanes start at stop-bar towards upstream
			for (auto& laneObj : appObj.mpLanes)
			{
				for (size_t i = 1, j = laneObj.mpNodes.size(); i < j; i++)
				{
					auto& nodeObj = laneObj.mpNodes[i];
					auto& downstreamNodeObj = laneObj.mpNodes[i-1];
					nodeObj.heading = nodeObj.ptNode.direction2pt(downstreamNodeObj.ptNode);
				}
				laneObj.mpNodes[0].heading = laneObj.mpNodes[1].heading;
			}
		}
		else
		{ // order of node sequence on outbound lanes starts at cross-walk towards downstream
			for (auto& laneObj : appObj.mpLanes)
			{
				for (size_t i = 0, j = laneObj.mpNodes.size() - 1; i < j; i++)
				{
					auto& nodeObj = laneObj.mpNodes[i];
					auto& downstreamNodeObj = laneObj.mpNodes[i+1];
					nodeObj.heading = nodeObj.ptNode.direction2pt(downstreamNodeObj.ptNode);
				}
				laneObj.mpNodes.back().heading = laneObj.mpNodes[laneObj.mpNodes.size() - 2].heading;
			}
		}
	}
}

auto getBoundaryWaypoint = [](const GeoUtils::point2D_t& ptNode, const uint16_t& heading, const double& width, const bool& inbound)->GeoUtils::point2D_t
{
	GeoUtils::point2D_t waypoint;
	int32_t direction = (inbound) ? 1 : -1;
	double alpha = DsrcConstants::deg2rad(DsrcConstants::deca2unit<uint16_t>(heading));
	waypoint.x = ptNode.x + direction * static_cast<int32_t>(width * cos(alpha));
	waypoint.y = ptNode.y - direction * static_cast<int32_t>(width * sin(alpha));
	return(waypoint);
};

void LocAware::buildPolygons(void)
{
	for (auto& intObj : mpIntersection)
		LocAware::buildPolygons(intObj);
}

void LocAware::buildPolygons(NmapData::IntersectionStruct& intObj)
{ // build IntersectionPolygon & ApproachPolygon
	for (auto& appObj : intObj.mpApproaches)
	{
		if ((appObj.type == MsgEnum::approachType::crosswalk) || appObj.mpLanes.empty())
			continue;
		uint8_t crosswalkIndx = (uint8_t)((appObj.id - 1) / 2);
		if (crosswalkIndx > 4)
			continue;
		for (const auto& laneObj : appObj.mpLanes)
		{
			double width = laneObj.width * NmapData::laneWidthRatio;
			const auto& frontNode = laneObj.mpNodes.front();
			GeoUtils::point2D_t waypoint = getBoundaryWaypoint(frontNode.ptNode, frontNode.heading, width, true);
			intObj.mpPolygon.push_back(waypoint);
			waypoint = getBoundaryWaypoint(frontNode.ptNode, frontNode.heading, width, false);
			intObj.mpPolygon.push_back(waypoint);
			for (const auto& nodeObj : laneObj.mpNodes)
			{
				waypoint = getBoundaryWaypoint(nodeObj.ptNode, nodeObj.heading, width, true);
				appObj.mpPolygon.push_back(waypoint);
				waypoint = getBoundaryWaypoint(nodeObj.ptNode, nodeObj.heading, width, false);
				appObj.mpPolygon.push_back(waypoint);
			}
		}
		appObj.mpPolygon = GeoUtils::convexHullAndrew(appObj.mpPolygon);
		appObj.mpPolygonType = GeoUtils::convexcave(appObj.mpPolygon);
	}
	intObj.mpPolygon = GeoUtils::convexHullAndrew(intObj.mpPolygon);
	intObj.mpPolygonType = GeoUtils::convexcave(intObj.mpPolygon);
}
/// --- end of functions to process the intersection nmap file --- ///



/// --- start of functions to encode MAP and update decoded MAP --- ///
auto speed_mph2mps = [](const uint8_t& speed_limit)->uint16_t
{
	return((speed_limit == 0xFF) ? MsgEnum::unknown_speed :
		((speed_limit == 0) ? 0 : DsrcConstants::mph2unit<uint16_t>(speed_limit)));
};

auto speed_mps2mph = [](const uint16_t& speed_limit)->uint8_t
{
	return((speed_limit ==  MsgEnum::unknown_speed) ? 0xFF :
		((speed_limit == 0) ? 0 : static_cast<uint8_t>(round(DsrcConstants::unit2mph<uint16_t>(speed_limit)))));
};

auto IntObj2MapData = [](const NmapData::IntersectionStruct& intObj, MapData_element_t& mapData)->void
{
	if (intObj.mpApproaches.empty())
		return;
	mapData.reset();
	mapData.regionalId       = intObj.regionalId;
	mapData.id               = intObj.id;
	mapData.mapVersion       = intObj.mapVersion;
	mapData.attributes       = intObj.attributes;
	mapData.geoRef.latitude  = intObj.geoRef.latitude;
	mapData.geoRef.longitude = intObj.geoRef.longitude;
	mapData.geoRef.elevation = (intObj.attributes.test(0)) ? intObj.geoRef.elevation : MsgEnum::unknown_elevation;
	mapData.mpApproaches.resize(intObj.mpApproaches.size());
	size_t appCnt = 0;
	for (const auto& appObj : intObj.mpApproaches)
	{
		auto& appData = mapData.mpApproaches[appCnt++];
		if (appObj.mpLanes.empty())
			continue;
		appData.id          = appObj.id;
		appData.type        = appObj.type;
		appData.speed_limit = speed_mph2mps(appObj.speed_limit);
		if (std::find(mapData.speeds.begin(), mapData.speeds.end(),	appData.speed_limit) == mapData.speeds.end())
			mapData.speeds.push_back(appData.speed_limit);
		appData.mpLanes.resize(appObj.mpLanes.size());
		size_t laneCnt = 0;
		for (const auto& laneObj : appObj.mpLanes)
		{
			auto& laneData = appData.mpLanes[laneCnt++];
			laneData.id           = laneObj.id;
			laneData.type         = laneObj.type;
			laneData.attributes   = laneObj.attributes;
			laneData.width        = laneObj.width;
			laneData.controlPhase = laneObj.controlPhase;
			if (!laneObj.mpConnectTo.empty())
			{
				laneData.mpConnectTo.resize(laneObj.mpConnectTo.size());
				size_t connCnt = 0;
				for (const auto& connObj : laneObj.mpConnectTo)
				{
					auto& connData = laneData.mpConnectTo[connCnt++];
					connData.regionalId     = connObj.regionalId;
					connData.intersectionId = connObj.intersectionId;
					connData.laneId         = connObj.laneId;
					connData.laneManeuver   = connObj.laneManeuver;
				}
			}
			if (!laneObj.mpNodes.empty() && (laneObj.numpoints > 0))
			{ // inbound lane may have included way-points from its upstream outbound lane
				laneData.mpNodes.resize(laneObj.numpoints);
				GeoUtils::point2D_t prev_ptNode{0,0};
				for (size_t i_node = 0; i_node < laneObj.numpoints; i_node++)
				{
					const auto& nodeObj = laneObj.mpNodes[i_node];
					auto& nodeData = laneData.mpNodes[i_node];
					nodeData.offset_x  = nodeObj.ptNode.x - prev_ptNode.x;
					nodeData.offset_y  = nodeObj.ptNode.y - prev_ptNode.y;
					nodeData.latitude  = nodeObj.geoNode.latitude;
					nodeData.longitude = nodeObj.geoNode.longitude;
					nodeData.useXY = true;
					prev_ptNode = nodeObj.ptNode;
				}
			}
		}
	}
};

size_t LocAware::encode_mapdata_payload(void)
{
	size_t ret = 0;
	Frame_element_t dsrcFrameIn;
	dsrcFrameIn.dsrcMsgId = MsgEnum::DSRCmsgID_map;
	for (auto& intObj : mpIntersection)
	{
		intObj.mapPayload.resize(DsrcConstants::maxMsgSize);
		IntObj2MapData(intObj, dsrcFrameIn.mapData);
		dsrcFrameIn.mapData.isSingleFrame = speedLimitInLane;
		size_t payload_size = AsnJ2735Lib::encode_msgFrame(dsrcFrameIn, &intObj.mapPayload[0], intObj.mapPayload.size());
		if (payload_size > 0)
		{
			intObj.mapPayload.resize(payload_size);
			intObj.mapPayload.shrink_to_fit();
			ret++;
		}
		else
			intObj.mapPayload.clear();
	}
	return(ret);
}

uint32_t LocAware::checkMapUpdate(const uint8_t* buf, size_t size)
{ // decode MAP
	Frame_element_t dsrcFrameOut;
	if ((AsnJ2735Lib::decode_msgFrame(buf, size, dsrcFrameOut) == 0) || (dsrcFrameOut.dsrcMsgId != MsgEnum::DSRCmsgID_map))
		return(0);
	const auto& mapIn = dsrcFrameOut.mapData;
	if (mapIn.mpApproaches.empty())
		return(0);
	// check whether mapIn is the same version as that is stored in mpIntersection
	uint8_t mapVersion = LocAware::getMapVersion(mapIn.regionalId, mapIn.id);
	if (mapVersion == mapIn.mapVersion) // same version
		return(ids2id(mapIn.regionalId, mapIn.id));
	// this is either a new (mapVersion = 0) or an updated (mapVersion > 0) mapIn
	NmapData::IntersectionStruct* pIntObj = new NmapData::IntersectionStruct;
	pIntObj->regionalId = mapIn.regionalId;
	pIntObj->id = mapIn.id;
	pIntObj->mapVersion = mapIn.mapVersion;
	if (mapVersion > 0)
		pIntObj->name = LocAware::getIntersectionNameById(mapIn.regionalId, mapIn.id);
	else
	{
		std::ostringstream oss;
		oss << mapIn.regionalId << std::string("-") << mapIn.id;
		pIntObj->name = oss.str();
	}
	pIntObj->attributes = mapIn.attributes;
	pIntObj->geoRef.latitude  = mapIn.geoRef.latitude;
	pIntObj->geoRef.longitude = mapIn.geoRef.longitude;
	pIntObj->geoRef.elevation = mapIn.geoRef.elevation;
	GeoUtils::setEnuCoord(pIntObj->geoRef, pIntObj->enuCoord);
	pIntObj->mapPayload.assign(buf, buf + size);
	pIntObj->radius = 0;
	pIntObj->mpApproaches.resize(mapIn.mpApproaches.size());
	auto& speeds = pIntObj->speeds;
	size_t appCnt = 0;
	for (const auto& appData : mapIn.mpApproaches)
	{
		if (appData.mpLanes.empty())
			continue;
		auto& appObj = pIntObj->mpApproaches[appCnt++];
		appObj.id = appData.id;
		appObj.speed_limit =  speed_mps2mph(appData.speed_limit);
		if (std::find(speeds.begin(), speeds.end(),	appObj.speed_limit) == speeds.end())
			speeds.push_back(appObj.speed_limit);
		appObj.type = appData.type;
		appObj.mindist2intsectionCentralLine = 0;
		appObj.mpLanes.resize(appData.mpLanes.size());
		size_t laneCnt = 0;
		for (const auto& laneData : appData.mpLanes)
		{
			auto& laneObj = appObj.mpLanes[laneCnt++];
			laneObj.id           = laneData.id;
			laneObj.type         = laneData.type;
			laneObj.attributes   = laneData.attributes;
			laneObj.width        = laneData.width;
			laneObj.controlPhase = laneData.controlPhase;
			if (!laneData.mpConnectTo.empty())
			{
				laneObj.mpConnectTo.resize(laneData.mpConnectTo.size());
				size_t connCnt = 0;
				for (const auto& connData : laneData.mpConnectTo)
				{
					auto& connObj = laneObj.mpConnectTo[connCnt++];
					connObj.intersectionId = connData.intersectionId;
					connObj.laneId = connData.laneId;
					connObj.laneManeuver = connData.laneManeuver;
				}
			}
			if (!laneData.mpNodes.empty())
			{
				laneObj.numpoints = laneData.mpNodes.size();
				laneObj.mpNodes.resize(laneData.mpNodes.size());

				GeoUtils::point2D_t ptNode{0, 0};
				size_t nodeCnt = 0;
				for (const auto& nodeData : laneData.mpNodes)
				{
					auto& nodeObj = laneObj.mpNodes[nodeCnt];
					if (nodeData.useXY)
					{
						ptNode.x += nodeData.offset_x;
						ptNode.y += nodeData.offset_y;
						GeoUtils::enu2lla(pIntObj->enuCoord, ptNode, nodeObj.geoNode);
					}
					else
					{
						nodeObj.geoNode.latitude  = nodeData.latitude;
						nodeObj.geoNode.longitude = nodeData.longitude;
						nodeObj.geoNode.elevation = pIntObj->geoRef.elevation;
					}
					nodeCnt++;
				}
			}
		}
	}
	pIntObj->mpApproaches.resize(appCnt);
	if (saveNewMap2nmap)
		LocAware::saveNmap(*pIntObj);
	if (initiated)
	{ // link outbound lane way-points of this intersection to its downstream connected inbound lane
		LocAware::setOutbond2InboundWaypoints(*pIntObj);
		// rebuild downstream intersection polygons
		for (const auto& connId : pIntObj->mpConnIntersections)
		{
			uint16_t regionalId = (uint16_t)((connId >> 16) & 0xFFFF);
			uint16_t intersectionId = (uint16_t)(connId & 0xFFFF);
			uint8_t  intIndex = getIndexByIntersectionId(regionalId, intersectionId);
			if (intIndex == 0xFF)
				continue;
			auto& intObj = mpIntersection[intIndex];
			LocAware::setLocalOffsetAndHeading(intObj);
			LocAware::buildPolygons(intObj);
		}
		// link inbound lane way-points of this intersection to its upstream connected outbound lane
		LocAware::getOutbond2InboundWaypoints(*pIntObj);
		// build polygons for this intersection
		LocAware::setLocalOffsetAndHeading(*pIntObj);
		LocAware::buildPolygons(*pIntObj);
	}
	// add to intersection list
	LocAware::addIntersection(*pIntObj);
	delete pIntObj;
	return(ids2id(mapIn.regionalId, mapIn.id));
}

void LocAware::setSaveNewMap2nmap(const bool& option)
	{saveNewMap2nmap = option;}

void LocAware::addIntersection(const NmapData::IntersectionStruct& intObj)
{
	uint16_t regionalId = intObj.regionalId;
	uint16_t intersectionId = intObj.id;
	auto it = std::find_if(mpIntersection.begin(), mpIntersection.end(),
		[&regionalId, &intersectionId](const NmapData::IntersectionStruct& obj)
		{return((obj.id == intersectionId) && (obj.regionalId == regionalId));});
	if (it != mpIntersection.end())
		it = mpIntersection.erase(it);
	it = mpIntersection.insert(it, intObj);
	// update IndexMap
	uint8_t intIndx = (uint8_t)(it - mpIntersection.begin());
	for (auto itMap = IndexMap.begin(); itMap != IndexMap.end();)
	{
		if (((uint16_t)(((itMap->first) >> 8) & 0xFFFF) == intersectionId) && (((uint16_t)(((itMap->first) >> 24) & 0xFFFF) == regionalId)))
			itMap = IndexMap.erase(itMap);
		else
			++itMap;
	}
	uint8_t appIndx = 0;
	for (const auto& appObj : intObj.mpApproaches)
	{
		uint8_t laneIndx = 0;
		for (const auto& laneObj : appObj.mpLanes)
		{
			IndexMap[getIndexMapKey(regionalId, intersectionId, laneObj.id)] = getIndexMapValue(intIndx, appIndx, laneIndx);
			laneIndx++;
		}
		appIndx++;
	}
}
/// --- end of functions to encode MAP and update decoded MAP --- ///



/// --- start of functions to get static map data elements --- ///
bool LocAware::isInitiated(void) const
	{return(initiated);}

uint8_t LocAware::getMapVersion(const uint16_t& regionalId, const uint16_t& intersectionId) const
{
	auto it = std::find_if(mpIntersection.begin(), mpIntersection.end(),
		[&regionalId, &intersectionId](const NmapData::IntersectionStruct& obj)
		{return((obj.id == intersectionId) && (obj.regionalId == regionalId));});
	return((it != mpIntersection.end()) ? (it->mapVersion) : 0);
}

std::vector<uint32_t> LocAware::getIntersectionIds(void) const
{
	std::vector<uint32_t> ids;
	for (const auto& item : mpIntersection)
		ids.push_back(ids2id(item.regionalId, item.id));
	return(ids);
}

void LocAware::setIntersectionName(const std::string& name, const uint16_t& regionalId, const uint16_t& intersectionId)
{
	uint8_t intIndex = getIndexByIntersectionId(regionalId, intersectionId);
	if (intIndex != 0xFF)
		mpIntersection[intIndex].name = name;
}

std::string LocAware::getIntersectionNameById(const uint16_t& regionalId, const uint16_t& intersectionId) const
{
	auto it = std::find_if(mpIntersection.begin(), mpIntersection.end(),
		[&regionalId, &intersectionId](const NmapData::IntersectionStruct& obj)
		{return((obj.id == intersectionId) && (obj.regionalId == regionalId));});
	return ((it != mpIntersection.end())	? (it->name) : std::string());
}

uint32_t LocAware::getIntersectionIdByName(const std::string& name) const
{
	auto it = std::find_if(mpIntersection.begin(), mpIntersection.end(),
		[&name](const NmapData::IntersectionStruct& obj){return(obj.name.compare(name) == 0);});
	return((it != mpIntersection.end()) ? ids2id(it->regionalId, it->id) : 0);
}

uint8_t LocAware::getIndexByIntersectionId(const uint16_t& regionalId, const uint16_t& intersectionId) const
{
	auto it = std::find_if(mpIntersection.begin(), mpIntersection.end(),
		[&regionalId, &intersectionId](const NmapData::IntersectionStruct& obj)
		{return((obj.id == intersectionId) && (obj.regionalId == regionalId));});
	return((uint8_t)((it != mpIntersection.end()) ? (it - mpIntersection.begin()) : 0xFF));
}

std::string LocAware::getIntersectionNameByIndex(const uint8_t& intersectionIndex) const
{
	return((intersectionIndex < mpIntersection.size()) ? mpIntersection[intersectionIndex].name : std::string());
}

uint8_t LocAware::getIndexByApproachId(const uint8_t& intersectionIndx, const uint8_t& approachId) const
{
	const auto& approaches = mpIntersection[intersectionIndx].mpApproaches;
	auto it = std::find_if(approaches.begin(), approaches.end(),
		[&approachId](const NmapData::ApproachStruct& obj) {return(obj.id == approachId);});
	return((uint8_t)((it != approaches.end()) ? (it - approaches.begin()) : 0xFF));
}

uint8_t LocAware::getControlPhaseByLaneId(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId) const
{
	auto inds = LocAware::getIndexesByIds(regionalId, intersectionId, laneId);
	return((!inds.empty()) ? mpIntersection[inds[0]].mpApproaches[inds[1]].mpLanes[inds[2]].controlPhase : 0);
}

uint8_t LocAware::getControlPhaseByAprochId(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& approachId) const
{
	uint8_t intIndex = getIndexByIntersectionId(regionalId, intersectionId);
	if ((intIndex == 0xFF) || (size_t)approachId > mpIntersection[intIndex].mpApproaches.size())
		return(0);
	auto& lanes = mpIntersection[intIndex].mpApproaches[approachId - 1].mpLanes;
	auto it = std::find_if(lanes.begin(), lanes.end(),
		[](const NmapData::LaneStruct& obj){return(obj.attributes.test(1) == true);});
	return((it == lanes.end()) ? 0 : it->controlPhase);
}

uint8_t LocAware::getControlPhaseByIds(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& approachId, const uint8_t& laneId) const
{
	if ((approachId == 0) && (laneId == 0))
		return(0);
	return((laneId > 0) ? LocAware::getControlPhaseByLaneId(regionalId, intersectionId, laneId)
		: LocAware::getControlPhaseByAprochId(regionalId, intersectionId, approachId));
}

uint8_t LocAware::getApproachIdByLaneId(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId) const
{
	auto inds = LocAware::getIndexesByIds(regionalId, intersectionId, laneId);
	return((!inds.empty()) ? mpIntersection[inds[0]].mpApproaches[inds[1]].id : 0);
}

uint32_t LocAware::getLaneLength(const uint16_t& regionalId, const uint16_t& intersectionId, const uint8_t& laneId) const
{
	auto inds = LocAware::getIndexesByIds(regionalId, intersectionId, laneId);
	return ((!inds.empty()) ? mpIntersection[inds[0]].mpApproaches[inds[1]].mpLanes[inds[2]].mpNodes.back().dTo1stNode : 0);
}

GeoUtils::geoRefPoint_t LocAware::getIntersectionRefPoint(const uint8_t& intersectionIndx) const
{
	return(mpIntersection[intersectionIndx].geoRef);
}

uint8_t LocAware::getLaneIdByIndexes(const uint8_t& intersectionIndx, const uint8_t& approachIndx, const uint8_t& laneIndx) const
{
	return(((intersectionIndx < mpIntersection.size())
		&& (approachIndx < mpIntersection[intersectionIndx].mpApproaches.size())
		&& (laneIndx < mpIntersection[intersectionIndx].mpApproaches[approachIndx].mpLanes.size())) ?
		mpIntersection[intersectionIndx].mpApproaches[approachIndx].mpLanes[laneIndx].id : 0);
}

std::vector<uint8_t> LocAware::getMapdataPayload(const uint16_t& regionalId, const uint16_t& intersectionId) const
{
	auto it = std::find_if(mpIntersection.begin(), mpIntersection.end(),
		[&regionalId, &intersectionId](const NmapData::IntersectionStruct& obj)
		{return((obj.id == intersectionId) && (obj.regionalId == regionalId));});
	return((it != mpIntersection.end()) ? it->mapPayload : std::vector<uint8_t>());
}

bool LocAware::getSpeedLimits(std::vector<uint8_t>& speedLimits, const uint16_t& regionalId, const uint16_t& intersectionId) const
{
	auto it = std::find_if(mpIntersection.begin(), mpIntersection.end(),
		[&regionalId, &intersectionId](const NmapData::IntersectionStruct& obj)
		{return((obj.id == intersectionId) && (obj.regionalId == regionalId));});
	if (it == mpIntersection.end())
		return(false);
	for (auto& appObj : it->mpApproaches)
	{ /// one inbound approach could have multiple control phases (e.g., straight through & protected left-turn)
		std::vector<uint8_t> controlPhases;
		if ((appObj.type == MsgEnum::approachType::inbound) && !appObj.mpLanes.empty())
		{
			for (auto& laneObj : appObj.mpLanes)
			{
				if ((laneObj.controlPhase > 0) && (std::find(controlPhases.begin(), controlPhases.end(), laneObj.controlPhase) == controlPhases.end()))
					controlPhases.push_back(laneObj.controlPhase);
			}
			for (auto& phase : controlPhases)
				speedLimits[phase - 1] = appObj.speed_limit;
		}
	}
	return(true);
}
/// --- end of functions to get static map data elements --- ///



/// --- start of functions to locate BSM on MAP --- ///
auto isPointNearIntersection = [](const NmapData::IntersectionStruct& intObj, const GeoUtils::geoPoint_t& geoPoint)->bool
{ // check whether geoPt is inside radius of an intersection
	GeoUtils::point3D_t ptENU;
	GeoUtils::lla2enu(intObj.enuCoord, geoPoint, ptENU);
	return(sqrt(ptENU.x * ptENU.x + ptENU.y * ptENU.y) <= DsrcConstants::hecto2unit<int32_t>(intObj.radius));
};

auto isPointInsideIntersectionBox = [](const NmapData::IntersectionStruct& intObj, const GeoUtils::point2D_t& ptENU)->bool
	{return(GeoUtils::isPointInsidePolygon(intObj.mpPolygon, ptENU));};

auto isPointOnApproach = [](const NmapData::ApproachStruct& appObj, const GeoUtils::point2D_t& ptENU)->bool
	{return (GeoUtils::isPointInsidePolygon(appObj.mpPolygon, ptENU));};

auto onApproaches = [](const NmapData::IntersectionStruct& intObj, const GeoUtils::point2D_t& ptENU)->std::vector<uint8_t>
{ // also do this when geoPoint is near the intersection (check first with isPointNearIntersection)
	std::vector<uint8_t> ret;
	for (auto it = intObj.mpApproaches.begin(); it != intObj.mpApproaches.end(); ++it)
	{
		if (!it->mpPolygon.empty() && isPointOnApproach(*it, ptENU))
			ret.push_back(static_cast<uint8_t>(it - intObj.mpApproaches.begin()));
	}
	return(ret);
};

auto getHeadingDifference = [](const uint16_t& nodeHeading, const double& ptHeading)->double
{ // headingDiff in [-180..180];
	double d = ptHeading - DsrcConstants::deca2unit<uint16_t>(nodeHeading);
	if (d > 180.0)
		d -= 360.0;
	else if (d < -180.0)
		d += 360.0;
	return(d);
};

auto getHeadingErrorBound = [](const double& ptSpeed)->double
{
	return((ptSpeed < NmapData::lowSpeedThreshold) ? NmapData::headingErrorBoundLowSpeed : NmapData::headingErrorBoundNormal);
};

auto getIdxByLocationType = [](const std::vector<GeoUtils::vehicleTracking_t>& aVehicleTrackingState)->int
{
	std::vector<int> indexes(3, -1);
	for (int i = 0, j = (int)aVehicleTrackingState.size(); i < j; i++)
	{
		const auto& item = aVehicleTrackingState[i];
		if ((item.intsectionTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::onInbound) && (indexes[0] == -1))
			indexes[0] = i;
		else if ((item.intsectionTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::onOutbound) && (indexes[1] == -1))
			indexes[1] = i;
		else if ((item.intsectionTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::insideIntersectionBox) && (indexes[2] == -1))
			indexes[2] = i;
	}
	return((indexes[0] != -1)? indexes[0] : ((indexes[1] != -1)? indexes[1] : ((indexes[2] != -1)? indexes[2] : -1)));
};

auto getIdx4minLatApproch = [](const std::vector<GeoUtils::vehicleTracking_t>& aApproachTrackingState)->int
{
	double dmax = 1000.0; // in centimetres
	int idx = -1;
	for (size_t i = 0, j = aApproachTrackingState.size(); i < j; i++)
	{
		const auto& item = aApproachTrackingState[i];
		double d = std::abs(item.laneProj.proj2segment.d);
		if (d < dmax)
		{
			idx = static_cast<int>(i);
			dmax = d;
		}
	}
	return(idx);
};

auto getIdx4minLatLane = [](const std::vector<GeoUtils::laneTracking_t>& aLaneTrackingState)->int
{
	double dmax = 1000.0; // in centimetres
	int idx = -1;
	for (size_t i = 0, j = aLaneTrackingState.size(); i < j; i++)
	{
		const auto& item = aLaneTrackingState[i];
		double d = std::abs(item.laneProj.proj2segment.d);
		if ((item.vehicleLaneStatus == MsgEnum::laneLocType::inside) && (d < dmax))
		{
			idx = static_cast<int>(i);
			dmax = d;
		}
	}
	return(idx);
};

auto getIdx4minLatNode = [](const std::vector<GeoUtils::laneProjection_t>& aProj2Lane, const uint16_t& laneWidth)->int
{
	int idx = -1;
	double dmax	= (double)laneWidth;
	for (size_t i = 0, j = aProj2Lane.size(); i < j; i++)
	{
		const auto& item = aProj2Lane[i];
		double d = std::abs(item.proj2segment.d);
		if ((item.proj2segment.t >= 0.0) && (item.proj2segment.t <= 1.0) && (d < dmax))
		{
			idx = static_cast<int>(i);
			dmax = d;
		}
	}
	return(idx);
};

auto getIdx4specicalCase = [](const std::vector<GeoUtils::laneProjection_t>& aProj2Lane, const uint16_t& laneWidth)->int
{
	double dwidth = (double)laneWidth;
	std::vector< std::pair<size_t, double> > candidates;
	for (size_t i = 0, j = aProj2Lane.size() - 1; i < j; i++)
	{
		const auto& item = aProj2Lane[i];
		const auto& nextItem = aProj2Lane[i+1];
		double d1 = std::abs(item.proj2segment.d);
		double d2 = std::abs(nextItem.proj2segment.d);
		if ((item.proj2segment.t > 1.0) && (d1 < dwidth) && (nextItem.proj2segment.t < 0.0) && (d2 < dwidth))
		{
			if (d1 < d2)
				candidates.push_back(std::make_pair(i, d1));
			else
				candidates.push_back(std::make_pair(i+1, d2));
		}
	}
	if (candidates.empty())
		return(-1);
	auto it = std::min_element(candidates.begin(), candidates.end(),
		[](const std::pair<size_t, double>& s1, const std::pair<size_t, double>& s2){return(s1.second < s2.second);});
	return((it != candidates.end()) ? (int)(it->first) : -1);
};

auto projectPt2Lane = [](const NmapData::LaneStruct& laneObj, const MsgEnum::approachType& type,
	const GeoUtils::point2D_t& ptENU, const GeoUtils::motion_t& motionState)->GeoUtils::laneTracking_t
{
	double headingErrorBound = getHeadingErrorBound(motionState.speed);
	std::vector<GeoUtils::laneProjection_t> aProj2Lane;
	GeoUtils::laneProjection_t proj2lane;

	if (type == MsgEnum::approachType::inbound)
	{
		for (uint8_t i = (uint8_t)(laneObj.mpNodes.size() - 1); i > 0; i--)
		{
			const auto& fromNodeObj = laneObj.mpNodes[i];
			const auto& toNodeObj   = laneObj.mpNodes[i-1];
			if (std::abs(getHeadingDifference(fromNodeObj.heading, motionState.heading)) > headingErrorBound)
				continue;
			proj2lane.nodeIndex = i;
			GeoUtils::projectPt2Line(fromNodeObj.ptNode, toNodeObj.ptNode, ptENU, proj2lane.proj2segment);
			aProj2Lane.push_back(proj2lane);
		}
	}
	else
	{ // outbound
		for (uint8_t i = 0, j = (uint8_t)(laneObj.mpNodes.size() - 1); i < j; i++)
		{
			const auto& fromNodeObj = laneObj.mpNodes[i];
			const auto& toNodeObj   = laneObj.mpNodes[i+1];
			if (std::abs(getHeadingDifference(fromNodeObj.heading, motionState.heading)) > headingErrorBound)
				continue;
			proj2lane.nodeIndex = i;
			GeoUtils::projectPt2Line(fromNodeObj.ptNode, toNodeObj.ptNode, ptENU, proj2lane.proj2segment);
			aProj2Lane.push_back(proj2lane);
		}
	}

	GeoUtils::laneTracking_t laneTrackingState{MsgEnum::laneLocType::outside,{0, {0.0, 0.0, 0.0}}};
	if (aProj2Lane.empty())
		return(laneTrackingState);
	if (aProj2Lane.front().proj2segment.t < 0)
	{
		laneTrackingState.vehicleLaneStatus = MsgEnum::laneLocType::approaching;
		laneTrackingState.laneProj = aProj2Lane.front();
		return(laneTrackingState);
	}
	if (aProj2Lane.back().proj2segment.t > 1)
	{
		laneTrackingState.vehicleLaneStatus = MsgEnum::laneLocType::leaving;
		laneTrackingState.laneProj = aProj2Lane.back();
		return(laneTrackingState);
	}
	// get index of the lane segment on which projection_t.t is between [0,1],
	// projection_t.d is within laneWidth, and has the minimum projection_t.d
	// among all success projected segments
	int idx = getIdx4minLatNode(aProj2Lane, laneObj.width);
	if (idx >= 0)
	{
		laneTrackingState.vehicleLaneStatus = MsgEnum::laneLocType::inside;
		laneTrackingState.laneProj = aProj2Lane[idx];
		return(laneTrackingState);
	}
	// special case
	idx = getIdx4specicalCase(aProj2Lane, laneObj.width);
	if (idx >= 0)
	{
		laneTrackingState.vehicleLaneStatus = MsgEnum::laneLocType::inside;
		laneTrackingState.laneProj = aProj2Lane[idx];
		return(laneTrackingState);
	}
	return(laneTrackingState);
};

auto locateVehicleOnApproach = [](const NmapData::ApproachStruct& appObj, const GeoUtils::point2D_t& ptENU,
	const GeoUtils::motion_t& motionState, GeoUtils::vehicleTracking_t& vehicleTrackingState)->bool
{
	vehicleTrackingState.reset();
	std::vector<GeoUtils::laneTracking_t> aLaneTrackingState;
	std::vector<size_t> aIndex;
	// one record per lane regardless whether the vehicle is on the lane or not
	for (auto it =  appObj.mpLanes.begin(); it != appObj.mpLanes.end(); ++it)
	{
		auto laneTrackingState = projectPt2Lane(*it, appObj.type, ptENU, motionState);
		if (laneTrackingState.vehicleLaneStatus != MsgEnum::laneLocType::outside)
		{
			aLaneTrackingState.push_back(laneTrackingState);
			aIndex.push_back(it - appObj.mpLanes.begin());
		}
	}
	if (aLaneTrackingState.empty())
		return(false);
	// when project to multiple lanes, find the lane with minimum distance away from it
	int idx = getIdx4minLatLane(aLaneTrackingState);
	if (idx >= 0)
	{
		vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus =
			(appObj.type == MsgEnum::approachType::inbound) ? MsgEnum::mapLocType::onInbound : MsgEnum::mapLocType::onOutbound;
		vehicleTrackingState.intsectionTrackingState.laneIndex = static_cast<uint8_t>(aIndex[idx]);
		vehicleTrackingState.laneProj = aLaneTrackingState[idx].laneProj;
		return(true);
	}
	return(false);
};

std::vector<uint8_t> LocAware::nearedIntersections(const GeoUtils::geoPoint_t& geoPoint) const
{
	std::vector<uint8_t> ret;
	for (auto it = mpIntersection.begin(); it != mpIntersection.end(); ++it)
	{
		GeoUtils::geoPoint_t pt{geoPoint.latitude, geoPoint.longitude, DsrcConstants::deca2unit<int32_t>(it->geoRef.elevation)};
		if (isPointNearIntersection(*it, pt))
			ret.push_back(static_cast<uint8_t>(it - mpIntersection.begin()));
	}
	return(ret);
}

bool LocAware::isOutboundConnect2Inbound(const NmapData::ConnectStruct& connObj, const GeoUtils::geoPoint_t& geoPoint,
	const GeoUtils::motion_t& motionState, GeoUtils::vehicleTracking_t& vehicleTrackingState) const
{
	std::vector<uint8_t> connectToindex = LocAware::getIndexesByIds(connObj.regionalId, connObj.intersectionId, connObj.laneId);
	if (!connectToindex.empty())
	{
		const auto& intObj = mpIntersection[connectToindex[0]];
		const auto& appObj = intObj.mpApproaches[connectToindex[1]];
		if (appObj.type == MsgEnum::approachType::inbound)
		{	// convert geoPoint to ptENU at connectTo intersection
			GeoUtils::geoPoint_t pt{geoPoint.latitude, geoPoint.longitude, DsrcConstants::deca2unit<int32_t>(intObj.geoRef.elevation)};
			GeoUtils::point2D_t ptENU;
			GeoUtils::lla2enu(intObj.enuCoord, pt, ptENU);
			// check whether ptENU is onInbound
			if (isPointOnApproach(appObj, ptENU) && locateVehicleOnApproach(appObj, ptENU, motionState, vehicleTrackingState))
			{
				vehicleTrackingState.intsectionTrackingState.intersectionIndex = connectToindex[0];
				vehicleTrackingState.intsectionTrackingState.approachIndex = connectToindex[1];
				return(true);
			}
		}
	}
	return(false);
}

auto getPtDist2egress = [](const NmapData::ApproachStruct& appObj, const GeoUtils::point2D_t& ptENU)->double
{ // project ptENU onto the closest lane segment on egress approach
	double dminimum = 2000.0; // in centimetres
	GeoUtils::projection_t proj2segment;

	for (const auto& laneObj : appObj.mpLanes)
	{
		GeoUtils::projectPt2Line(laneObj.mpNodes[0].ptNode, laneObj.mpNodes[1].ptNode, ptENU, proj2segment);
		double d = proj2segment.t * proj2segment.length;
		if ((d <= 0) && (std::abs(d) < dminimum))
			dminimum = std::abs(d);
	}
	return(dminimum);
};

bool LocAware::locateVehicleInMap(const GeoUtils::connectedVehicle_t& cv, GeoUtils::vehicleTracking_t& cvTrackingState) const
{
	cvTrackingState.reset();
	if (!cv.isVehicleInMap)
	{ // find target intersections that geoPoint is on
		auto intersectionList = LocAware::nearedIntersections(cv.geoPoint);
		if (intersectionList.empty())
			return(false);
		std::vector<GeoUtils::vehicleTracking_t> aVehicleTrackingState; // at most one record per intersection
		for (const auto& intIndx : intersectionList)
		{ // convert cv.geoPoint to ptENU
			const auto& intObj = mpIntersection[intIndx];
			GeoUtils::geoPoint_t pt{cv.geoPoint.latitude, cv.geoPoint.longitude, DsrcConstants::deca2unit<int32_t>(intObj.geoRef.elevation)};
			GeoUtils::point2D_t ptENU;
			GeoUtils::lla2enu(intObj.enuCoord, pt, ptENU);
			GeoUtils::vehicleTracking_t vehicleTrackingState;
			vehicleTrackingState.reset();
			// check whether ptENU is inside intersection box first
			if (isPointInsideIntersectionBox(intObj, ptENU))
			{
				vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus = MsgEnum::mapLocType::insideIntersectionBox;
				vehicleTrackingState.intsectionTrackingState.intersectionIndex = intIndx;
				aVehicleTrackingState.push_back(vehicleTrackingState);
				continue;
			}
			// find target approaches that ptENU is on
			auto approachList = onApproaches(intObj, ptENU);
			if (approachList.empty())
				continue;
			std::vector<GeoUtils::vehicleTracking_t> aApproachTrackingState; // at most one record per approach
			// find target lanes that ptENU is on
			for (const auto& appIndx : approachList)
			{
				const auto& appObj = intObj.mpApproaches[appIndx];
				if (!appObj.mpLanes.empty() && locateVehicleOnApproach(appObj, ptENU, cv.motionState, vehicleTrackingState))
				{
					vehicleTrackingState.intsectionTrackingState.intersectionIndex = intIndx;
					vehicleTrackingState.intsectionTrackingState.approachIndex = appIndx;
					aApproachTrackingState.push_back(vehicleTrackingState);
				}
			}
			if (!aApproachTrackingState.empty())
			{ // when project to multiple approaches, find the approach/lane with minimum distance away from it
				int idx = getIdx4minLatApproch(aApproachTrackingState);
				if (idx >= 0)
					aVehicleTrackingState.push_back(aApproachTrackingState[idx]);
			}
		}
		if (aVehicleTrackingState.empty())
			return(false);
		// when ptENU in multiple intersections, find the intersection in the order of onInbound, onOutbound, insideIntersectionBox
		int idx = getIdxByLocationType(aVehicleTrackingState);
		if (idx >= 0)
		{
			cvTrackingState = aVehicleTrackingState[idx];
			return(true);
		}
		return(false);
	}

	// vehicle was already in map, so intersectionIndex is known
	const auto& prevIntTrackingState = cv.vehicleTrackingState.intsectionTrackingState;
	const auto& intersectionIndex = prevIntTrackingState.intersectionIndex;
	const auto& intObj = mpIntersection[intersectionIndex];
	// convert cv.geoPoint to ptENU
	GeoUtils::geoPoint_t pt{cv.geoPoint.latitude, cv.geoPoint.longitude, DsrcConstants::deca2unit<int32_t>(intObj.geoRef.elevation)};
	GeoUtils::point2D_t ptENU;
	GeoUtils::lla2enu(intObj.enuCoord, pt, ptENU);
	// action based on the previous vehicleIntersectionStatus
	if (prevIntTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::insideIntersectionBox)
	{ // vehicle was initiated inside the intersection box, so approachIndex & laneIndex are unknown.
		// next vehicleIntersectionStatus:
		//  1. remains insideIntersectionBox
		//  2. entered one of the outbound lanes (onOutbound)
		//  3. moved outside the MAP

		// check whether vehicle remains insideIntersectionBox
		if (isPointInsideIntersectionBox(intObj, ptENU))
		{ // no change on vehicleTracking_t, return
			cvTrackingState = cv.vehicleTrackingState;
			return(true);
		}
		// vehicle is not insideIntersectionBox, check whether it is on an outbound lane (onOutbound)
		std::vector<GeoUtils::vehicleTracking_t> aApproachTrackingState;
		auto approachList = onApproaches(intObj, ptENU);
		if (!approachList.empty())
		{
			for (const auto& appIndx: approachList)
			{
				const auto& appObj = intObj.mpApproaches[appIndx];
				GeoUtils::vehicleTracking_t vehicleTrackingState;
				vehicleTrackingState.reset();
				if ((appObj.type == MsgEnum::approachType::outbound) && locateVehicleOnApproach(appObj, ptENU, cv.motionState, vehicleTrackingState))
				{
					vehicleTrackingState.intsectionTrackingState.intersectionIndex = intersectionIndex;
					vehicleTrackingState.intsectionTrackingState.approachIndex = appIndx;
					aApproachTrackingState.push_back(vehicleTrackingState);
				}
			}
		}
		// find the approach with minimum distance away from the center of the lane
		int idx = aApproachTrackingState.empty() ? -1 : getIdx4minLatApproch(aApproachTrackingState);
		if (idx < 0) // vehicle is not insideIntersectionBox nor onOutbound, reset tracking to outside
			return(false);
		// vehicle is onOutbound, check whether it is on a connecting inbound lane (onInbound)
		const auto& vehicleTrackingState = aApproachTrackingState[idx];
		const auto& appObj = intObj.mpApproaches[vehicleTrackingState.intsectionTrackingState.approachIndex];
		const auto& connectTo = appObj.mpLanes[vehicleTrackingState.intsectionTrackingState.laneIndex].mpConnectTo;
		if (!connectTo.empty() && connectTo.size() == 1)
		{
			GeoUtils::vehicleTracking_t inboundTrackingState;
			inboundTrackingState.reset();
			if (LocAware::isOutboundConnect2Inbound(connectTo[0], cv.geoPoint, cv.motionState, inboundTrackingState))
			{
				cvTrackingState = inboundTrackingState;
				return(true);
			}
		}
		// vehicle remains onOutbound
		cvTrackingState = vehicleTrackingState;
		return(true);
	}
	else if (prevIntTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::onInbound)
	{ // vehicle was onInbound, so approachIndex & laneIndex are known.
		// next vehicleIntersectionStatus:
		//  1. remains onInbound
		//  2. entered intersection box (atIntersectionBox)
		//  3. entered its connecting outbound lane (onOutbound)
		//  4. moved outside the MAP
		const auto& approachIndex = prevIntTrackingState.approachIndex;
		const auto& laneIndex = prevIntTrackingState.laneIndex;
		const auto& appObj  = intObj.mpApproaches[approachIndex];
		const auto& laneObj = appObj.mpLanes[laneIndex];
		// check whether vehicle remains onInbound
		GeoUtils::vehicleTracking_t vehicleTrackingState;
		vehicleTrackingState.reset();
		if (isPointOnApproach(appObj, ptENU) && locateVehicleOnApproach(appObj, ptENU, cv.motionState, vehicleTrackingState))
		{
			vehicleTrackingState.intsectionTrackingState.intersectionIndex = intersectionIndex;
			vehicleTrackingState.intsectionTrackingState.approachIndex = approachIndex;
			cvTrackingState = vehicleTrackingState;
			if (vehicleTrackingState.intsectionTrackingState.laneIndex != laneIndex)
			{
				if (isPointInsideIntersectionBox(intObj, ptENU))
				{ // the vehicle is atIntersectionBox
					cvTrackingState.intsectionTrackingState = prevIntTrackingState;
					cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus = MsgEnum::mapLocType::atIntersectionBox;
					//  update cvTrackingState.laneProj
					cvTrackingState.laneProj.nodeIndex = 1;
					GeoUtils::projectPt2Line(laneObj.mpNodes[1].ptNode, laneObj.mpNodes[0].ptNode, ptENU, cvTrackingState.laneProj.proj2segment);
				}
				else if (std::abs(vehicleTrackingState.laneProj.proj2segment.d) >= std::abs(cv.vehicleTrackingState.laneProj.proj2segment.d) / 2.0)
				{
					auto laneTrackingState = projectPt2Lane(laneObj, appObj.type, ptENU, cv.motionState);
					if (laneTrackingState.vehicleLaneStatus == MsgEnum::laneLocType::inside)
					{ // maintain the lane
						cvTrackingState.intsectionTrackingState = prevIntTrackingState;
						cvTrackingState.laneProj = laneTrackingState.laneProj;
					}
				}
			}
			return(true);
		}
		// vehicle is not onInbound, check whether it is on its connecting outbound lane (onOutbound) at the same intersection
		std::vector<uint8_t> connAppIndex;
		for (const auto& connObj : laneObj.mpConnectTo)
		{
			auto connectToindex = LocAware::getIndexesByIds(connObj.regionalId, connObj.intersectionId, connObj.laneId);
			if (!connectToindex.empty())
				connAppIndex.push_back(connectToindex[1]);
		}
		std::vector<GeoUtils::vehicleTracking_t> aApproachTrackingState;
		auto approachList = onApproaches(intObj, ptENU);
		if (!approachList.empty())
		{
			for (const auto& appIndx : approachList)
			{
				const auto& connAppObj = intObj.mpApproaches[appIndx];
				if ((connAppObj.type == MsgEnum::approachType::outbound)
						&& (std::find(connAppIndex.begin(), connAppIndex.end(), appIndx) != connAppIndex.end())
						&& locateVehicleOnApproach(connAppObj, ptENU, cv.motionState, vehicleTrackingState))
				{
					vehicleTrackingState.intsectionTrackingState.intersectionIndex = intersectionIndex;
					vehicleTrackingState.intsectionTrackingState.approachIndex = appIndx;
					aApproachTrackingState.push_back(vehicleTrackingState);
				}
			}
		}
		// find the approach with minimum distance away from the center of the lane
		int idx = aApproachTrackingState.empty() ? -1 : getIdx4minLatApproch(aApproachTrackingState);
		if (idx >= 0)
		{ // vehicle is onOutbound, check whether is is on connecting inbound lane of the next intersection (onInbound)
			const auto& outboundTrackingState = aApproachTrackingState[idx];
			const auto& outboundAppObj = intObj.mpApproaches[outboundTrackingState.intsectionTrackingState.approachIndex];
			const auto& connectTo = outboundAppObj.mpLanes[outboundTrackingState.intsectionTrackingState.laneIndex].mpConnectTo;
			if (!connectTo.empty() && connectTo.size() == 1)
			{
				GeoUtils::vehicleTracking_t inboundTrackingState;
				inboundTrackingState.reset();
				if (LocAware::isOutboundConnect2Inbound(connectTo[0], cv.geoPoint, cv.motionState, inboundTrackingState))
				{
					cvTrackingState = inboundTrackingState;
					return(true);
				}
			}
			// vehicle remains onOutbound (can't find connecting inbound lane)
			cvTrackingState = outboundTrackingState;
			return(true);
		}
		// vehicle is not onInbound nor onOutbound; project ptENU onto the closest lane segment on which the vehicle enters the intersection box
		GeoUtils::projection_t proj2segment;
		GeoUtils::projectPt2Line(laneObj.mpNodes[1].ptNode, laneObj.mpNodes[0].ptNode, ptENU, proj2segment);
		double distInto = proj2segment.t * proj2segment.length - laneObj.mpNodes[1].dTo1stNode;
		if (isPointInsideIntersectionBox(intObj, ptENU) || (std::abs(distInto) < laneObj.mpNodes[0].dTo1stNode / 2.0))
		{ // if inside intersection polygon or distance into is less than gapDist, set vehicleIntersectionStatus to atIntersectionBox
			cvTrackingState.intsectionTrackingState = prevIntTrackingState;
			cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus = MsgEnum::mapLocType::atIntersectionBox;
			//  update cvTrackingState.laneProj
			cvTrackingState.laneProj.nodeIndex = 1;
			cvTrackingState.laneProj.proj2segment = proj2segment;
			return(true);
		}
		return(false);
	}
	else if (prevIntTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::onOutbound)
	{ // vehicle was onOutbound, so approachIndex & laneIndex are known.
		// next vehicleIntersectionStatus:
		//  1. switch to inbound lane to the downstream intersection (onInbound)
		//  2. remain onOutbound
		//  3. outside (finished onOutbound and there is no downstream intersections)
		const auto& approachIndex = prevIntTrackingState.approachIndex;
		const auto& laneIndex = prevIntTrackingState.laneIndex;
		const auto& appObj  = intObj.mpApproaches[approachIndex];
		const auto& laneObj = appObj.mpLanes[laneIndex];
		const auto& connectTo = laneObj.mpConnectTo;
		// vehicle onOutbound, check whether the vehicle is on connecting inbound lane (onInbound)
		if (!connectTo.empty() && connectTo.size() == 1)
		{
			GeoUtils::vehicleTracking_t inboundTrackingState;
			inboundTrackingState.reset();
			if (LocAware::isOutboundConnect2Inbound(connectTo[0], cv.geoPoint, cv.motionState, inboundTrackingState))
			{
				cvTrackingState = inboundTrackingState;
				return(true);
			}
		}
		// vehicle not entered onInbound, check whether it remains onOutbound
		GeoUtils::vehicleTracking_t vehicleTrackingState;
		vehicleTrackingState.reset();
		if (isPointOnApproach(appObj, ptENU) && locateVehicleOnApproach(appObj, ptENU, cv.motionState, vehicleTrackingState))
		{ // remains onOutbound
			vehicleTrackingState.intsectionTrackingState.intersectionIndex = intersectionIndex;
			vehicleTrackingState.intsectionTrackingState.approachIndex = approachIndex;
			cvTrackingState = vehicleTrackingState;
			return(true);
		}
		return(false); // outside
	}
	else
	{ // vehicle was atIntersectionBox, so approachIndex & laneIndex are known.
		// next vehicleIntersectionStatus:
		//  1. remains atIntersectionBox
		//  2. entered one of the outbound lanes (onOutbound)
		//  3. onInbound - due to GPS overshooting when stopped near the stop-bar, need to give the chance for correction
		//  4. outside a MAP
		const auto& approachIndex = prevIntTrackingState.approachIndex;
		const auto& laneIndex = prevIntTrackingState.laneIndex;
		const auto& appObj  = intObj.mpApproaches[approachIndex];
		const auto& laneObj = appObj.mpLanes[laneIndex];
		// check whether vehicle is on connecting outbound lane (onOutbound)
		std::vector<uint8_t> connAppIndex;
		for (const auto& connObj : laneObj.mpConnectTo)
		{
			auto connectToindex = LocAware::getIndexesByIds(connObj.regionalId, connObj.intersectionId, connObj.laneId);
			if (!connectToindex.empty())
				connAppIndex.push_back(connectToindex[1]);
		}
		std::vector<GeoUtils::vehicleTracking_t> aApproachTrackingState;
		std::vector<double> dist2egress;
		auto approachList = onApproaches(intObj, ptENU);
		if (!approachList.empty())
		{
			for (const auto& appIndx : approachList)
			{
				const auto& connAppObj = intObj.mpApproaches[appIndx];
				if ((connAppObj.type == MsgEnum::approachType::outbound)
					&& (std::find(connAppIndex.begin(), connAppIndex.end(), appIndx) != connAppIndex.end()))
				{
					GeoUtils::vehicleTracking_t vehicleTrackingState;
					vehicleTrackingState.reset();
					if (locateVehicleOnApproach(connAppObj, ptENU, cv.motionState, vehicleTrackingState))
					{
						vehicleTrackingState.intsectionTrackingState.intersectionIndex = intersectionIndex;
						vehicleTrackingState.intsectionTrackingState.approachIndex = appIndx;
						aApproachTrackingState.push_back(vehicleTrackingState);
					}
					double d = getPtDist2egress(connAppObj, ptENU);
					if (d < intObj.mpApproaches[appIndx].mindist2intsectionCentralLine / 2.0)
					 dist2egress.push_back(d);
				}
			}
		}
		// find the approach with minimum distance away from the center of the lane
		int idx = aApproachTrackingState.empty() ? -1 : getIdx4minLatApproch(aApproachTrackingState);
		if (idx >= 0)
		{ // vehicle onOutbound, check whether the vehicle is on connecting inbound lane (onInbound)
			const auto& outboundTrackingState   = aApproachTrackingState[idx];
			const auto& outboundAppObj = intObj.mpApproaches[outboundTrackingState.intsectionTrackingState.approachIndex];
			const auto& connectTo = outboundAppObj.mpLanes[outboundTrackingState.intsectionTrackingState.laneIndex].mpConnectTo;
			if (!connectTo.empty() && connectTo.size() == 1)
			{
				GeoUtils::vehicleTracking_t inboundTrackingState;
				inboundTrackingState.reset();
				if (LocAware::isOutboundConnect2Inbound(connectTo[0], cv.geoPoint, cv.motionState, inboundTrackingState))
				{
					cvTrackingState = inboundTrackingState;
					return(true);
				}
			}
			// vehicle onOutbound
			cvTrackingState = outboundTrackingState;
			return(true);
		}
		// vehicle is not onOutbound, check whether ptENU is on the approach it entered the intersection box (onInbound)
		GeoUtils::vehicleTracking_t vehicleTrackingState;
		vehicleTrackingState.reset();
		if (isPointOnApproach(appObj, ptENU) && locateVehicleOnApproach(appObj, ptENU, cv.motionState, vehicleTrackingState))
		{
			vehicleTrackingState.intsectionTrackingState.intersectionIndex = intersectionIndex;
			vehicleTrackingState.intsectionTrackingState.approachIndex = approachIndex;
			cvTrackingState = vehicleTrackingState;
			return(true);
		}
		//  vehicle not onOutbound nor onInbound, go to be atIntersectionBox
		//  project ptENU onto the inbound lane that the vehicle entered the intersection box
		GeoUtils::projection_t proj2segment;
		GeoUtils::projectPt2Line(laneObj.mpNodes[1].ptNode, laneObj.mpNodes[0].ptNode, ptENU, proj2segment);
		double distInto = proj2segment.t * proj2segment.length - laneObj.mpNodes[1].dTo1stNode;
		if (isPointInsideIntersectionBox(intObj, ptENU) || (std::abs(distInto) < laneObj.mpNodes[0].dTo1stNode))
		{ // if inside intersection polygon or distance into is less than gapDist, set vehicleIntersectionStatus to atIntersectionBox
			cvTrackingState.intsectionTrackingState = prevIntTrackingState;
			cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus = MsgEnum::mapLocType::atIntersectionBox;
			//  update cvTrackingState.laneProj
			cvTrackingState.laneProj.nodeIndex = 1;
			cvTrackingState.laneProj.proj2segment = proj2segment;
			return(true);
		}
		// vehicle is not near onInbound, check whether it's near onOutbound
		if (!dist2egress.empty())
		{ // set vehicleIntersectionStatus to atIntersectionBox
			cvTrackingState.intsectionTrackingState = prevIntTrackingState;
			cvTrackingState.intsectionTrackingState.vehicleIntersectionStatus = MsgEnum::mapLocType::atIntersectionBox;
			//  update cvTrackingState.laneProj
			cvTrackingState.laneProj.nodeIndex = 1;
			cvTrackingState.laneProj.proj2segment = proj2segment;
			return(true);
		}
		return(false);
	}
}

void LocAware::updateLocationAware(const GeoUtils::vehicleTracking_t& vehicleTrackingState, GeoUtils::locationAware_t& vehicleLocationAware) const
{
	vehicleLocationAware.reset();
	switch(vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus)
	{
	case MsgEnum::mapLocType::outside:
		break;
	case MsgEnum::mapLocType::insideIntersectionBox:
		vehicleLocationAware.regionalId = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].regionalId;
		vehicleLocationAware.intersectionId = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex].id;
		break;
	default: // onInbound, atIntersectionBox, onOutbound
		const auto& intObj  = mpIntersection[vehicleTrackingState.intsectionTrackingState.intersectionIndex];
		const auto& appObj  = intObj.mpApproaches[vehicleTrackingState.intsectionTrackingState.approachIndex];
		const auto& laneObj = appObj.mpLanes[vehicleTrackingState.intsectionTrackingState.laneIndex];
		vehicleLocationAware.regionalId = intObj.regionalId;
		vehicleLocationAware.intersectionId = intObj.id;
		vehicleLocationAware.laneId = laneObj.id;
		vehicleLocationAware.controlPhase = laneObj.controlPhase;
		vehicleLocationAware.speed_limit = (double)appObj.speed_limit * DsrcConstants::mph2mps;
		for (const auto& connObj : laneObj.mpConnectTo)
		{
			switch(connObj.laneManeuver)
			{
			case MsgEnum::maneuverType::uTurn:
				vehicleLocationAware.maneuvers.set(3);
				break;
			case MsgEnum::maneuverType::leftTurn:
				vehicleLocationAware.maneuvers.set(2);
				break;
			case MsgEnum::maneuverType::rightTurn:
				vehicleLocationAware.maneuvers.set(0);
				break;
			case MsgEnum::maneuverType::straightAhead:
			case MsgEnum::maneuverType::straight:
				vehicleLocationAware.maneuvers.set(1);
				break;
			default:
				break;
			}
			GeoUtils::connectTo_t connectTo{connObj.regionalId, connObj.intersectionId, connObj.laneId, connObj.laneManeuver};
			vehicleLocationAware.connect2go.push_back(connectTo);
		}
	}
	GeoUtils::point2D_t pt;
	LocAware::getPtDist2D(vehicleTrackingState, pt);
	vehicleLocationAware.dist2go.distLong = DsrcConstants::hecto2unit<int32_t>(pt.x);
	vehicleLocationAware.dist2go.distLat = DsrcConstants::hecto2unit<int32_t>(pt.y);
}

void LocAware::getPtDist2D(const GeoUtils::vehicleTracking_t& vehicleTrackingState, GeoUtils::point2D_t& pt) const
{ // return distance to stop-bar
	if ((vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::outside)
		|| (vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::insideIntersectionBox))
	{
		pt.x = 0;
		pt.y = 0;
		return;
	}
	const auto& intIndx = vehicleTrackingState.intsectionTrackingState.intersectionIndex;
	const auto& appIndx = vehicleTrackingState.intsectionTrackingState.approachIndex;
	const auto& laneIndx = vehicleTrackingState.intsectionTrackingState.laneIndex;
	const auto& nodeIndx = vehicleTrackingState.laneProj.nodeIndex;
	uint32_t nodeDistTo1stNode = (nodeIndx == 0) ? 0
		: mpIntersection[intIndx].mpApproaches[appIndx].mpLanes[laneIndx].mpNodes[nodeIndx].dTo1stNode;
	double ptIntoLine = vehicleTrackingState.laneProj.proj2segment.t * vehicleTrackingState.laneProj.proj2segment.length;
	pt.y = static_cast<int32_t>(vehicleTrackingState.laneProj.proj2segment.d);
	if (vehicleTrackingState.intsectionTrackingState.vehicleIntersectionStatus == MsgEnum::mapLocType::onOutbound)
	{ // pt.x is distance downstream from the intersection box boundary,
		// node index has the same direction of lane projection
		pt.x = static_cast<int32_t>(nodeDistTo1stNode + ptIntoLine);
	}
	else
	{ // pt.x is distance to stop-bar, node index has opposite direction from lane projection
		// positive pt.x onOutbound, negative atIntersectionBox
		pt.x = static_cast<int32_t>(nodeDistTo1stNode - ptIntoLine);
	}
}
/// --- end of functions to locate BSM on MAP --- ///
