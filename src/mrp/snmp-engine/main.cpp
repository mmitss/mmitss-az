/***************************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

***************************************************************************************
main.cpp (SnmpEngine)
Created by: Niraj Vasant Altekar
University of Arizona   
College of Engineering

This code was developed under the supervision of Professor Larry Head
in the Systems and Industrial Engineering Department.

***************************************************************************************

Description:
------------
This is a wrapper application for the SnmpEngine class. Once started, the applciation
waits for the messages over UDP socket. The received message is parsed into a JSON 
and its type is identified. Based on the message type, appropriate methods from the 
SnmpEngine class are called to process and execute the SnmpGet or SnmpSet request
in the target SNMP device.

***************************************************************************************/

# include <iostream>
# include <fstream>
# include "json/json.h"
# include "UdpSocket.h"
# include "SnmpEngine.h"

int main()
{
    std::string setCustomMibsCommand = "export MIBS=ALL";
    system(setCustomMibsCommand.c_str()); 

    Json::Value jsonObject_config;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject_config, &errors);        
    delete reader;
    configJson.close();

    // Read the network config for the SnmpEngine applciation    
    std::string ascIp = jsonObject_config["SignalController"]["IpAddress"].asString();
    int ascNtcipPort = jsonObject_config["SignalController"]["NtcipPort"].asUInt();

    // Open a UDP socket for receiving messages from other clients for SnmpSet or SnmpGet
    UdpSocket snmpEngineSocket(static_cast<short unsigned int>(jsonObject_config["PortNumber"]["SnmpEngine"].asInt()));
    
    // Variables to store the received message and its type
    char receiveBuffer[10240]{};
    std::string msgType;
    
    // Variables to store the JSON object and JSON strings to be received
    Json::Value receivedJson;
    std::string receivedOid{};

    // Variables to store the JSON object and JSON strings to be sent
    Json::Value sendingJson;
    Json::StreamWriterBuilder writeBuilder;
    writeBuilder["commentStyle"] = "None";
	writeBuilder["indentation"] = "";
    std::string sendingJsonString{};    

    // Variables to store the SNMP requests related information
    int value{};
    int snmpResponse{};

    // Variables to store the network information of request senders - Used for SnmpGet requests.
    std::string senderIp{};
    short unsigned int senderPort{};

    // Instantiate an object opf SnmpEngine class
    SnmpEngine snmp(ascIp, ascNtcipPort);

    // Creator another reader
    Json::CharReader * inputReader = builder.newCharReader();

    while(true)
    {
        snmpEngineSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string jsonString(receiveBuffer);
        inputReader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &receivedJson, &errors);  
        
        // Parse the message type and OID for set or get request
        msgType = receivedJson["MsgType"].asString();
        receivedOid = receivedJson["OID"].asString();

        if(msgType=="SnmpSetRequest")
        {
            value = receivedJson["Value"].asInt();
            
            // Process the SNMP set request for given OID and given value
            snmpResponse = snmp.processSnmpRequest("set", receivedOid, value);
        }

        else if(msgType=="SnmpGetRequest")
        {
            // Stored the netowork information of the requestor so that the
            // values received by SnmpGet call can be returned back
            senderIp = snmpEngineSocket.getSenderIP();
            senderPort = snmpEngineSocket.getSenderPort();

            // SnmpGet request do not require any value to be provided initially
            // This value is filled after getting the response from the target SNMP device
            value=0;

            // Process the SnmpGet request
            snmpResponse = snmp.processSnmpRequest("get", receivedOid, value);
            
            // Formulate the JSOn string to send back to the requestor
            sendingJson["MsgType"] = "SnmpGetResponse";
            sendingJson["OID"] = receivedOid;
            sendingJson["Value"] = snmpResponse;
            sendingJsonString = Json::writeString(writeBuilder, sendingJson);

            // Send the response to the requestor
            snmpEngineSocket.sendData(senderIp, senderPort, sendingJsonString);
        }
    }
    delete inputReader;
    snmpEngineSocket.closeSocket();
    return 0;
}