
#include "AsnJ2735Lib.h"
#include "locAware.h"
#include "geoUtils.h"
#include "MapEditor.h"

MapEditor::MapEditor()
{
    bool singleFrame{false};

    readConfigFile();

	LocAware *tempPlocAwareLib = new LocAware(mapPayloadFileName, singleFrame);
	plocAwareLib = tempPlocAwareLib;
}

void MapEditor::readConfigFile()
{
    Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
	ifstream jsonconfigfile("/nojournal/bin/mmitss-phase3-master-config.json");

	string configJsonString((std::istreambuf_iterator<char>(jsonconfigfile)), std::istreambuf_iterator<char>());
	reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
	delete reader;

    intersectionId = jsonObject["IntersectionID"].asInt();
	regionalId = jsonObject["RegionalID"].asInt();
    intersectionName = jsonObject["IntersectionName"].asString();
    mapPayloadFileName = "/home/debashis/Desktop/mmitss/src/tools/map-editor/map/" + intersectionName + ".map.payload";
    // mapPayloadFileName = "/home/debashis/Desktop/mmitss/src/tools/map-editor/map/" + intersectionName + ".nmap";
}

void MapEditor::processMapPayLoad()
{
    std::vector<uint8_t> mapPayload = plocAwareLib->getMapdataPayload(regionalId, intersectionId);
	if (mapPayloadFileName.find(std::string(".nmap")) != std::string::npos)
	{ /// log hex payload
		std::cout << "Log encoded MAP payload." << std::endl;
		logMapPayload(mapPayload);
	}
	// decode MAP
	Frame_element_t dsrcFrameOut;
	if ((AsnJ2735Lib::decode_msgFrame(&mapPayload[0], mapPayload.size(), dsrcFrameOut) > 0)
		&& (dsrcFrameOut.dsrcMsgId == MsgEnum::DSRCmsgID_map))
	{
		MapData_element_t& mapData = dsrcFrameOut.mapData;
		std::cout << "IntersectionId = " << mapData.id << ", version = " << static_cast<unsigned int>(mapData.mapVersion) << std::endl;
		if (mapPayloadFileName.find(std::string(".payload")) != std::string::npos)
		{ /// log decoded MAP into nmap file
			std::cout << "Log namp file." << std::endl;
			plocAwareLib->saveNmap(regionalId, intersectionId);
            plocAwareLib->getMapdataPayload(regionalId, intersectionId);
            std::cout << "Got the file." << std::endl;
		}
	}
	else
		std::cerr << "Failed decode_msgFrame for MAP" << std::endl;
}

void MapEditor::logMsgHex(std::ofstream& OS, const uint8_t* buf, size_t size)
{
	OS << std::hex;
	for (size_t i = 0; i < size; i++)
		OS << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buf[i]);
	OS << std::dec << std::endl;
}

void MapEditor::logMapPayload(const std::vector<uint8_t>& payload)
{
	std::string fout = std::string("./map/") + intersectionName + std::string(".map.payload");
	std::ofstream OS_OUT(fout);
	// OS_OUT << "Intersection: " << intersectionName << std::endl << std::endl;
	// OS_OUT << "MapData payload size " << payload.size() << std::endl;
	OS_OUT << "payload " << intersectionName << " ";
	logMsgHex(OS_OUT, &payload[0], payload.size());
	OS_OUT << std::endl;
	OS_OUT.close();
}

MapEditor::~MapEditor()
{
}