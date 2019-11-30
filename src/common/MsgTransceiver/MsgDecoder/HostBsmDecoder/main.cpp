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
    BasicVehicle basicVehicle;
    double latitude{};
    double longitude{};
    int secMark{};
    double elevation{};
    double heading{};
    double speed{};
    std::ofstream locationLog("locationLog.csv");
    locationLog << "secMark,latitude,longitude,elevation,heading,speed" << std::endl;

    while(true)
    {
        decoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedPayload(receiveBuffer);
        std::string bsmJsonString = decoder.bsmDecoder(receivedPayload);
        basicVehicle.json2BasicVehicle(bsmJsonString);
        secMark = basicVehicle.getSecMark_Second();
        latitude = basicVehicle.getLatitude_DecimalDegree();
        longitude = basicVehicle.getLongitude_DecimalDegree();
        elevation = basicVehicle.getElevation_Meter();
        heading = basicVehicle.getHeading_Degree();
        speed = basicVehicle.getSpeed_MeterPerSecond();
        locationLog << secMark*1000 << "," << std::setprecision(10) << latitude << "," << longitude << "," << std::setprecision(5) << elevation << "," << std::setprecision(2) << heading << "," << std::setprecision(2) << speed << std::endl;
        std::cout << secMark*1000 << "," << std::setprecision(10) << latitude << "," << longitude << "," << std::setprecision(5) << elevation << "," << heading << "," << speed << std::endl;
        decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(bsmReceiverPortNo), bsmJsonString);
    }
    locationLog.close();
    return 0;
}
