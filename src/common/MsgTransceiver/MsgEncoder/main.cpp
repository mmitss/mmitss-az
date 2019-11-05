#include "TransceiverEncoder.h"
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "msgEnum.h"
#include "json.h"

int main()
{ 
    Json::Value jsonObject_config;
	Json::Reader reader;
	std::ifstream configJson("ConfigurationInfo.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
	reader.parse(configJsonString.c_str(), jsonObject_config);


    UdpSocket encoderSocket(10003);
    TransceiverEncoder encoder;
    std::string messagePayload{};
    char receiveBuffer[5120];
    // bool receiveSuccess{};
    const string LOCALHOST = "127.0.0.1";
 
    while(true)
    {
        // receiveSuccess = encoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        encoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string jsonString(receiveBuffer);
        
        if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_bsm)
        {
            const int bsmReceiverPortNo = (jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]).asInt();//10002; // Need to change Message Decoder port to Msg Sender port.
            // const int bsmReceiverPortNo = 20054;
            messagePayload = encoder.BSMEncoder(jsonString);
            std::cout << messagePayload << std::endl;
            // const string bsmReceiverIP = "10.254.56.52";
            
            encoderSocket.sendData(LOCALHOST, bsmReceiverPortNo,messagePayload);
            std::cout << "msg Sent"<< std::endl;
        }
       
        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_srm)
        {
            const int receiverPortNo = (jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]).asInt();//10002; // Need to change Message Decoder port to Msg Sender port.
            messagePayload = encoder.SRMEncoder(jsonString);
            std::cout << messagePayload << std::endl;
            encoderSocket.sendData(LOCALHOST, receiverPortNo,messagePayload);
        }

        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_spat)
        {
            messagePayload = encoder.SPaTEncoder(jsonString);
            std::cout << messagePayload << std::endl;
            //encoderSocket.sendData("10.254.56.101",10001,messagePayload);
        }

        else if (encoder.getMessageType(jsonString) == MsgEnum::DSRCmsgID_ssm)
        {   
            const int receiverPortNo = (jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageDecoder"]).asInt();
            messagePayload = encoder.SSMEncoder(jsonString);
            std::cout << messagePayload << std::endl;
            encoderSocket.sendData(LOCALHOST, receiverPortNo,messagePayload);
            //encoderSocket.sendData("10.254.56.101",10001,messagePayload);
        }        
        
    }
    return 0;
}
