#include "../TransceiverDecoder.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "json/json.h"
#include "msgEnum.h"
#include <BasicVehicle.h>

int main()
{
    Json::Value jsonObject_config;
	Json::Reader reader;
	std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject_config);
    
    TransceiverDecoder decoder;
    UdpSocket decoderSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["HostBsmDecoder"].asInt()));
    char receiveBuffer[5120];
    
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    int bsmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
    const int dataCollectorPortNo = (jsonObject_config["PortNumber"]["DataCollector"]).asInt();
    BasicVehicle basicVehicle;
    double latitude{};
    double longitude{};
    int secMark{};
    double elevation{};
    double heading{};
    double speed{};

    while(true)
    {
        std::string receivedPayload = decoderSocket.receivePayloadHexString();
        std::string bsmJsonString = decoder.bsmDecoder(receivedPayload);
        std::cout << "Decoded HostBSM" << std::endl;
        std::cout << bsmJsonString << std::endl;
        basicVehicle.json2BasicVehicle(bsmJsonString);
        secMark = basicVehicle.getSecMark_Second();
        latitude = basicVehicle.getLatitude_DecimalDegree();
        longitude = basicVehicle.getLongitude_DecimalDegree();
        elevation = basicVehicle.getElevation_Meter();
        heading = basicVehicle.getHeading_Degree();
        speed = basicVehicle.getSpeed_MeterPerSecond();
        decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(bsmReceiverPortNo), bsmJsonString);
        decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), bsmJsonString);
        
    }
    return 0;
}
