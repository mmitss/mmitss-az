#include "TransceiverEncoder.h"
#include <iostream>
#include <fstream>
#include <UdpSocket.h>
#include "msgEnum.h"
#include "json/json.h"
#include "RsuMsgPacket.h"

int main()
{
    Json::Value jsonObject_config;
    Json::Reader reader;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject_config);

    UdpSocket encoderSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt()));
    TransceiverEncoder encoder;
    std::string messagePayload{};
    char receiveBuffer[5120];
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    bool isMap = false;

    RsuMsgPacket rsuMsgPacket;
    std::string msgToRsu{};

    const string sourceDsrcDeviceIp = jsonObject_config["SourceDsrcDeviceIp"].asString();
    const int sourceDsrcDevicePort = jsonObject_config["PortNumber"]["DsrcImmediateForwarder"].asInt();
    const int systemPerformanceDataCollectorPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["SystemPerformanceDataCollector"].asInt());
   
    int msgType{};
    std::string applicationPlatform = encoder.getApplicationPlatform();

    while (true)
    {
        encoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string jsonString(receiveBuffer);
        
        if(jsonString.substr(0,4) == "0012")
            isMap = true;
        else
            isMap = false;

        if(isMap == false)
        {
            msgType = encoder.getMessageType(jsonString);
            if (msgType == MsgEnum::DSRCmsgID_bsm)
            {
                messagePayload = encoder.BSMEncoder(jsonString);
                msgToRsu = rsuMsgPacket.getMsgPacket(messagePayload);
                encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
                encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), messagePayload);
                std::cout << "Encoded BSM and sent to RSU" << std::endl;
            }

            else if (msgType == MsgEnum::DSRCmsgID_srm)
            {
                messagePayload = encoder.SRMEncoder(jsonString);
                msgToRsu = rsuMsgPacket.getMsgPacket(messagePayload);
                encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
                encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), messagePayload);
                std::cout << "Encoded SRM and sent to RSU" << std::endl;
            }

            else if (msgType == MsgEnum::DSRCmsgID_spat)
            {
                messagePayload = encoder.SPaTEncoder(jsonString);
                msgToRsu = rsuMsgPacket.getMsgPacket(messagePayload);
                encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
                encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), messagePayload);
                std::cout << "Encoded SPAT and sent to RSU" << std::endl;
            }

            else if (msgType == MsgEnum::DSRCmsgID_ssm)
            {
                messagePayload = encoder.SSMEncoder(jsonString);
                msgToRsu = rsuMsgPacket.getMsgPacket(messagePayload);
                encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
                encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), messagePayload);
                std::cout << "Encoded SSM and sent to RSU" << std::endl;
            }

        }
        else if (isMap == true)
        {
            msgToRsu = rsuMsgPacket.getMsgPacket(jsonString);
            encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
            encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), jsonString);
            std::cout << "Sent MAP to RSU" << std::endl;
        }

        else if (encoder.sendSystemPerformanceDataLog()== true)
        {
            if (applicationPlatform == "roadside")
            {
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(systemPerformanceDataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("SSM"));
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(systemPerformanceDataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("MAP"));
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(systemPerformanceDataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("SPaT"));
            }

            else if (applicationPlatform == "vehicle")
            {
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(systemPerformanceDataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("HostBSM"));
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(systemPerformanceDataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("SRM"));
            
            }
        }
        
    }
    return 0;
}
