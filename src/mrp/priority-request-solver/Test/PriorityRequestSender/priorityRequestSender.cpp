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
    Json::Value jsonObject_config;
    Json::Reader reader;
    std::ifstream configJson("/nojournal/bin/mmitss-phase3-master-config.json");
    std::string configJsonString((std::istreambuf_iterator<char>(configJson)), std::istreambuf_iterator<char>());
    reader.parse(configJsonString.c_str(), jsonObject_config);
    const string LOCALHOST = jsonObject_config["HostIp"].asString();
    const int priorityRequestSolverPortNo = static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PrioritySolver"].asInt());
    UdpSocket prioritySenderSocket = (static_cast<short unsigned int>(jsonObject_config["PortNumber"]["PriorityRequestServer"].asInt()));

  
    std::ifstream jsonfile("priorityRequest.json");
    std::string sendingJsonString((std::istreambuf_iterator<char>(jsonfile)),
    std::istreambuf_iterator<char>());
    std::cout << "JsonString: " << sendingJsonString << std::endl;
    
    prioritySenderSocket.sendData(LOCALHOST, priorityRequestSolverPortNo, sendingJsonString);
}
