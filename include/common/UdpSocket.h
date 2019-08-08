#pragma once

#include <arpa/inet.h>
#include <string>

using std::string;

class UdpSocket
{
    private:
        int selfName{};
        sockaddr_in selfIdentifier{};
        int senderPort{};
        string senderIP{};
    public:
        UdpSocket(const int port);
        void sendData(const string receiverIP, const int receiverPort, const string sendBuffer);
        void receiveData(char *recvBuffer);
        int getSenderPort();
        string getSenderIP();
        void closeSocket();
};