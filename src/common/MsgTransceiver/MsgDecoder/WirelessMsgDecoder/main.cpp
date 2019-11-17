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
    UdpSocket decoderSocket(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"].asInt());
    char receiveBuffer[5120];
    const string LOCALHOST = jsonObject_config["HostIp"].asString();

    const int mapReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
    const int bsmReceiverPortNo = (jsonObject_config["PortNumber"]["VehicleHMI"]).asInt();
    const int srmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestServer"]).asInt();
    const int vehicleHmiPortNo = (jsonObject_config["PortNumber"]["VehicleHMI"]).asInt();
    const int ssmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();

    while(true)
    {
        decoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedPayload(receiveBuffer);
        int msgType = decoder.getMessageType(receivedPayload);

        if (msgType == MsgEnum::DSRCmsgID_map)
        {
            std::string mapJsonString = decoder.createJsonStingOfMapPayload(receivedPayload);
            decoderSocket.sendData(LOCALHOST, mapReceiverPortNo, mapJsonString);
            std::cout << "Decoded MAP" << std::endl;
        }
        
        else if (msgType == MsgEnum::DSRCmsgID_bsm)
        {
            std::string bsmJsonString = decoder.bsmDecoder(receivedPayload);
            decoderSocket.sendData(LOCALHOST, bsmReceiverPortNo, bsmJsonString);
            std::cout << "Decoded BSM" << std::endl;
        }
        
        else if (msgType == MsgEnum::DSRCmsgID_srm)
        {
            std::string srmJsonString = decoder.srmDecoder(receivedPayload);
            decoderSocket.sendData(LOCALHOST, srmReceiverPortNo, srmJsonString);
            std::cout << "Decoded SRM" << std::endl;
        }
        
        else if (msgType == MsgEnum::DSRCmsgID_spat)
        {
            std::string spatJsonString = decoder.spatDecoder(receivedPayload);
            decoderSocket.sendData(LOCALHOST, vehicleHmiPortNo, spatJsonString);
            std::cout << "Decoded SPAT" << std::endl;
        }
        
        else if (msgType == MsgEnum::DSRCmsgID_ssm)
        {
            std::string ssmJsonString = decoder.ssmDecoder(receivedPayload);
            decoderSocket.sendData(LOCALHOST, ssmReceiverPortNo,ssmJsonString);
            std::cout << "Decoded SSM" << std::endl;
        }        
    }
    
    return 0;
}
