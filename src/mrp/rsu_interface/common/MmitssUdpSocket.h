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
        MmitssUdpSocket(const int port);
        void send_data(const int receiverPort, const string sendBuffer);
        void send_data(const string receiverIP, const int receiverPort, const string sendBuffer);
        string receive_data();
        int getSenderPort();
        string getSenderIP();
        void close_socket();
};

#endif //_MMITSSUDPSOCKET_H_
