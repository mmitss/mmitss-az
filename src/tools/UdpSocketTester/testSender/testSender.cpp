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


const short unsigned int SENDPORTNO = 10001;
const string LOCALHOST = "127.0.0.1";

int main()
{
    UdpSocket sendSocket(SENDPORTNO);
    const int receiverPortNo = 1516;
	ifstream json_file("BSM.json");
    std::string originalJsonString((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
    cout << originalJsonString << endl;
    BasicVehicle basicVehicle;
    basicVehicle.json2BasicVehicle(originalJsonString);
	string newJsonString = basicVehicle.basicVehicle2Json();
    cout << newJsonString << endl;
	sendSocket.sendData(LOCALHOST, receiverPortNo, newJsonString);
    sendSocket.closeSocket();    
    return 0;
}
