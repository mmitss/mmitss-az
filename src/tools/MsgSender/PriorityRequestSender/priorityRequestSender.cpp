#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <UdpSocket.h>
#include <unistd.h>
// #include <jsoncpp/json/json.h>
#include "json/json.h"
#include "requestEntry.h"
#include <vector>

unsigned int microseconds = 100000;
using std::vector;

int main()
{
    vector<requestEntry> ActiveRequestList;
    //Socket Communication
    UdpSocket prioritymSenderSocket(20060);
    const string LOCALHOST = "127.0.0.1";
    const int receiverPortNo = 20003;
    std::string sendingJsonString;
    int noOfRequest = 6;
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    vector<int>vehicleID{1201, 1205, 1205, 1240, 1250, 1250};
    vector<int>vehicleType{6, 2, 2, 9, 2, 2};
    vector<int>basicVehicleRole{16 ,12, 12, 9, 12, 12};
    vector<int>laneID{2, 8, 9, 5, 8, 9};
    vector<double>ETA{24.0, 40.0,40.0, 35.0, 45.0, 45.0};
    vector<double>ETADuration{5.0, 5.0, 5.0, 5.0, 5.0, 5.0};
    vector<int>requestedPhase{3, 8, 3, 4, 4, 7};
    vector<int>requestStatus{5, 4, 4, 5, 4, 4};

    jsonObject["MsgType"] = "PriorityRequest";
    jsonObject["PriorityRequestList"]["noOfRequest"] = noOfRequest;        //ActiveRequestTable.size();
    jsonObject["PriorityRequestList"]["minuteOfYear"] = 22478;   //minuteOfYear;
    jsonObject["PriorityRequestList"]["msOfMinute"] = 24;        //msOfMinute;
    jsonObject["PriorityRequestList"]["regionalID"] = 0;         //regionalID;
    jsonObject["PriorityRequestList"]["intersectionID"] = 23426; //intersectionID;
    
    // for (unsigned int i = 0; i < ActiveRequestTable.size(); i++)
    for (unsigned int i = 0; i < noOfRequest; i++)
    {
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleID"] = vehicleID[i] ;               //ActiveRequestTable[i].vehicleID;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleType"] = vehicleType[i];
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["basicVehicleRole"] =basicVehicleRole[i]; //ActiveRequestTable[i].basicVehicleRole;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["inBoundLaneID"] = laneID[i];       //ActiveRequestTable[i].vehicleLaneID;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] =ETA[i];         //static_cast<int>(ActiveRequestTable[i].vehicleETA / ETA_CONVERTION)+ fmod(ActiveRequestTable[i].vehicleETA, ETA_CONVERTION);
        //jsonObject["PriorityRequestList"]["requestorInfo"][i]["inBoundApproachID"] = ActiveRequestTable[i].vehicleApproachID;
        //jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Minute"] = static_cast<int>(ActiveRequestTable[i].vehicleETA / ETA_CONVERTION);
        //jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Second"] = fmod(ActiveRequestTable[i].vehicleETA, ETA_CONVERTION);
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"] = ETADuration[i]; //ETA_DURATION;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"] = requestedPhase[i];  //ActiveRequestTable[i].signalGroup;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["priorityRequestStatus"] = requestStatus[i]; //ActiveRequestTable[i].prsStatus;
    }

    sendingJsonString = fastWriter.write(jsonObject);
    std::cout << "JsonString: " << sendingJsonString << std::endl;
    prioritymSenderSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);
}

//     std::ifstream infile;
//     int count = 1;

//     infile.open("bsmLog_fullLoop.txt");

//     if (infile.fail())
//         std::cout << "Fail to open file" << std::endl;

//     else
//     {
//         for (std::string line; getline(infile, line);)
//         {

//             bsmSenderSocket.sendData(LOCALHOST, receiverPortNo, line);
//             std::cout << "sent" << count << std::endl;
//             count++;
//             usleep(microseconds);
//         }
//     }
// }