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
    UdpSocket decoderSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"].asInt()));
    char receiveBuffer[5120];
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    const string HMIControllerIP = jsonObject_config["HMIControllerIP"].asString();
    const string DataCollectorIP = jsonObject_config["DataCollectorIP"].asString();

    const int dataCollectorPortNo = (jsonObject_config["PortNumber"]["DataCollector"]).asInt();
    const int mapReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
    const int srmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestServer"]).asInt();
    const int vehicleHmiPortNo = (jsonObject_config["PortNumber"]["HMIController"]).asInt();
    const int ssmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();

    while(true)
    {

        std::string receivedPayload = decoderSocket.receivePayloadHexString();
        int msgType = decoder.getMessageType(receivedPayload);

        if (msgType == MsgEnum::DSRCmsgID_map)
        {
            std::string mapJsonString = decoder.createJsonStingOfMapPayload(receivedPayload);
            decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(mapReceiverPortNo), mapJsonString);
            std::cout << "Decoded MAP" << std::endl;
        }
        
        else if (msgType == MsgEnum::DSRCmsgID_bsm)
        {
            std::string bsmJsonString = decoder.bsmDecoder(receivedPayload);
            decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), bsmJsonString);
            decoderSocket.sendData(HMIControllerIP, static_cast<short unsigned int>(vehicleHmiPortNo), bsmJsonString);
            decoderSocket.sendData(DataCollectorIP , static_cast<short unsigned int>(dataCollectorPortNo), bsmJsonString);
            std::cout << "Decoded BSM" << std::endl;
        }
        
        else if (msgType == MsgEnum::DSRCmsgID_srm)
        {
            std::string srmJsonString = decoder.srmDecoder(receivedPayload);
            decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(srmReceiverPortNo), srmJsonString);
            decoderSocket.sendData(DataCollectorIP , static_cast<short unsigned int>(dataCollectorPortNo), srmJsonString);
            std::cout << "Decoded SRM" << std::endl;
        }
        
        else if (msgType == MsgEnum::DSRCmsgID_spat)
        {
            std::string spatJsonString = decoder.spatDecoder(receivedPayload);
            decoderSocket.sendData(HMIControllerIP, static_cast<short unsigned int>(vehicleHmiPortNo), spatJsonString);
            decoderSocket.sendData(DataCollectorIP , static_cast<short unsigned int>(dataCollectorPortNo), spatJsonString);
            std::cout << "Decoded SPAT" << std::endl;
        }
        
        else if (msgType == MsgEnum::DSRCmsgID_ssm)
        {
            std::string ssmJsonString = decoder.ssmDecoder(receivedPayload);
            decoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(ssmReceiverPortNo),ssmJsonString);
            decoderSocket.sendData(DataCollectorIP , static_cast<short unsigned int>(dataCollectorPortNo), ssmJsonString);
            std::cout << "Decoded SSM" << std::endl;
        }        
    }
    
    return 0;
}
