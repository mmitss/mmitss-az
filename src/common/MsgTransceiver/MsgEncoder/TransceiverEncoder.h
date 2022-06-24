/***********************************************************************************
 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.
**********************************************************************************
  TransceiverEncoder.h
  Created by: Debashis Das & Niraj Altekar
  University of Arizona   
  College of Engineering
  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.
  Revision History:
  1. Header file for TransceiverEncoder class
*/

#pragma once
#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::fixed;
using std::showpoint;
using std::setprecision;

class TransceiverEncoder
{
    private:
        int messageType{};
        int bsmMsgCount{};
        int srmMsgCount{};
        int ssmMsgCount{};
        int mapMsgCount{};
        int spatMsgCount{};
        int msgSentTime{};
        double timeInterval{0.0};
        string applicationPlatform{};
        string intersectionName{};

    public:
        TransceiverEncoder();
        ~TransceiverEncoder();
        
        int getMessageType(string jsonString);
        string BSMEncoder(string jsonString);
        string SRMEncoder(string jsonString);
        string SPaTEncoder(string jsonString);
        string SSMEncoder(string jsonString);
        string createJsonStringForSystemPerformanceDataLog(string msgCountType);
        string getApplicationPlatform();
        bool sendSystemPerformanceDataLog();
        void setMapMsgCount(int msgCount);  
};