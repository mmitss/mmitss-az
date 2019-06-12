#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "../common/MmitssUdpSocket.h"

using std::cout;
using std::endl;
using std::string;
using std::ofstream;

const int PORT = 4448; 

int main()
{
    MmitssUdpSocket receiveSocket(PORT);
    string receivedData{};
    cout << "Waiting for messages..." << endl;
    int i = 1;
    std::clock_t start;
    double duration;
    start = std::clock();
 
    while(i)
    {   
        receivedData = receiveSocket.receive_data();
        duration = (( std::clock() - start ) / (double) 1000);
        cout << std::fixed << std::setprecision(1) << duration <<" Received msg# " << i <<  " from " << receiveSocket.getSenderIP() << ":" << receiveSocket.getSenderPort() << endl;
        i++;
    }
    receiveSocket.close_socket();
    return 0;
}
