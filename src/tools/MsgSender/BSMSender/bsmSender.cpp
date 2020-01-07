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
    UdpSocket bsmSenderSocket(20010);
    const string LOCALHOST = "10.12.6.56";
    const int receiverPortNo = 10007;
    std::string sendingJsonString;

    std::ifstream infile;
    int count = 1;

    infile.open("bsmLog.txt");

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