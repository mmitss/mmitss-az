//Explain at the header about what it does???

#include <iostream>
#include <string.h>
#include <unistd.h>
#include "UdpSocket.h"

using std::string;
using std::cout;
using std::endl;

const int MINPORTNO = 1024;
const int MAXPORTNO = 65535;
const int RECVBUFFERSIZE = 5120;

UdpSocket::UdpSocket(const int port)
{
    if(port >= MINPORTNO || port <= MAXPORTNO)
    {
        if((selfName = socket(AF_INET, SOCK_DGRAM, 0))< 0) // If socket creation is unsuccessful, socket() function returns '-1'.
            cout << "Socket creation failed!" << endl;
        else
        {
            memset(&selfIdentifier, 0, sizeof(selfIdentifier));
            selfIdentifier.sin_family = AF_INET;
            selfIdentifier.sin_addr.s_addr = INADDR_ANY;
            selfIdentifier.sin_port = htons(port);
            if ( bind(selfName, (sockaddr *)&selfIdentifier, sizeof(selfIdentifier)) < 0 )
                cout << "Socket binding failed!" << endl;
        }
    }
    else
    {
        cout << "Invalid port number. Use a port between 1024 to 65535 (both limits included)" << endl;;
    }
}
void UdpSocket::sendData(const string receiverIP, const int receiverPort, const string sendBuffer)
{
    sockaddr_in receiverIdentifier{};
    receiverIdentifier.sin_family = AF_INET;
    receiverIdentifier.sin_addr.s_addr = inet_addr(receiverIP.c_str());
    receiverIdentifier.sin_port = htons(receiverPort);
    socklen_t receiverAddrLen = sizeof(receiverIdentifier);
    sendto(selfName, sendBuffer.c_str(), strlen(sendBuffer.c_str()), 0, (sockaddr*)&receiverIdentifier, receiverAddrLen);
}
void UdpSocket::receiveData(char *receiveBuffer, size_t sizeofReceiveBuffer)
{
    sockaddr_in senderIdentifier{};
    socklen_t senderAddrLen = sizeof(senderIdentifier);
    size_t n = recvfrom(selfName, receiveBuffer, sizeofReceiveBuffer, 0, (sockaddr*)&senderIdentifier, &senderAddrLen);
    receiveBuffer[n] = '\0'; 
    senderPort =  ntohs(senderIdentifier.sin_port);
    senderIP = inet_ntoa(senderIdentifier.sin_addr);
}
int UdpSocket::getSenderPort()
{
    return senderPort;
}
string UdpSocket::getSenderIP()
{
    return senderIP;
}
void UdpSocket::closeSocket()
{
    close(selfName);
}
