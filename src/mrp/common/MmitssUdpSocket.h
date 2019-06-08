#ifndef _MMITSSUDPSOCKET_H_
#define _MMITSSUDPSOCKET_H_

#include <arpa/inet.h>
#include <string>

using std::string;

class MmitssUdpSocket
{
    private:
        int selfName{};
        sockaddr_in selfIdentifier{};
        int senderPort{};
        string senderIP{};
    public:
        MmitssUdpSocket(int port);
        void send_data(int receiverPort, string sendBuffer);
        void send_data(string receiverIP, int receiverPort, string sendBuffer);
        string receive_data();
        int getSenderPort();
        string getSenderIP();
        void close_socket();
};

#endif //_MMITSSUDPSOCKET_H_