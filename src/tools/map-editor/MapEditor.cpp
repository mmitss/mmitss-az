
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
	Json::Value jsonFileObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	Json::CharReader *fileReader = builder.newCharReader();
	string errors{};
	string jsonFileErrors{};
	ifstream jsonConfigfile("/nojournal/bin/mmitss-phase3-master-config.json");
	ifstream configfile("configfile.json");

	string configJsonString((std::istreambuf_iterator<char>(jsonConfigfile)), std::istreambuf_iterator<char>());
	reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);
	delete reader;

	string configFileJsonString((std::istreambuf_iterator<char>(configfile)), std::istreambuf_iterator<char>());
	fileReader->parse(configFileJsonString.c_str(), configFileJsonString.c_str() + configFileJsonString.size(), &jsonFileObject, &jsonFileErrors);
    delete fileReader;

    intersectionId = jsonObject["IntersectionID"].asInt();
	regionalId = jsonObject["RegionalID"].asInt();
    intersectionName = jsonObject["IntersectionName"].asString();
	mapPayloadFileName = jsonFileObject["FileName"].asString();

}

void MapEditor::processMapPayLoad()
{
    std::vector<uint8_t> mapPayload = plocAwareLib->getMapdataPayload(static_cast<uint16_t>(regionalId), static_cast<uint16_t>(intersectionId));
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
		
		// If the function decodes mappayload file and logs the decoded MAP into nmap file
		if (mapPayloadFileName.find(std::string(".payload")) != std::string::npos)
		{ 
			plocAwareLib->saveNmap(static_cast<uint16_t>(regionalId), static_cast<uint16_t>(intersectionId));
            plocAwareLib->getMapdataPayload(static_cast<uint16_t>(regionalId), static_cast<uint16_t>(intersectionId));

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