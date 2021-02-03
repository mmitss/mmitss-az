#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <UdpSocket.h>
#include <unistd.h>
// #include <jsoncpp/json/json.h>
#include "json/json.h"



unsigned int microseconds = 100000;
using std::vector;

int main()
{
    Json::Value jsonObject;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    Json::CharReaderBuilder builder;
    Json::CharReader * reader = builder.newCharReader();
    std::string errors{};
    reader->parse(configJsonString.c_str(), configJsonString.c_str() + configJsonString.size(), &jsonObject, &errors);        
    delete reader;

    const string LOCALHOST = jsonObject["HostIp"].asString();
    const int priorityRequestSolverPortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["PrioritySolver"].asInt());
    UdpSocket prioritySenderSocket = (static_cast<short unsigned int>(jsonObject["PortNumber"]["PriorityRequestServer"].asInt()));

  
    std::ifstream jsonfile("priorityRequest.json");
    std::string sendingJsonString((std::istreambuf_iterator<char>(jsonfile)),
    std::istreambuf_iterator<char>());
    std::cout << "JsonString: " << sendingJsonString << std::endl;
    
    prioritySenderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(priorityRequestSolverPortNo), sendingJsonString);
}
