#ifndef CLIENTTCPSOCKET_H
#define CLIENTTCPSOCKET_H

#include <string>
#include <netinet/in.h>
#include "tcpsocket.h"


class ClientTCPSocket : public TCPSocket
{
    public:
        ClientTCPSocket(std::string server,int port_n);
        void connect();
};

#endif // CLIENTTCPSOCKET_H
