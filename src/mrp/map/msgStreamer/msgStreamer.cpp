#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>

#include "../../common/MmitssUdpSocket.h"

using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::ifstream;

int main()
{
    MmitssUdpSocket sendSocket(6053);
    string receiverIP = "127.0.0.1";
    int receiverPort = 1516;
    ifstream ifs("msg.txt");
    stringstream ss;
    ss << ifs.rdbuf();
    string msg = ss.str();
    cout << "Streaming\n " << msg << " to " << receiverIP << ":" << receiverPort << endl;
    while(true)
        sendSocket.send_data(receiverPort, msg);
    sendSocket.close_socket();    
    return 0;
}
