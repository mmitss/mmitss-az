#include "TransceiverDecoder.h"
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "json.h"
#include "msgEnum.h"

int main()
{
    Json::Value jsonObject_config;
	Json::Reader reader;
	std::ifstream configJson("ConfigurationInfo.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject_config);
    
    TransceiverDecoder decoder;
    UdpSocket decoderSocket(10004);
    char receiveBuffer[5120];
    const string LOCALHOST = "127.0.0.1";
    // bool receiveSuccess{};
    while(true)
    {
        // receiveSuccess = decoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        decoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);

        std::cout << "Message Type: " << decoder.getMessageType(receivedJsonString) << std::endl;
        //std::cout << decoder.decodeMessagePayload(receivedJsonString) << std::endl;

        if (decoder.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_map)
        {
            const int mapReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
            // const int mapReceiverPortNo = 20004;
            // const string LOCALHOST = "127.0.0.1";
            std::string mapJsonString = decoder.createJsonStingOfMapPayload(receivedJsonString);
            decoderSocket.sendData(LOCALHOST, mapReceiverPortNo, mapJsonString);
            //std::cout<<"Message is sent"<<std::endl;
        }

        else if (decoder.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_bsm)
        {
            const int bsmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
            // const int bsmReceiverPortNo = 20004;
            // const string LOCALHOST = "127.0.0.1";
            std::string bsmJsonString = decoder.bsmDecoder(receivedJsonString);
            decoderSocket.sendData(LOCALHOST, bsmReceiverPortNo, bsmJsonString);
            //std::cout<<"Message is sent"<<std::endl;
            
        }

        else if (decoder.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_srm)
        {
            const int srmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestServer"]).asInt();
            // const int srmReceiverPortNo = 20002;
            // const string LOCALHOST = "127.0.0.1";
            std::string srmJsonString = decoder.srmDecoder(receivedJsonString);
            decoderSocket.sendData(LOCALHOST, srmReceiverPortNo, srmJsonString);
            // std::cout<<"Message is sent"<<std::endl;
        }

        else if (decoder.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_spat)
        {
        }

        else if (decoder.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_ssm)
        {
            const int ssmReceiverPortNo = (jsonObject_config["PortNumber"]["PriorityRequestGenerator"]).asInt();
            // const int ssmReceiverPortNo = 20004;
            // const string LOCALHOST = "127.0.0.1";
            std::string ssmJsonString = decoder.ssmDecoder(receivedJsonString);
            decoderSocket.sendData(LOCALHOST, ssmReceiverPortNo,ssmJsonString);
            //std::cout<<"Message is sent"<<std::endl;
        }        
    }

return 0;
}