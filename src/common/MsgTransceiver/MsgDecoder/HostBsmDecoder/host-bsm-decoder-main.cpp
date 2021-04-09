#include "../TransceiverDecoder.h"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "json/json.h"

int main()
{
    Json::Value jsonObject_config;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject_config, &errors);        
    delete reader;

    TransceiverDecoder decoder;
    UdpSocket decoderSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["HostBsmDecoder"].asInt()));

    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    int bsmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
    const int dataCollectorPortNo = (jsonObject_config["PortNumber"]["DataCollector"]).asInt();

    double currentTime{};
    string receivedPayload{};
    string extractedPayload{};
    string bsmJsonString{};
    string applicationPlatform = decoder.getApplicationPlatform();

    while (true)
    {
        receivedPayload = decoderSocket.receivePayloadHexString();
        currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

        size_t pos = receivedPayload.find("0014");

        if (pos != string::npos)
        {
            extractedPayload = receivedPayload.erase(0, pos);
            bsmJsonString = decoder.bsmDecoder(extractedPayload);
            cout << "[" << fixed << showpoint << setprecision(2) << currentTime << "] Decoded HostBSM" << endl;
            decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(bsmReceiverPortNo), bsmJsonString);
            decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), bsmJsonString);
        }

        if (decoder.sendSystemPerformanceDataLog() == true && applicationPlatform == "vehicle")
            decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), decoder.createJsonStringForSystemPerformanceDataLog("HostBSM"));
    }

    return 0;
}
