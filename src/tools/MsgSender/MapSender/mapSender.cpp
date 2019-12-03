#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <UdpSocket.h>
#include <unistd.h>

unsigned int microseconds = 100000;

int main()
{
    //Socket Communication
    UdpSocket bsmSenderSocket(20030);
    const string LOCALHOST = "10.12.6.103";
    const int receiverPortNo = 10001;
    std::string sendingJsonString;

    std::ifstream infile;

    infile.open("mapPayload.txt");

    if (infile.fail())
        std::cout << "Fail to open file" << std::endl;

    else
    {
        for (std::string line; getline(infile, line);)
        {

            bsmSenderSocket.sendData(LOCALHOST, receiverPortNo, line);
            std::cout << "sent" << std::endl;
        }
    }
}