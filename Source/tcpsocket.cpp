#include "tcpsocket.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cmath>

TCPSocket::TCPSocket(){}
TCPSocket::TCPSocket(struct sockaddr_storage* addr, int sock_fd):address(addr),so_fd(sock_fd)
{
    //char char_ip[140];
    //inet_ntop(PF_INET, (struct in_addr*)&(address->sin_addr.s_addr), char_ip, sizeof(char_ip)-1);
    //ip = char_ip;
}
//Initialize the Socket and the corresponding members
int TCPSocket::init(std::string sever)
{
    int status;
    //std::cout << "Beginning the process ..." << std::endl;

    //Looking for the first valid Address
    status = retrieveHostAddr(sever.c_str());

    if(status == 0)
    {
        for(struct addrinfo* mainSockAddr = res; mainSockAddr != NULL; mainSockAddr = mainSockAddr->ai_next)
        {
            void *addr;     //The IP address Container
            if (mainSockAddr->ai_family == AF_INET)
            { // IPv4
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)mainSockAddr->ai_addr; //Casting ai_addr to sock_addr_in
                addr = &(ipv4->sin_addr);
                ipver = "IPv4";
                addr_size = (socklen_t) sizeof(struct sockaddr_in);
                //Saving the address in Sock Storage
                address = (struct sockaddr_storage *)ipv4;
            }
            else
            { // IPv6
                struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)mainSockAddr->ai_addr;//Casting ai_addr to sock_addr_in
                addr = &(ipv6->sin6_addr);
                ipver = "IPv6";
                addr_size = (socklen_t) sizeof(struct sockaddr_in6);
                //Saving the address in Sock Storage
                address = (struct sockaddr_storage *)ipv6;
            }

            // convert the IP to a string
            char char_ip[INET6_ADDRSTRLEN];
            inet_ntop(mainSockAddr->ai_family, addr, char_ip, sizeof char_ip);
            ip = char_ip;



            //Creating the socket using the first retrieved IP
            if((so_fd = socket(mainSockAddr->ai_family, mainSockAddr->ai_socktype,mainSockAddr->ai_protocol)) == -1)
                continue;
        }

    }
    else
    {
        //perror("Host cannot be retrieved.");
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res); //Freeing the rest of addresses

    //Check if the Socket is established
    if(so_fd != -1)
    {
        //std::cout << "Socket has been created" << std::endl;
        return 0;
    }
    else
        return -1;
}

//Retrieve The a valid address
int TCPSocket::retrieveHostAddr(const char* hostname)
{
    struct addrinfo init;
    int result;

    memset(&init, 0, sizeof init);  // Setting the bits to 0
    init.ai_family = AF_UNSPEC;     // IPv4 or IPv6
    init.ai_socktype = SOCK_STREAM; // TCP stream sockets

    //converting the port number(int) to char*
    std::stringstream convert;
    convert << port;
    std::string s = convert.str();
    const char* pchar = s.c_str();
    //std::cout << "Looking for the host ..." << std::endl;

    //Check if a host has been set
    //If not it uses current system IP
    if(strlen(hostname) == 0)
    {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[128];

       if (getifaddrs(&ifaddr) == -1) {
            perror("getifaddrs");
            exit(EXIT_FAILURE);
        }

       // Walk through linked list, maintaining head pointer so we can free list later
       for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
       {
            if (ifa->ifa_addr == NULL)
                continue;

           family = ifa->ifa_addr->sa_family;

          // For an AF_INET* interface address, display the address
           if (family == AF_INET /*|| family == AF_INET6*/)
           {
                s = getnameinfo(ifa->ifa_addr,(family == AF_INET) ? sizeof(struct sockaddr_in) :sizeof(struct sockaddr_in6),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0)
                {
                    printf("getnameinfo() failed: %s\n", gai_strerror(s));
                    exit(EXIT_FAILURE);
                }
            }
        }

       freeifaddrs(ifaddr);
       result = getaddrinfo (host, pchar, &init, &res);
    }
    else
        result = getaddrinfo (hostname, pchar, &init, &res);

    //set the results
    if(result != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        return -1;
    }
    //std::cout << "Hosts have been listed ..." << std::endl;

    return result;
}

//Sending Data
ssize_t TCPSocket::send(const char* buffer, size_t len)
{
    return write(so_fd, buffer, len);
}

//Receiving Data
ssize_t TCPSocket::receive(char* buffer, size_t len)
{
    return read(so_fd, buffer, len);
}
int TCPSocket::sendFile(std::string fs_name)
{
    char sdbuf[LENGTH]; // Send buffer
    FILE *fs = fopen(fs_name.c_str(), "r"); //Opening the file
    if(fs == NULL)
    {
        std::cout << "ERROR: File " <<  fs_name << " cannot be opened. errno = " <<  errno << std::endl;
        return -1;
    }
    bzero(sdbuf, LENGTH);

    int fs_block_sz;
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0) //Sending The file
    {
        if(::send(so_fd, sdbuf, fs_block_sz, 0) < 0)
        {
            std::cout << "ERROR: File " <<  fs_name << " cannot be sent. errno = " <<  errno << std::endl;
            return -1;
        }
        bzero(sdbuf, LENGTH);
    }
    fclose(fs);
    return 0;
}
int TCPSocket::recieveFile(std::string fr_name,int blockcount)
{
    printf("%d\n\n\n",blockcount);
    FILE *fr = fopen(fr_name.c_str(), "a");
    char revbuf[LENGTH]; // Recieve buffer

    if(fr == NULL)
    {
        std::cout << "ERROR: File " <<  fr_name << " cannot be opened. errno = " <<  errno << std::endl;
        return -1;
    }
    bzero(revbuf, LENGTH);
    int fr_block_sz = 0;
    int block = 0;
    while((fr_block_sz = ::recv(so_fd, revbuf, LENGTH, 0)) > 0) //Recieving the file
    {
        int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr); //Wrting to the file
        if(write_sz < fr_block_sz)
        {
            std::cout  << "File write failed." << std::endl;
        }

        bzero(revbuf, LENGTH);

        if (fr_block_sz == 0 || fr_block_sz != LENGTH)
                break;

        if(fr_block_sz < 0)
        {
            if (errno == EAGAIN)
                std::cout << "recv() timed out.\n" << std::endl;
            else
            {
                fprintf(stderr, "recv() failed due to errno = %d\n", errno);
                return -1;
            }
        }
        block++;
        std::cout <<ceilf(((100 / (float)blockcount ) * (float)block )*100)/100<< "%";
        std::cout.flush();
        }
        /*float progress = (((float)block / (float)blockcount));
        std::cout << progress;
        int barWidth = 60;
        std::cout << "  Downloading ... [";
        int pos = barWidth * progress;
        if(block == blockcount-1){
            for (int i = 0; i < barWidth; ++i)
            {
                std::cout << "#";
            }
            std::cout << "] 100%\r";
            //std::cout.flush();
            block++;
        }
        else
        {
            for (int i = 0; i < barWidth; ++i)
            {
                 if (i <= pos) std::cout << "#";
                       else std::cout << "-";
            }
            std::cout << "] " << round(progress*100.0) << "%\r";
            //std::cout.flush();
        }
    }*/
    fclose(fr);
    return 0;

}
//Return Port Number
int TCPSocket::getPort()
{
    return port;
}
//Return IP Version
std::string TCPSocket::getIPVer()
{
    return ipver;
}

//Return IP
std::string TCPSocket::getIP()
{
    return ip;
}

TCPSocket::~TCPSocket()
{
    close(so_fd);
}
