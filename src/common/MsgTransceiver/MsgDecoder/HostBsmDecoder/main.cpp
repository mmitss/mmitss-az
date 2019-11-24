#include "../TransceiverDecoder.h"
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "json/json.h"
#include "msgEnum.h"

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

    while(true)
    {
        decoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedPayload(receiveBuffer);
        std::string bsmJsonString = decoder.bsmDecoder(receivedPayload);
        decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(bsmReceiverPortNo), bsmJsonString);
        std::cout << "Decoded BSM" << std::endl;
    }
    
    return 0;
}
