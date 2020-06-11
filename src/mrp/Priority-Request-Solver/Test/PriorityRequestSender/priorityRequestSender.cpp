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
    // vector<int>vehicleType{6, 9, 2, 2, 2, 2};
    // vector<int>basicVehicleRole{16, 9, 13, 13, 13, 13};
    // vector<int>laneID{2, 5, 8, 9, 8, 9};

    // vector<double>ETA{25.0, 8.0, 10.0, 10.0, 15.0, 15.0};
    // vector<double>ETADuration{3.0, 2.0, 3.0, 3.0, 3.0, 3.0};

    // vector<int>requestedPhase{3, 4, 2, 5, 6, 1};

    // vector<int>requestStatus{5, 5, 4, 4, 4, 4};
    // vector<double>vehicleLatitude{32.1217, 32.2435, 32.1145, 32.2435, 32.1145, 32.1156};
    // vector<double>vehicleLongitude{-110.2456, -110.9624, -110.2456, -110.9624, -110.2456, -110.2340};
    // vector<double>vehicleElevation{720.0, 721.2, 721.0, 721.2, 721.0, 724.6};
    // vector<double>vehicleHeading{89.0, 262.0, 262.0, 262.0, 262.0, 188.4};
    // vector<double>vehicleSpeed{15.65, 17.5, 17.9, 17.9, 18.0, 18.0};

    //With mulptiple EV: 4 request 



    // vector<int>vehicleID{1201, 1202, 1205, 1205,  1250, 1250, 1102, 1102, 1120, 1120,};
    // vector<int>vehicleType{6, 9, 2, 2, 2, 2, 2, 2, 2, 2};
    // vector<int>basicVehicleRole{16 ,9, 13, 13, 13, 13, 13, 13, 13, 13};
    // vector<int>laneID{2, 8, 9, 5, 8, 9, 8, 9, 18, 19};
    // vector<double>ETA{25.0, 5.0, 10.0, 10.0, 15.0, 15.0, 10.0, 10.0, 15.0, 15.0};
    // vector<double>ETADuration{3.0, 2.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0};
    // vector<int>requestedPhase{3, 4, 4, 7, 8, 3, 4, 7, 8, 3};
    // vector<int>requestStatus{5, 5, 4, 4, 4, 4, 4, 4, 4, 4};
    // vector<double>vehicleLatitude{32.1217, 32.2435, 32.1145, 32.2435, 32.1145, 32.2435, 32.1145, 32.2435, 32.1145, 32.1156};
    // vector<double>vehicleLongitude{-110.2456, -110.9624, -110.2456, -110.9624, -110.2456, -110.2340, -110.2456, -110.9624, -110.2456, -110.9624};
    // vector<double>vehicleElevation{720.0, 721.2, 721.0, 721.2, 721.0, 724.6, 720.0, 721.2, 721.0, 721.2};
    // vector<double>vehicleHeading{89.0, 262.0, 262.0, 262.0, 262.0, 188.4, 89.0, 262.0, 262.0, 262.0};
    // vector<double>vehicleSpeed{15.65, 18.34, 18.34, 17.5, 18.52, 18.34, 18.34, 17.5, 18.52,18.52};


    //With single EV

    vector<int> vehicleID{1201, 1205, 1205, 1240};
    vector<int> vehicleType{6,2,2,9,};
    vector<int> basicVehicleRole{16, 13, 13, 9};
    vector<int> laneID{2, 8, 9, 5};
    vector<double> ETA{24.0, 16.0, 16.0, 8.0};
    vector<double> ETADuration{5.0, 5.0, 5.0, 2.0};
    vector<int> requestedPhase{3, 2, 5, 4};
    vector<int> requestStatus{5, 4, 4, 5};
    vector<double>vehicleLatitude{32.1217, 32.2435, 32.1145, 32.1156};
    vector<double>vehicleLongitude{-110.2456, -110.9624, -110.2456, -110.2340};
    vector<double>vehicleElevation{720.0, 721.2,721.0, 724.6};
    vector<double>vehicleHeading{89.0, 262.0, 262.0, 188.4};
    vector<double>vehicleSpeed{15.65, 18.34, 18.34, 15.7};

    //Without EV: one transit, one truck

    // vector<int>vehicleID{1201, 1240};
    // vector<int>vehicleType{6, 9};
    // vector<int>basicVehicleRole{16, 9};
    // vector<int>laneID{2, 8};
    // vector<double>ETA{25.0, 35.0};
    // vector<double>ETADuration{5.0, 5.0};
    // vector<int>requestedPhase{4, 3};
    // vector<int>requestStatus{ 4, 4};
    // vector<double>vehicleLatitude{};
    // vector<double>vehicleLongitude{};
    // vector<double>vehicleElevation{};
    // vector<double>vehicleHeading{};
    // vector<double>vehicleSpeed{};

    //Without EV: two transit

    // vector<int>vehicleID{1201, 1240};
    // vector<int>vehicleType{6, 6};
    // vector<int>basicVehicleRole{16, 16};
    // vector<int>laneID{2, 8};
    // vector<double>ETA{25.0, 40.0};
    // vector<double>ETADuration{5.0, 5.0};
    // vector<int>requestedPhase{2, 8};
    // vector<int>requestStatus{ 4, 4};
    // vector<double>vehicleLatitude{32.1217, 32.2435};
    // vector<double>vehicleLongitude{-110.2456, -110.9624};
    // vector<double>vehicleElevation{720.0, 721.2};
    // vector<double>vehicleHeading{89.0, 262.0};
    // vector<double>vehicleSpeed{15.65, 18.34};

    //For Coordination: 4 request for two cycle

    // vector<int>vehicleID{1201, 1240, 1201, 1240};
    // vector<int>vehicleType{6, 6, 6, 6};
    // vector<int>basicVehicleRole{16, 16, 16, 16};
    // vector<int>laneID{2, 8, 2, 8};
    // vector<double>ETA{5.0, 95.0, 5.0, 95.0};
    // vector<double>ETADuration{15.0, 15.0, 15.0, 15.0};
    // vector<int>requestedPhase{2, 2, 6, 6};
    // vector<int>requestStatus{ 4, 4, 4 ,4};
    // vector<double>vehicleLatitude{32.1217, 32.2435, 32.1145, 32.1156};
    // vector<double>vehicleLongitude{-110.2456, -110.9624, -110.2456, -110.2340};
    // vector<double>vehicleElevation{720.0, 721.2,721.0, 724.6};
    // vector<double>vehicleHeading{89.0, 262.0, 262.0, 188.4};
    // vector<double>vehicleSpeed{15.65, 18.34, 18.34, 16.26};

    
    //Single vehicle
    // vector<int>vehicleID{1201};
    // vector<int>vehicleType{6};
    // vector<int>basicVehicleRole{16};
    // vector<int>laneID{8};
    // vector<double>ETA{25.0};
    // vector<double>ETADuration{2.0};
    // vector<int>requestedPhase{8};
    // vector<int>requestStatus{4};
    // vector<double>vehicleLatitude{32.1217};
    // vector<double>vehicleLongitude{-110.2456};
    // vector<double>vehicleElevation{720.0};
    // vector<double>vehicleHeading{89.0};
    // vector<double>vehicleSpeed{15.65};


    noOfRequest = static_cast<int>(requestedPhase.size());
    jsonObject["MsgType"] = "PriorityRequest";
    jsonObject["PriorityRequestList"]["noOfRequest"] = noOfRequest; //ActiveRequestTable.size();
    jsonObject["PriorityRequestList"]["minuteOfYear"] = 22478;      //minuteOfYear;
    jsonObject["PriorityRequestList"]["msOfMinute"] = 24;           //msOfMinute;
    jsonObject["PriorityRequestList"]["regionalID"] = 0;            //regionalID;
    jsonObject["PriorityRequestList"]["intersectionID"] = 23426;    //intersectionID;

    // for (unsigned int i = 0; i < ActiveRequestTable.size(); i++)
    for (int i = 0; i < noOfRequest; i++)
    {
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleID"] = vehicleID[i]; //ActiveRequestTable[i].vehicleID;
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["vehicleType"] = vehicleType[i];
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["basicVehicleRole"] = basicVehicleRole[i];   
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["inBoundLaneID"] = laneID[i];                
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA"] = ETA[i];                             
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["ETA_Duration"] = ETADuration[i];            
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["requestedSignalGroup"] = requestedPhase[i]; 
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["priorityRequestStatus"] = requestStatus[i]; 

        jsonObject["PriorityRequestList"]["requestorInfo"][i]["latitude_DecimalDegree"] = vehicleLatitude[i];
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["longitude_DecimalDegree"] = vehicleLongitude[i];
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["elevation_Meter"] = vehicleElevation[i];
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["heading_Degree"] = vehicleHeading[i];
        jsonObject["PriorityRequestList"]["requestorInfo"][i]["speed_MeterPerSecond"] = vehicleSpeed[i];
    }

    sendingJsonString = fastWriter.write(jsonObject);
    std::cout << "JsonString: " << sendingJsonString << std::endl;
    prioritySenderSocket.sendData(LOCALHOST, receiverPortNo, sendingJsonString);
}
