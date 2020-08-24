#pragma once
#include <string>

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
        std::string applicationPlatform{};
        std::string intersectionName{};

    public:
        TransceiverEncoder();
        ~TransceiverEncoder();
        
        int getMessageType(std::string jsonString);
        std::string BSMEncoder(std::string jsonString);
        std::string SRMEncoder(std::string jsonString);
        std::string SPaTEncoder(std::string jsonString);
        std::string SSMEncoder(std::string jsonString);
        std::string createJsonStringForSystemPerformanceDataLog(std::string msgCountType);
        std::string getApplicationPlatform();
        bool sendSystemPerformanceDataLog();
        void setMapMsgCount(int msgCount);  
};