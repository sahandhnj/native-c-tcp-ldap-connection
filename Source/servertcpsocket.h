#ifndef SERVERTCPSOCKET_H
#define SERVERTCPSOCKET_H
#include <string>
#include <netinet/in.h>
#include "tcpsocket.h"


class ServerTCPSocket : public TCPSocket
{
    private:
        int sd;             //Second Socket Handler
        bool listner;       //Check if the socket is listening

    public:
        ServerTCPSocket(std::string server,int port_n);
        ServerTCPSocket(int port_n);
        int bindAndListen();   //bind and listen
        TCPSocket* accept();//accept the connection


};

#endif // SERVERTCPSOCKET_H
