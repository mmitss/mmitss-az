/*
**********************************************************************************

 © 2019 Arizona Board of Regents on behalf of the University of Arizona with rights
       granted for USDOT OSADP distribution with the Apache 2.0 open source license.

**********************************************************************************

  UdpSocket.h  
  Created by: Niraj Vasant Altekar
  University of Arizona   
  College of Engineering

  This code was developed under the supervision of Professor Larry Head
  in the Systems and Industrial Engineering Department.

  Revision History:
  1. 
*/
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
        std::string receivePayloadHexString();
        short unsigned int getSenderPort();
        string getSenderIP();
        void closeSocket();
};
