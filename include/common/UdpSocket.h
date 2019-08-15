#pragma once

#include <arpa/inet.h>
#include <string>

using std::string;

const int MINPORTNO = 1024;
const int MAXPORTNO = 65534;
const int RECVBUFFERSIZE = 5120;

class UdpSocket
{
    private:
        int selfName{};
        sockaddr_in selfIdentifier{};
        short unsigned int senderPort{};
        string senderIP{};
        
    public:
        UdpSocket(const short unsigned int port);
        UdpSocket(const short unsigned int port, int timeOutSec, int timeOutMicroSec);
        void sendData(const string receiverIP, const short unsigned int receiverPort, const string sendBuffer);
        bool receiveData(char *recvBuffer, size_t sizeofReceiveBuffer);
        int getSenderPort();
        string getSenderIP();
        void closeSocket();
};