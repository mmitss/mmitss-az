#include "TransceiverEncoder.h"
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "msgEnum.h"
#include "json/json.h"

int main()
{ 
    Json::Value jsonObject_config;
	Json::Reader reader;
	std::ifstream configJson("../ConfigurationInfo.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject_config);


    UdpSocket encoderSocket(10003);
    TransceiverEncoder encoder;
    std::string messagePayload{};
    char receiveBuffer[5120];
    const string LOCALHOST = "127.0.0.1";
 
    while(true)
    {
        encoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string jsonString(receiveBuffer);
        
        if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_bsm)
        {
            const int bsmReceiverPortNo = (jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]).asInt();//10002; // Need to change Message Decoder port to Msg Sender port.
    
            messagePayload = encoder.BSMEncoder(jsonString);

            
            encoderSocket.sendData(LOCALHOST, bsmReceiverPortNo,messagePayload);
            
        }
       
        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_srm)
        {
            const int receiverPortNo = (jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]).asInt();//10002; // Need to change Message Decoder port to Msg Sender port.
            messagePayload = encoder.SRMEncoder(jsonString);
  
            encoderSocket.sendData(LOCALHOST, receiverPortNo,messagePayload);
        }

        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_spat)
        {
            messagePayload = encoder.SPaTEncoder(jsonString);
            
        }

        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_ssm)
        {   
            const int receiverPortNo = (jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]).asInt();
            messagePayload = encoder.SSMEncoder(jsonString);

            encoderSocket.sendData(LOCALHOST, receiverPortNo,messagePayload);

        }        
        
    }
    return 0;
}
