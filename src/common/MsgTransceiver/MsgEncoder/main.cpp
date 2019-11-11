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


    UdpSocket encoderSocket(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt());
    TransceiverEncoder encoder;
    std::string messagePayload{};
    char receiveBuffer[5120];
    const string LOCALHOST = "127.0.0.1";
const int messageSenderPortNo = (jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageSender"]).asInt();
 
    while(true)
    {
        encoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string jsonString(receiveBuffer);
        
        if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_bsm)
        {
    
            messagePayload = encoder.BSMEncoder(jsonString);           
            encoderSocket.sendData(LOCALHOST, messageSenderPortNo,messagePayload);
            
        }
       
        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_srm)
        {

            messagePayload = encoder.SRMEncoder(jsonString);  
            encoderSocket.sendData(LOCALHOST, messageSenderPortNo,messagePayload);
        }

        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_spat)
        {
          
            messagePayload = encoder.SPaTEncoder(jsonString);
            encoderSocket.sendData(LOCALHOST, messageSenderPortNo,messagePayload);
            
        }

        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_ssm)
        {   
            messagePayload = encoder.SSMEncoder(jsonString);
            encoderSocket.sendData(LOCALHOST, messageSenderPortNo,messagePayload);

        }        
        
    }
    return 0;
}
