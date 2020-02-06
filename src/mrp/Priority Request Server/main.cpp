#include <iostream>
#include <fstream>
#include "PriorityRequestServer.h"
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

    PriorityRequestServer PRS;
    SignalRequest signalRequest;
    SignalStatus signalStatus;


    UdpSocket PRSSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PriorityRequestServer"].asInt()));
    const int ssmReceiverPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["MessageTransceiver"]["MessageEncoder"].asInt());
    char receiveBuffer[5120];
    // bool receiveSuccess{};
    const string LOCALHOST = jsonObject_config["HostIp"].asString();//"127.0.0.1";
    
    PRS.deleteMapPayloadFile();
    PRS.writeMAPPayloadInFile();

    while (true)
    {
        // receiveSuccess = PRSSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        PRSSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);
        
        if (PRS.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_srm)
        {
            signalRequest.json2SignalRequest(receivedJsonString);
            signalStatus.signalStatus2Json(PRS.creatingSignalRequestTable(signalRequest));
            PRS.printvector();
            std::cout << std::endl;
            PRS.deleteTimedOutRequestfromActiveRequestTable();
            std::string ssmJsonString = PRS.createSSMJsonString(signalRequest, signalStatus);
            PRSSocket.sendData(LOCALHOST, static_cast<short unsigned int>(ssmReceiverPortNo), ssmJsonString);
        }

        else if (PRS.getMessageType(receivedJsonString) == MsgEnum::DSRCmsgID_spat)
        {
            PRS.getPhaseGroup(receivedJsonString);
        }

        else
        {
            PRS.updateETAInActiveRequestTable();
            std::string ssmJsonString = PRS.createSSMJsonString(signalRequest, signalStatus);
            PRSSocket.sendData(LOCALHOST, static_cast<short unsigned int>(ssmReceiverPortNo), ssmJsonString);
        }
    }

    return 0;
}