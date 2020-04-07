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
    UdpSocket prioritySenderSocket(20060);
    const string LOCALHOST = "127.0.0.1";
    const int receiverPortNo = 20003;
    std::string sendingJsonString;
    int noOfRequest{};
    Json::Value jsonObject;
    Json::FastWriter fastWriter;
    //With mulptiple EV
    
    // vector<int>vehicleID{1201, 1205, 1205, 1240, 1250, 1250};
    // vector<int>vehicleType{6, 2, 2, 9, 2, 2};
    // vector<int>basicVehicleRole{16 ,13, 13, 9, 13, 13};
    // vector<int>laneID{2, 8, 9, 5, 8, 9};
    // vector<double>ETA{25.0, 30.0,30.0, 35.0, 40.0, 40.0};
    // vector<double>ETADuration{5.0, 5.0, 5.0, 5.0, 5.0, 5.0};
    // vector<int>requestedPhase{3, 8, 3, 4, 4, 7};
    // vector<int>requestStatus{5, 4, 4, 5, 4, 4};

    //With mulptiple EV: no left turn phase
    
    // vector<int>vehicleID{1201, 1205, 1240, 1250};
    // vector<int>vehicleType{6, 2, 9, 2};
    // vector<int>basicVehicleRole{16 ,13, 9, 13};
    // vector<int>laneID{2, 8, 5, 14};
    // vector<double>ETA{25.0, 30.0, 35.0, 40.0};
    // vector<double>ETADuration{5.0, 5.0, 5.0, 5.0};
    // vector<int>requestedPhase{3, 2, 4, 4};
    // vector<int>requestStatus{5, 4,  5, 4};
        
    //With single EV
    
    vector<int>vehicleID{1201, 1205, 1205, 1240};
    vector<int>vehicleType{6, 2, 2, 9,};
    vector<int>basicVehicleRole{16 ,13, 13, 9};
    vector<int>laneID{2, 8, 9, 5};
    vector<double>ETA{24.0, 30.0, 30.0, 35.0};
    vector<double>ETADuration{5.0, 5.0, 5.0, 5.0};
    vector<int>requestedPhase{3, 2, 5, 4};
    vector<int>requestStatus{5, 4, 4, 5};
    
    //Without EV: one transit, one truck
    
    // vector<int>vehicleID{1201, 1240};
    // vector<int>vehicleType{6, 9};
    // vector<int>basicVehicleRole{16, 9};
    // vector<int>laneID{2, 8};
    // vector<double>ETA{25.0, 35.0};
    // vector<double>ETADuration{5.0, 5.0};
    // vector<int>requestedPhase{4, 3};
    // vector<int>requestStatus{ 4, 4};

    //Without EV: two transit

    // vector<int>vehicleID{1201, 1240};
    // vector<int>vehicleType{6, 6};
    // vector<int>basicVehicleRole{16, 16};
    // vector<int>laneID{2, 8};
    // vector<double>ETA{25.0, 40.0};
    // vector<double>ETADuration{5.0, 5.0};
    // vector<int>requestedPhase{4, 3};
    // vector<int>requestStatus{ 4, 4};

    //For Coordination: 4 request for two cycle

    // vector<int>vehicleID{1201, 1240, 1201, 1240};
    // vector<int>vehicleType{6, 6, 6, 6};
    // vector<int>basicVehicleRole{16, 16, 16, 16};
    // vector<int>laneID{2, 8, 2, 8};
    // vector<double>ETA{20.0, 140.0, 20.0, 140.0};
    // vector<double>ETADuration{25.0, 25.0, 25.0, 25.0};
    // vector<int>requestedPhase{2, 2, 6, 6};
    // vector<int>requestStatus{ 4, 4, 4 ,4};

    

    noOfRequest = static_cast<int>(requestedPhase.size());
    jsonObject["MsgType"] = "PriorityRequest";
    jsonObject["PriorityRequestList"]["noOfRequest"] = noOfRequest;        //ActiveRequestTable.size();
    jsonObject["PriorityRequestList"]["minuteOfYear"] = 22478;   //minuteOfYear;
    jsonObject["PriorityRequestList"]["msOfMinute"] = 24;        //msOfMinute;
    jsonObject["PriorityRequestList"]["regionalID"] = 0;         //regionalID;
    jsonObject["PriorityRequestList"]["intersectionID"] = 23426; //intersectionID;
    
    // for (unsigned int i = 0; i < ActiveRequestTable.size(); i++)
    for (unsigned int i = 0; i < noOfRequest; i++)
    {
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleID"] = vehicleID[i];               //ActiveRequestTable[i].vehicleID;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleType"] = vehicleType[i];
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["basicVehicleRole"] = basicVehicleRole[i]; //ActiveRequestTable[i].basicVehicleRole;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["inBoundLaneID"] = laneID[i];       //ActiveRequestTable[i].vehicleLaneID;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] = ETA[i];         //static_cast<int>(ActiveRequestTable[i].vehicleETA / ETA_CONVERTION)+ fmod(ActiveRequestTable[i].vehicleETA, ETA_CONVERTION);
        //jsonObject["PriorityRequestList"]["requestorInfo"][i]["inBoundApproachID"] = ActiveRequestTable[i].vehicleApproachID;
        //jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Minute"] = static_cast<int>(ActiveRequestTable[i].vehicleETA / ETA_CONVERTION);
        //jsonObject["SignalStatus"]["requestorInfo"][i]["ETA_Second"] = fmod(ActiveRequestTable[i].vehicleETA, ETA_CONVERTION);
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"] = ETADuration[i]; //ETA_DURATION;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"] = requestedPhase[i];  //ActiveRequestTable[i].signalGroup;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["priorityRequestStatus"] = requestStatus[i]; //ActiveRequestTable[i].prsStatus;
    }

    sendingJsonString = fastWriter.write(jsonObject);
    std::cout << "JsonString: " << sendingJsonString << std::endl;
    prioritySenderSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);
}
