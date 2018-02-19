#include "clienttcpsocket.h"
#include <stdio.h>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

ClientTCPSocket::ClientTCPSocket(std::string server,int port_n) : TCPSocket()
{
    port = port_n;  //Set the port
    init(server);   //Start the Socket
}

//Connect to the socket
void ClientTCPSocket::connect()
{
    if (::connect(so_fd, (struct sockaddr*)address, addr_size) != 0)
        perror("connect() failed");
}
