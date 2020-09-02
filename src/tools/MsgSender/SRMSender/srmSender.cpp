#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <UdpSocket.h>
#include <unistd.h>

unsigned int microseconds = 1000000;

int main()
{
    //Socket Communication
    UdpSocket srmSenderSocket(20020);
    const string LOCALHOST = "10.12.6.108";
    const int receiverPortNo = 20002;
    std::string sendingJsonString;

    std::ifstream infile;
    int count = 1;

    infile.open("srmLog.txt");

    if (infile.fail())
        std::cout << "Fail to open file" << std::endl;

    else
    {
        for (std::string line; getline(infile, line);)
        {

            srmSenderSocket.sendData(LOCALHOST, receiverPortNo, line);
            std::cout << "sent" << count << std::endl;
            count++;
            usleep(microseconds);
        }
    }
}