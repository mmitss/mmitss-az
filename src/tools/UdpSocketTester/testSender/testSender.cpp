#include <iostream>
#include <fstream>
#include <cstring>
#include <json.h>
#include <UdpSocket.h>
#include <BasicVehicle.h>

using std::cout;
using std::endl;
using std::string;
using std::ifstream;

const short unsigned int SENDPORTNO = 10200;
const string LOCALHOST = "127.0.0.1";

int main()
{
    try
    {
        UdpSocket sendSocket(SENDPORTNO);

        const int receiverPortNo = 1516;
        
        ifstream json_file("BSM.json"); 
        std::string originalJsonString((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
                
        BasicVehicle basicVehicle;
        basicVehicle.json2BasicVehicle(originalJsonString);
        
        string newJsonString = basicVehicle.basicVehicle2Json();
        cout << newJsonString << endl;
        
        sendSocket.sendData(LOCALHOST, receiverPortNo, newJsonString);
        
        sendSocket.closeSocket();   
    }
    catch(int e)
    {
        if (e == 101)
            cout << "Failed to create the socket." << endl;
        else if (e == 102)
            cout << "Failed to bind the socket to port no: " << SENDPORTNO << ". Check if this port is already in use."<< endl;
        else if (e == 103)
            cout << "Socket port number: " << SENDPORTNO << " is out of range. Select any available port number between 1024 and 65535." << endl;
        return 1;
    }
    return 0;
}
