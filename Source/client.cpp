#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "clienttcpsocket.h"

#define BUFFER_SIZE 4096

int main(int argc, char** argv)
{
    /*int i = myldapConnection();
    if(i != 0)
        return 0;
*/
    int len;
    std::string message;
    char line[BUFFER_SIZE];         //Message Buffer

    if(argc != 3)
    {
        std::cout << "usage: client <ip> <port>";
        return 0;
    }

    ClientTCPSocket* stream = new ClientTCPSocket(argv[1],atoi(argv[2]));
    stream->connect();          //Connecting to the socket

    while(stream != NULL ) {
        message = "";
        std::cout << "> ";
        getline(std::cin, message);     //Reading The commands

        //PUT and GET
        if(((message[0] == 'G') && (message[1] == 'E') && (message[2] == 'T')) || ((message[0] == 'P') && (message[1] == 'U') && (message[2] == 'T')) )
        {
            std::string fileName="";
            std::string methode="";

            for(int i = 0;i<3;i++)
                methode += message[i];

            for(int i = 4;message[i] !='\0';i++)
                fileName += message[i];

                if(methode == "PUT")
                {
                    stream->send(message.c_str(),message.size());
                    int fs = stream->sendFile(fileName);
                    if(fs != 0)
                        std::cout << "File cannot be sent!" << std::endl;
                }
                else if(methode == "GET")
                {
                    ssize_t len;
                    char miniBuffer[512];
                    stream->send(message.c_str(),message.size());
                    len = stream->receive(miniBuffer,sizeof(miniBuffer));
                    miniBuffer[len] = 0;
                    //std::string str(miniBuffer);

                    int fs = stream->recieveFile(fileName,atoi(miniBuffer));
                    if(fs != 0)
                        std::cout << "File cannot be downloaded!" << std::endl;
                }
        }
        else if(message == "LIST")
        {
            stream->send(message.c_str(), message.size());
            len = stream->receive(line, sizeof(line));
            line[len] = 0;
            printf("%s\n", line);
        }
        else if(message == "QUIT")
        {
            stream->send(message.c_str(), message.size());
            delete stream;
            std::cout << "QUITING" << std::endl;
            break;
        }
        else
            std::cout << "Unknown Command !!" << std::endl;
    }
    return 0;
}
