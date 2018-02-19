#include "servertcpsocket.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>

ServerTCPSocket::ServerTCPSocket(std::string server,int port_n): TCPSocket()
{
    listner = false;
    port = port_n;
    init(server);
}
ServerTCPSocket::ServerTCPSocket(int port_n): TCPSocket()
{
    listner = false;
    port = port_n;
    init("");
}

int ServerTCPSocket::bindAndListen()
{
    //Free the Socket if it is in use
    int optval = 1;
    setsockopt(so_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    //std::cout << "Binding to the socket ..." << std::endl;
    //bind the socket and address
    int result = bind(so_fd, (struct sockaddr *)address, addr_size);
    if (result != 0)
    {
        perror("bind() failed");
        return result;
    }

    //std::cout << "Listening ..." << std::endl;
    //start listening
    result = listen(so_fd, 5);
    if (result != 0)
    {
        perror("listen() failed");
        return result;
    }
    //socket is listening
    listner = true;
    return result;
}

TCPSocket* ServerTCPSocket::accept()
{
    if (listner == false)
    {
        return NULL;
    }

    //accept the connection
    sd = ::accept(so_fd, (struct sockaddr *)address, &addr_size);
    //std::cout << "listening  ...." << std::endl;
    if (sd < 0)
    {
        perror("accept() failed");
        return NULL;
    }

    //return the new socket
    return new TCPSocket(address,sd);
}
