#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <fstream>
#include <UdpSocket.h>
#include <unistd.h>
// #include <jsoncpp/json/json.h>
#include "json/json.h"
#include <vector>

unsigned int microseconds = 100000;
using std::vector;
using std::ofstream;

int main()
{
    //Socket Communication
    UdpSocket bsmSenderSocket(20060);
    // const string LOCALHOST = "127.0.0.1";
    const string LOCALHOST = "10.12.6.56";
    const int receiverPortNo = 20022;
    // std::string sendingJsonString{};
    // Json::Value jsonObject;
    // Json::FastWriter fastWriter;
    // Json::StyledStreamWriter styledStreamWriter;
    // ofstream outputter("bsm.json");
    
    // jsonObject["Timestamp_verbose"] = getVerboseTimestamp();
    // jsonObject["Timestamp_posix"] = getPosixTimestamp();
    // jsonObject["MsgType"] = "BSM";
    // jsonObject["BasicVehicle"]["temporaryID"] = 1201;
    // jsonObject["BasicVehicle"]["secMark_Second"] = 2340004;
    // jsonObject["BasicVehicle"]["position"]["latitude_DecimalDegree"] = 32.235885;
    // jsonObject["BasicVehicle"]["position"]["longitude_DecimalDegree"] = -110.953452;
    // jsonObject["BasicVehicle"]["position"]["elevation_Meter"] = 720;
    // jsonObject["BasicVehicle"]["speed_MeterPerSecond"] = 15.65;
    // jsonObject["BasicVehicle"]["heading_Degree"] = 89.0;
    // jsonObject["BasicVehicle"]["type"] = "Transit";
    // jsonObject["BasicVehicle"]["size"]["length_cm"] = 300;
    // jsonObject["BasicVehicle"]["size"]["width_cm"] = 165;
    // sendingJsonString = fastWriter.write(jsonObject);

    std::ifstream jsonfile("bsm.json");
    std::string sendingJsonString((std::istreambuf_iterator<char>(jsonfile)),
    std::istreambuf_iterator<char>());
    std::cout << "JsonString: " << sendingJsonString << std::endl;
    bsmSenderSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);
    // styledStreamWriter.write(outputter, jsonObject);
}
