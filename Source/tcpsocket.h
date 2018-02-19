#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

#define LENGTH 1000
class TCPSocket
{
    public:
        TCPSocket();
        TCPSocket(struct sockaddr_storage* addr, int sock_fd);
        ~TCPSocket();
        ssize_t send(const char* buffer, size_t len);
        ssize_t receive(char* buffer, size_t len);
        int sendFile(std::string fs_name);
        int recieveFile(std::string fs_name,int blockcount);
        std::string getIP();
        std::string getIPVer();
        int getPort();

    protected:
        struct sockaddr_storage* address;    //Address in the struct
        int port;                           //Port number
        int so_fd;                          //Socket File descriptor
        socklen_t addr_size;                 //Size of the address

        int init(std::string server);

    private:
        struct addrinfo *res;               //Save The retrieved address
        std::string ipver;                  //IP Version Container
        std::string ip;                     //IP address in string

        int retrieveHostAddr(const char* hostname);

};

#endif // TCPSOCKET_H
