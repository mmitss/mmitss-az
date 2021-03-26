#include "../TransceiverDecoder.h"
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "json/json.h"
#include "msgEnum.h"

int main()
{
    Json::Value jsonObject_config;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject_config, &errors);        
    delete reader;

    TransceiverDecoder decoder;
    UdpSocket decoderSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"].asInt()));
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    const string HMIControllerIP = jsonObject_config["HMIControllerIP"].asString();
    const string messageDistributorIP = jsonObject_config["MessageDistributorIP"].asString();

    const bool peerDataDecoding = jsonObject_config["PeerDataDecoding"].asBool();

    const int dataCollectorPortNo = (jsonObject_config["PortNumber"]["DataCollector"]).asInt();
    const int mapReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
    const int srmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestServer"]).asInt();
    const int vehicleHmiPortNo = (jsonObject_config["PortNumber"]["HMIController"]).asInt();
    const int ssmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
    const int trajectoryAwarePortNo = (jsonObject_config["PortNumber"]["trajectoryAware"]).asInt();
    const int messageDistributorPort = (jsonObject_config["PortNumber"]["MessageDistributor"]).asInt();

    std::string receivedPayload{};
    std::string extractedPayload{};
    std::string applicationPlatform = decoder.getApplicationPlatform();
    int msgType{};

    while (true)
    {

        receivedPayload = decoderSocket.receivePayloadHexString();
        size_t pos = receivedPayload.find("001");
        if (pos != std::string::npos)
        {
            extractedPayload = receivedPayload.erase(0, pos);
            msgType = decoder.getMessageType(extractedPayload);

            if (msgType == MsgEnum::DSRCmsgID_map)
            {
                if ((applicationPlatform == "vehicle") || ((applicationPlatform == "roadside") && (peerDataDecoding == true)))
                {
                    std::string mapJsonString = decoder.createJsonStingOfMapPayload(extractedPayload);
                    decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(mapReceiverPortNo), mapJsonString);
                    std::cout << "Decoded MAP" << std::endl;
                }
            }

            else if (msgType == MsgEnum::DSRCmsgID_bsm)
            {
                std::string bsmJsonString = decoder.bsmDecoder(extractedPayload);
                decoderSocket.sendData(HMIControllerIP, static_cast<short unsigned int>(vehicleHmiPortNo), bsmJsonString);
                decoderSocket.sendData(messageDistributorIP, static_cast<short unsigned int>(messageDistributorPort), bsmJsonString);
                if(applicationPlatform=="roadside")
                {
                    decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(trajectoryAwarePortNo), bsmJsonString);
                }
                else if(applicationPlatform=="vehicle")
                {
                    decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), bsmJsonString);
                }
                std::cout << "Decoded BSM" << std::endl;
            }

            else if (msgType == MsgEnum::DSRCmsgID_srm)
            {
                if ((applicationPlatform == "roadside") || ((applicationPlatform == "vehicle") && (peerDataDecoding == true)))
                {                
                    std::string srmJsonString = decoder.srmDecoder(extractedPayload);
                    decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(srmReceiverPortNo), srmJsonString);
                    decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), srmJsonString);
                    std::cout << "Decoded SRM" << std::endl;
                }
            }

            else if (msgType == MsgEnum::DSRCmsgID_spat)
            {
                if ((applicationPlatform == "vehicle") || ((applicationPlatform == "roadside") && (peerDataDecoding == true)))
                {
                    std::string spatJsonString = decoder.spatDecoder(extractedPayload);
                    decoderSocket.sendData(HMIControllerIP, static_cast<short unsigned int>(vehicleHmiPortNo), spatJsonString);
                    decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), spatJsonString);
                    std::cout << "Decoded SPAT" << std::endl;
                }
            }

            else if (msgType == MsgEnum::DSRCmsgID_ssm)
            {
                if ((applicationPlatform == "vehicle") || ((applicationPlatform == "roadside") && (peerDataDecoding == true)))
                {                
                    std::string ssmJsonString = decoder.ssmDecoder(extractedPayload);
                    decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(ssmReceiverPortNo), ssmJsonString);
                    decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), ssmJsonString);
                    std::cout << "Decoded SSM" << std::endl;
                }
            }
        }

        if (decoder.sendSystemPerformanceDataLog() == true)
        {
            if (applicationPlatform == "roadside")
            {
                decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), decoder.createJsonStringForSystemPerformanceDataLog("RemoteBSM"));
            }

            else if (applicationPlatform == "vehicle")
            {
                decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), decoder.createJsonStringForSystemPerformanceDataLog("RemoteBSM"));
                decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), decoder.createJsonStringForSystemPerformanceDataLog("SSM"));
                decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), decoder.createJsonStringForSystemPerformanceDataLog("MAP"));
                decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), decoder.createJsonStringForSystemPerformanceDataLog("SPaT"));
            }
        }
    }

    return 0;
}
