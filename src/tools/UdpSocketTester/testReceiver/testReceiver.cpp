#include <iostream>
#include <string.h>

#include "../UdpSocket.h"

using std::cout;
using std::endl;
using std::string;

const int RECVPORTNO = 1516;

int main()
{
    UdpSocket receiveSocket(RECVPORTNO);
    char receiveDataBuffer[5120]{};
    receiveSocket.receiveData(receiveDataBuffer, sizeof(receiveDataBuffer));
    
    cout << receiveDataBuffer <<" from " << receiveSocket.getSenderIP() << ":" << receiveSocket.getSenderPort() << endl;
    receiveSocket.closeSocket();
    return 0;
}