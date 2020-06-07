# include <iostream>
# include <fstream>
# include "json/json.h"
# include "UdpSocket.h"
# include "SnmpEngine.h"
int main()
{
    // Read configuration items:
    Json::Value jsonObject_config;
    Json::Reader reader;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject_config);
    configJson.close();
    
    std::string ascIp = jsonObject_config["SignalController"]["IpAddress"].asString();
    int ascNtcipPort = jsonObject_config["SignalController"]["NtcipPort"].asUInt();

    UdpSocket snmpEngineSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["SnmpEngine"].asInt()));
    char receiveBuffer[10240]{};
    std::string msgType;
    Json::Value receivedJson;
    std::string receivedOid{};
    int value{};
    int snmpResponse{};

    Json::Value sendingJson;
    Json::FastWriter fastWriter;
    std::string sendingJsonString{};

    std::string senderIp{};
    short unsigned int senderPort{};

    SnmpEngine snmp(ascIp, ascNtcipPort);

    while(true)
    {
        snmpEngineSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        reader.parse(receiveBuffer, receivedJson);
        msgType = receivedJson["MsgType"].asString();
        receivedOid = receivedJson["OID"].asString();
        if(msgType=="SnmpSetRequest")
        {
            value = receivedJson["Value"].asInt();
            snmpResponse = snmp.processSnmpRequest("set", receivedOid, value);
            if(snmpResponse == -1)
            {
                std::cout << "Unable to set value=" << value << " to OID=" << receivedOid << std::endl;
            }
        }
        else if(msgType=="SnmpGetRequest")
        {
            value=0;
            snmpResponse = snmp.processSnmpRequest("get", receivedOid, value);
            
            senderIp = snmpEngineSocket.getSenderIP();
            senderPort = snmpEngineSocket.getSenderPort();
            sendingJson["MsgType"] = "SnmpGetResponse";
            sendingJson["OID"] = receivedOid;
            sendingJson["Value"] = snmpResponse;
            sendingJsonString = fastWriter.write(sendingJson);
            snmpEngineSocket.sendData(senderIp, senderPort, sendingJsonString);

            if (snmpResponse == -1)
            {
                std::cout << "Invalid response (-1) sent for OID=" << receivedOid << std::endl;
            }
        }
    }
    snmpEngineSocket.closeSocket();
    return 0;
}