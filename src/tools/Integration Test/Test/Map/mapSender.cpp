#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <UdpSocket.h>
#include <unistd.h>

unsigned int microseconds = 1000;

int main()
{
    //Socket Communication
    UdpSocket bsmSenderSocket(5010);
    const string LOCALHOST = "127.0.0.1";
    const int receiverPortNo = 10004;
    std::string sendingJsonString;

    std::ifstream infile;
    int count = 1;

    infile.open("mapPayload.txt");

    if (infile.fail())
        std::cout << "Fail to open file" << std::endl;

    else
    {
        for (std::string line; getline(infile, line);)
        {

            bsmSenderSocket.sendData(LOCALHOST, receiverPortNo, line);
            std::cout << "sent" << count << std::endl;
            count++;
            usleep(microseconds);
        }
    }
}