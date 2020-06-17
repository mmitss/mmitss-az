/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  UdpSocket.cpp  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. 
  Thrown exception:
    -> 101: Socket creation failed
    -> 102: Socket binding failed
    -> 103: Port number out of range
*/

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include "../../include/common/UdpSocket.h"

using std::cout;
using std::endl;
using std::string;

/************************************
 * CONSTRUCTORS 
 ************************************/

UdpSocket::UdpSocket(const short unsigned int port)
{
    if (port >= MINPORTNO && port <= MAXPORTNO)
    {
        if ((selfName = socket(AF_INET, SOCK_DGRAM, 0)) < 0) // If socket creation is unsuccessful, socket() function returns '-1'.
        {
            throw(101);
        }
        else
        {
            memset(&selfIdentifier, 0, sizeof(selfIdentifier));
            selfIdentifier.sin_family = AF_INET;
            selfIdentifier.sin_addr.s_addr = INADDR_ANY;
            selfIdentifier.sin_port = htons(port);
            if (bind(selfName, (sockaddr *)&selfIdentifier, sizeof(selfIdentifier)) < 0)
            {
                throw(102);
            }
        }
    }
    else
    {
        throw(103);
    }
}

UdpSocket::UdpSocket(const short unsigned int port, int timeOutSec, int timeOutMicroSec)
{
    if (port >= MINPORTNO && port <= MAXPORTNO)
    {

        if ((selfName = socket(AF_INET, SOCK_DGRAM, 0)) < 0) // If socket creation is unsuccessful, socket() function returns '-1'.
        {
            throw(101);
        }
        else
        {
            struct timeval tv;
            tv.tv_sec = timeOutSec;
            tv.tv_usec = timeOutMicroSec;
            if (setsockopt(selfName, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
            {
                perror("Error");
            }
            memset(&selfIdentifier, 0, sizeof(selfIdentifier));
            selfIdentifier.sin_family = AF_INET;
            selfIdentifier.sin_addr.s_addr = INADDR_ANY;
            selfIdentifier.sin_port = htons(port);
            if (bind(selfName, (sockaddr *)&selfIdentifier, sizeof(selfIdentifier)) < 0)
            {
                throw 102;
            }
        }
    }
    else
    {
        throw 103;
    }
}

/************************************
 * SEND AND RECEIVE FUNCTIONS
 ************************************/

void UdpSocket::sendData(const string receiverIP, const short unsigned int receiverPort, const string sendBuffer)
{
    sockaddr_in receiverIdentifier{};
    receiverIdentifier.sin_family = AF_INET;
    receiverIdentifier.sin_addr.s_addr = inet_addr(receiverIP.c_str());
    receiverIdentifier.sin_port = htons(receiverPort);
    socklen_t receiverAddrLen = sizeof(receiverIdentifier);
    sendto(selfName, sendBuffer.c_str(), strlen(sendBuffer.c_str()), 0, (sockaddr *)&receiverIdentifier, receiverAddrLen);
}

bool UdpSocket::receiveData(char *receiveBuffer, size_t sizeofReceiveBuffer)
{
    sockaddr_in senderIdentifier{};
    socklen_t senderAddrLen = sizeof(senderIdentifier);
    long int n = recvfrom(selfName, receiveBuffer, sizeofReceiveBuffer, 0, (sockaddr *)&senderIdentifier, &senderAddrLen);
    if (n < 0)
    {
        return 1;
    }
    else
    {
        receiveBuffer[n] = '\0';
        senderPort = ntohs(senderIdentifier.sin_port);
        senderIP = inet_ntoa(senderIdentifier.sin_addr);
        return 0;
    }
}

std::string UdpSocket::receivePayloadHexString()
{
    unsigned char receiveBuffer[2048]{};
    sockaddr_in senderIdentifier{};
    socklen_t senderAddrLen = sizeof(senderIdentifier);
    std::stringstream ss{};
    std::string payload{};

    long int n = recvfrom(selfName, receiveBuffer, sizeof(receiveBuffer), 0, (sockaddr *)&senderIdentifier, &senderAddrLen);

    if (n < 0)
    {
        return "1";
    }
    else
    {
        unsigned char receivedData[n]{};
        memcpy(receivedData, receiveBuffer, n);
        senderPort = ntohs(senderIdentifier.sin_port);
        senderIP = inet_ntoa(senderIdentifier.sin_addr);

        ss << std::hex;

        for (size_t i = 0; i < sizeof(receivedData); i++)
            ss << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(receivedData[i]);

        payload = ss.str();

        return payload;
    }
}

/************************************
 * GETTERS 
 ************************************/

short unsigned int UdpSocket::getSenderPort()
{
    return senderPort;
}

string UdpSocket::getSenderIP()
{
    return senderIP;
}

/************************************
 * OTHER OPERATIONAL FUNCTIONS
 ************************************/

void UdpSocket::closeSocket()
{
    close(selfName);
}
