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
    const string LOCALHOST = "10.12.6.101";
    const int receiverPortNo = 10005;
    std::string sendingJsonString;

    std::ifstream infile;
    int count = 1;

    infile.open("bsmLog_fullLoop.txt");

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
