#include "requesthandler.h"
#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <dirent.h>
#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <sstream>
#include <cstring>

RequestHandler::RequestHandler(TCPSocket* t_stream , char* t_dir) : stream(t_stream),dir(t_dir) { }

//Handles the request command
int RequestHandler::request(char* req)
{
    //Choose the function
    if(strcmp(req,"LIST") == 0)
        send_list();
    else if(strcmp(req,"QUIT") == 0)
        quit();
    else
    {

        std::string fileName="";
        std::string methode="";

        for(int i = 0;i<3;i++)
            methode += req[i];
        for(int i = 4;req[i] !='\0';i++)
            fileName += req[i];

        std::string miniBuffer = "";
        std::map <std::string , size_t> files = get_list();
        std::map <std::string , size_t>::iterator pos;
        for (pos = files.begin(); pos != files.end(); ++pos)
        {
                if(pos->first == fileName)
                miniBuffer = to_string(pos->second/LENGTH);
        }

        if(methode == "GET")
        {
           // std::cout << miniBuffer << " " << miniBuffer.size() << std::endl;
            stream->send(miniBuffer.c_str(),miniBuffer.size());
            stream->sendFile(fileName);
        }

        else if(methode =="PUT")
        {
            stream->recieveFile(fileName,100);
        }

    }


    return 0;
}

//Send The list of files and their sizes on the socket
void RequestHandler::send_list()
{
    //Save the list
    std::string dir_list = "";

    //Saving the list in one string to send on the socket
    std::map <std::string , size_t> files = get_list();
    std::map <std::string , size_t>::iterator pos;
    for (pos = files.begin(); pos != files.end(); ++pos)
        dir_list = dir_list + pos->first + "\t" + to_string(pos->second) + " bytes\n";

    stream->send(dir_list.c_str(), dir_list.size());
}

//Convert to String
template <typename T>
std::string RequestHandler::to_string(T const& value)
{
    std::stringstream sstr;
    sstr << value;
    return sstr.str();
}

//Return a map of files and sizes in a directory
std::map< std::string , size_t>  RequestHandler::get_list()
{
    DIR *dp;                                    //Directory
    struct dirent *dir_handler;                 //Directory Struct
    std::map <std::string , size_t> files;      //Files

    //Open The directory
    if((dp  = opendir(dir)) == NULL)
        std::cout << "Error opening the directory: " << dir << " : " << errno << std::endl;

    //Reading the name of the files into the file vector
    while ((dir_handler = readdir(dp)) != NULL)
        files[dir_handler->d_name] = getFileSize(dir_handler->d_name);


    //Close the directory and return the files
    closedir(dp);
    return files;
}

//Return the size of one file
double RequestHandler::getFileSize(char* filename)
{
    struct stat stat_buf; //File status struct

    //return the size of the file
    double rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

//Quiting the Stream
void RequestHandler::quit()
{
    delete stream;
}
