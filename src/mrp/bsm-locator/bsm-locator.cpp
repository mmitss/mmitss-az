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
#include "MapEngine.h"
#include "CSV.h"

int main()
{
    Json::Value config;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &config, &errors);        
    delete reader;

    UdpSocket s(static_cast<short unsigned int>(config["PortNumber"]["OBUBSMReceiver"].asInt()));

    while(true)
    {
        std::cout << "hgellow" << std::endl;
    }
 return 0;    
}
