/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

    main.cpp
    Created by: Niraj Vasant Altekar
    University of Arizona   
    College of Engineering

    This code was developed under the supervision of Professor Larry Head
    in the Systems and Industrial Engineering Department.

    Description:

*/

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unistd.h>
#include <iomanip>
#include "json/json.h"

#include <UdpSocket.h>
#include "BsmLocator.h"

int main()
{
    std::string configFilename("/nojournal/bin/mmitss-phase3-master-config.json");
    Json::Value config;
    std::ifstream configJson(configFilename);
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder readBuilder;
    Json::CharReader * reader = readBuilder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &config, &errors);        

    bool offmapBsmFiltering = config["OffmapBsmFiltering"].asBool();

    BsmLocator bsmLocator(configFilename);    

    UdpSocket s(static_cast<short unsigned int>(config["PortNumber"]["TrajectoryAware"].asInt()));
    std::string hostIp = config["HostIp"].asString();
    int dataCollectorPort = config["PortNumber"]["DataCollector"].asInt();

    char receiveBuffer[5120];
    Json::Value receivedJsonObject;
    
    Json::Value outgoingBsmJson;
    Json::StreamWriterBuilder writeBuilder;
    writeBuilder["commentStyle"] = "None";
    writeBuilder["indentation"] = "";
    std::string outgoingBsmJsonString{};

    while(true)
    {
        s.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);
        reader->parse(receivedJsonString.c_str(), receivedJsonString.c_str() + receivedJsonString.size(), &receivedJsonObject, &errors);        
        if(receivedJsonObject["MsgType"]=="BSM")
        {
            outgoingBsmJson = bsmLocator.getOnMapFields(receivedJsonObject);
            bool onMap = outgoingBsmJson["OnmapVehicle"]["onMap"].asBool();
            if(onMap==false)
            {
                if(offmapBsmFiltering==false)
                {
                    outgoingBsmJsonString = Json::writeString(writeBuilder,outgoingBsmJson);
                    s.sendData(hostIp, static_cast<unsigned short int>(dataCollectorPort), outgoingBsmJsonString);
                }
            }
            else
            {
                outgoingBsmJsonString = Json::writeString(writeBuilder,outgoingBsmJson);
                s.sendData(hostIp, static_cast<unsigned short int>(dataCollectorPort), outgoingBsmJsonString);
            }
        }
    }
 delete reader;
 return 0;    
}
