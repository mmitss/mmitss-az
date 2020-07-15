#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <UdpSocket.h>
#include <unistd.h>
#include "json/json.h"
#include "SignalRequest.h"
#include <vector>

using std::vector;

int main()
{

    UdpSocket requestGeneratorSocket(20050);
    const string LOCALHOST = "10.12.6.108";
    const int receiverPortNo = 20002;
    std::string sendingJsonString;
    // Json::Value jsonObject;
    // Json::FastWriter fastWriter;
    // Json::StyledStreamWriter styledStreamWriter;
    // std::ofstream outputter("srm.json");

    // jsonObject["Timestamp_verbose"] = "2020-03-09 11:40:38.95";
    // jsonObject["Timestamp_posix"] = 1583530838.0951719;
    // jsonObject["MsgType"] = "SRM";
    // jsonObject["SignalRequest"]["msgCount"] = 5;
    // jsonObject["SignalRequest"]["minuteOfYear"] = 95640;
    // jsonObject["SignalRequest"]["msOfMinute"] = 25000;
    // jsonObject["SignalRequest"]["regionalID"] = 0;
    // jsonObject["SignalRequest"]["intersectionID"] = 22391;
    // jsonObject["SignalRequest"]["priorityRequestType"] = 2;
    // jsonObject["SignalRequest"]["inBoundLane"]["LaneID"] = 2;
    // jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Minute"] = 0.0;
    // jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Second"] = 25.0;
    // jsonObject["SignalRequest"]["expectedTimeOfArrival"]["ETA_Duration"] = 2.0;
    // jsonObject["SignalRequest"]["vehicleID"] = 605;
    // jsonObject["SignalRequest"]["basicVehicleRole"] = 12;
    // jsonObject["SignalRequest"]["position"]["latitude_DecimalDegree"] = -112.7062188;
    // jsonObject["SignalRequest"]["position"]["longitude_DecimalDegree"] = -39.2675104;
    // jsonObject["SignalRequest"]["position"]["elevation_Meter"] = 526.0;
    // jsonObject["SignalRequest"]["heading_Degree"] = 120.0;
    // jsonObject["SignalRequest"]["speed_MeterPerSecond"] = 10.0;
    // jsonObject["SignalRequest"]["vehicleType"] = 6;

    // sendingJsonString = fastWriter.write(jsonObject);
    // styledStreamWriter.write(outputter, jsonObject);
    // std::cout << "JsonString: " << sendingJsonString << std::endl;
    // requestGeneratorSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);

    SignalRequest signalRequest;
    std::ifstream jsonfile("srm1.json");
    std::string json_str((std::istreambuf_iterator<char>(jsonfile)),
                         std::istreambuf_iterator<char>());

    signalRequest.json2SignalRequest(json_str);
    sendingJsonString = signalRequest.signalRequest2Json();

    std::cout << "JsonString: " << sendingJsonString << std::endl;
    requestGeneratorSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);
}