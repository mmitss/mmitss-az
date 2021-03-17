#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <UdpSocket.h>
#include <unistd.h>
#include "json/json.h"

enum msgType
{
    signalPlanRequest = 1,
    currentPhaseRequest = 2,
    schedule = 3
};

int getMessageType(std::string jsonString);
std::string getJsonString(std::string fileName);
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
    const int priorityRequestSolver_To_TCI_Interface_PortNo = static_cast<short unsigned int>(jsonObject["PortNumber"]["PrioritySolverToTCIInterface"].asInt());
    UdpSocket tciMsgSenderSocket(static_cast<short unsigned int>(jsonObject["PortNumber"]["TrafficControllerInterface"].asInt()));

    char receiveBuffer[5120];
    int msgType{};
    std::string sendingJsonString{};

    sendingJsonString = getJsonString("signalPlan.json");
    tciMsgSenderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(priorityRequestSolverPortNo), sendingJsonString);
    std::cout << "Sent Signal Plan to Solver " << sendingJsonString << std::endl;
    while (true)
    {
        tciMsgSenderSocket.receiveData(receiveBuffer, sizeof(receiveBuffer));
        std::string receivedJsonString(receiveBuffer);

        msgType = getMessageType(receivedJsonString);

        if (msgType == static_cast<int>(msgType::signalPlanRequest))
        {
            sendingJsonString = getJsonString("signalPlan.json");
        
            tciMsgSenderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(priorityRequestSolverPortNo),  sendingJsonString);
            std::cout << "Sent Signal Plan to Solver " << std::endl;
        }

        else if (msgType == static_cast<int>(msgType::currentPhaseRequest))
        {
            sendingJsonString = getJsonString("currPhase.json");
            
            tciMsgSenderSocket.sendData(LOCALHOST, static_cast<short unsigned int>(priorityRequestSolver_To_TCI_Interface_PortNo), sendingJsonString);
            std::cout << "Sent Current Phase Status to Solver" << std::endl;
        }
        else if (msgType == static_cast<int>(msgType::schedule))
            std::cout << "Received Schedule" << std::endl;
    }
}

int getMessageType(std::string jsonString)
{
    int messageType{};
    Json::Value jsonObject;
	Json::CharReaderBuilder builder;
	Json::CharReader *reader = builder.newCharReader();
	string errors{};
	bool parsingSuccessful = reader->parse(jsonString.c_str(), jsonString.c_str() + jsonString.size(), &jsonObject, &errors);
	delete reader;

    if (parsingSuccessful == true)
    {
        if ((jsonObject["MsgType"]).asString() == "TimingPlanRequest")
            messageType = static_cast<int>(msgType::signalPlanRequest);

        else if ((jsonObject["MsgType"]).asString() == "CurrNextPhaseRequest")
            messageType = static_cast<int>(msgType::currentPhaseRequest);

        else if ((jsonObject["MsgType"]).asString() == "Schedule")
            messageType = static_cast<int>(msgType::schedule);

        else
            std::cout << "Message type is unknown" << std::endl;
    }

    return messageType;
}

std::string getJsonString(std::string fileName)
{
    std::ifstream jsonfile(fileName);
    std::string json_str((std::istreambuf_iterator<char>(jsonfile)),
                         std::istreambuf_iterator<char>());

    return json_str;
}