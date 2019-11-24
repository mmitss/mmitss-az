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
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject_config);

    UdpSocket encoderSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt()));
    TransceiverEncoder encoder;
    std::string messagePayload{};
    char receiveBuffer[5120];
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    const int messageSenderPortNo = (jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageSender"]).asInt();

    while (true)
    {
        encoderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string jsonString(receiveBuffer);
        int msgType = encoder.getMessageType(jsonString);

        if (msgType == MsgEnum::DSRCmsgID_bsm)
        {
            messagePayload = encoder.BSMEncoder(jsonString);
            encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(messageSenderPortNo), messagePayload);
            std::cout << "Encoded BSM" << std::endl;
        }

        else if (msgType == MsgEnum::DSRCmsgID_srm)
        {
            messagePayload = encoder.SRMEncoder(jsonString);
            encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(messageSenderPortNo), messagePayload);
            std::cout << "Encoded SRM" << std::endl;
        }

        else if (msgType == MsgEnum::DSRCmsgID_spat)
        {
            messagePayload = encoder.SPaTEncoder(jsonString);
            encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(messageSenderPortNo), messagePayload);
            std::cout << "Encoded SPAT" << std::endl;
        }

        else if (msgType == MsgEnum::DSRCmsgID_ssm)
        {
            messagePayload = encoder.SSMEncoder(jsonString);
            encoderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(messageSenderPortNo), messagePayload);
            std::cout << "Encoded SSM" << std::endl;
        }
    }
    return 0;
}
