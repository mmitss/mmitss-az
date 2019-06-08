#include <iostream>
#include <string.h>

#include "../../common/MmitssUdpSocket.h"

using std::cout;
using std::endl;
using std::string;

int main()
{
    MmitssUdpSocket receiveSocket(4445);
    cout << "Waiting for messages..." << endl;    
    while(true)
    {   receiveSocket.receive_data(); 
        cout << "Received from " << receiveSocket.getSenderIP() << ":" << receiveSocket.getSenderPort() << endl;
    }
    receiveSocket.close_socket();
    return 0;
}
