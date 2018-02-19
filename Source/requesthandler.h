#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <tcpsocket.h>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <map>

#define BUFFER_SIZE 4096
class RequestHandler
{
    public:
        RequestHandler(TCPSocket* t_stream , char* t_dir);
        int request(char* req);

    private:
        TCPSocket* stream;      //Main stream
        char* dir;              //Directory of the Requests

        void send_list();
        std::map< std::string , size_t> get_list();
        double getFileSize(char* filename);
        template <typename T>
        std::string to_string(T const& value);

        void quit();
};

#endif // REQUESTHANDLER_H
