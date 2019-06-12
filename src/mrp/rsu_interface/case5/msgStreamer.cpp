#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include "../common/MmitssUdpSocket.h"

using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::ifstream;

const int PORT = 4444; 

int main()
{
    MmitssUdpSocket sendSocket(PORT);
    string receiverIP = "10.254.56.45";        
    int receiverPort = 1516;
    ifstream ifs("msg.txt");
    stringstream ss;
    ss << ifs.rdbuf();
    string msg = ss.str();
    cout << "Streaming\n " << msg << " to " << receiverIP << ":" << receiverPort << endl;
    while(true)
    { 
        sendSocket.send_data(receiverIP, receiverPort, msg);
        usleep(10000);
    }
    sendSocket.close_socket();   
    return 0;
}
