#pragma once
#include <string>

class TransceiverEncoder
{
    private:
        int messageType;

    public:
        TransceiverEncoder();
        ~TransceiverEncoder();
        
        int getMessageType(std::string jsonString);
        std::string BSMEncoder(std::string jsonString);
        std::string SRMEncoder(std::string jsonString);
        std::string SPaTEncoder(std::string jsonString);
        std::string SSMEncoder(std::string jsonString);
        

};