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
    ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject_config, &errors);        
    delete reader;

    UdpSocket encoderSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt()));
    TransceiverEncoder encoder;
    string messagePayload{};
    char receiveBuffer[5120];
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    bool isMap = false;
    int mapMsgCount{};

    RsuMsgPacket rsuMsgPacket;
    string msgToRsu{};
    
    const string sourceDsrcDeviceIp = jsonObject_config["SourceDsrcDeviceIp"].asString();
    const int sourceDsrcDevicePort = jsonObject_config["PortNumber"]["DsrcImmediateForwarder"].asInt();
    const int dataCollectorPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["DataCollector"].asInt());
   
    int msgType{};
    double currentTime{};
    string applicationPlatform = encoder.getApplicationPlatform();

    while (true)
    {
        encoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        string jsonString(receiveBuffer);
        currentTime = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        
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
                cout << "[" << fixed << showpoint << setprecision(2) << currentTime << "] Encoded BSM and sent to RSU" << endl;
            }

            else if (msgType == MsgEnum::DSRCmsgID_srm)
            {
                messagePayload = encoder.SRMEncoder(jsonString);
                msgToRsu = rsuMsgPacket.getMsgPacket(messagePayload);
                encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
                encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), messagePayload);
                cout << "[" << fixed << showpoint << setprecision(2) << currentTime << "] Encoded SRM and sent to RSU" << endl;
            }

            else if (msgType == MsgEnum::DSRCmsgID_spat)
            {
                messagePayload = encoder.SPaTEncoder(jsonString);
                msgToRsu = rsuMsgPacket.getMsgPacket(messagePayload);
                encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
                encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), messagePayload);
                cout << "[" << fixed << showpoint << setprecision(2) << currentTime << "] Encoded SPAT and sent to RSU" << endl;
            }

            else if (msgType == MsgEnum::DSRCmsgID_ssm)
            {
                messagePayload = encoder.SSMEncoder(jsonString);
                msgToRsu = rsuMsgPacket.getMsgPacket(messagePayload);
                encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
                encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), messagePayload);
                cout << "[" << fixed << showpoint << setprecision(2) << currentTime << "] Encoded SSM and sent to RSU" << endl;
            }

        }
        else if (isMap == true)
        {
            msgToRsu = rsuMsgPacket.getMsgPacket(jsonString);
            encoderSocket.sendData(sourceDsrcDeviceIp, static_cast<short unsigned int>(sourceDsrcDevicePort), msgToRsu);
            encoderSocket.sendData(LOCALHOST,static_cast<short unsigned int>(sourceDsrcDevicePort), jsonString);
            cout << "[" << fixed << showpoint << setprecision(2) << currentTime << "] Sent MAP to RSU" << endl;
            mapMsgCount = mapMsgCount + 1;
        }

        if (encoder.sendSystemPerformanceDataLog()== true)
        {
            if (applicationPlatform == "roadside")
            {
                encoder.setMapMsgCount(mapMsgCount);
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("SSM"));
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("MAP"));
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("SPaT"));
                mapMsgCount = 0;
            }

            else if (applicationPlatform == "vehicle")
            {
                encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(dataCollectorPortNo), encoder.createJsonStringForSystemPerformanceDataLog("SRM"));
            }
        }
        
    }
    return 0;
}
