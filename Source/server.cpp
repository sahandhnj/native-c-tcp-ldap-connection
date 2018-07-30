#include "servertcpsocket.h"
#include "requesthandler.h"

#include <map>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <ldap.h>
#include <termios.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

#define LDAP_HOST "test.com"
#define LDAP_PORT 389
#define SEARCHBASE "dc=test,dc=com"
#define SCOPE LDAP_SCOPE_SUBTREE
#define BIND_USER NULL		/* anonymous bind with user and pw NULL */
#define BIND_PW NULL


char* port;
TCPSocket *stream;
int ldapConnect(std::string username_input, std::string passsword_input);
void ShowStdinKeystrokes();
void HideStdinKeystrokes();

void *handle(void *args)
{
    RequestHandler *request = NULL;
    while(1)
    {
        if (request == NULL)
            request = new RequestHandler(stream,port);

        while(request != NULL && stream != NULL)
        {
            ssize_t len;
            char line[BUFFER_SIZE];
            while ((len = stream->receive(line, sizeof(line))) > 0)
            {
                pthread_join(pthread_self(), NULL);
                std::cout << "JOIN" << std::endl;
                line[len] = 0;
                printf("received - %s\n", line);
                request->request(line);
                pthread_detach(pthread_self());
                std::cout << "DETACH" << std::endl;
            }
        }

        if(request != NULL)
        {
            delete request;
            pthread_exit(NULL);
            continue;
        }
        return NULL;
    }
}

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cout << "usage: server <ip> <port>";
        return 0;
    }
    port = argv[2];
    ServerTCPSocket *first = new ServerTCPSocket(atoi(argv[1]));

    first->bindAndListen();
    std::cout << first->getIP() << std::endl;

    //TCPSocket *stream;
    while(1)
    {
        stream = first->accept();
        if(stream != NULL)
        {
            pthread_t thread;
            if (pthread_create(&thread, NULL, handle, NULL) != 0)
                fprintf(stderr, "Failed to create thread\n");
        }
    }
    delete first;
    return 0;
}


int ldapConnect(std::string username_input, std::string passsword_input)
{
   ShowStdinKeystrokes();
   std::cout << std::string( 100, '\n' );
   std::cout << "==============================" << std::endl;
   LDAP *ld;			/* LDAP resource handle */
   LDAPMessage *result, *e;	/* LDAP result handle   */
   BerElement *ber;		/* array of attributes  */
   char *attribute;
   char **vals;

   int /*i*/rc=0;

   char *attribs[3];		/* attribute array for search */

   attribs[0]=strdup("uid");		/* return uid and cn of entries */
   attribs[1]=strdup("cn");
   attribs[2]=NULL;		/* array must be NULL terminated */


   /* setup LDAP connection */
   if ((ld=ldap_init(LDAP_HOST, LDAP_PORT)) == NULL)
   {
      perror("ldap_init failed");
      return 2;
   }

   printf("Server: %s \nPort:   %d\n",LDAP_HOST,LDAP_PORT);
   std::cout << "==============================" << std::endl;

   /* anonymous bind */
   rc = ldap_simple_bind_s(ld, BIND_USER, BIND_PW);

   if (rc != LDAP_SUCCESS)
   {
      fprintf(stderr,"LDAP error: %s\n",ldap_err2string(rc));
      return 2;
   }
   else
   {
      printf("Bind to LDAP server successful.\n");
   }
   int checker = 0;
   int succ = 0;
   while((checker < 3) && (succ == 0)) {

	   /* get login */
	   const char * uid = username_input.c_str();
           const char * password = passsword_input.c_str();

	   /* create a filter */
	   char USERNAME[20];
	   memset(USERNAME, 0, 20);
	   strcat(USERNAME, "(uid=");
	   strcat(USERNAME, uid);
	   strcat(USERNAME, ")");

	   /* perform ldap search */
	   rc = ldap_search_s(ld, SEARCHBASE, SCOPE, USERNAME, attribs, 0, &result);

	   if(ldap_count_entries(ld, result) == 0) {
	      std::cout << "\nWrong credentials. (" << checker+1 << "/3)" << std::endl;
	      std::cout << USERNAME << std::endl;
              std::cout << password << std::endl;
	      checker++;
           }

	   char* DN = NULL;

	   for (e = ldap_first_entry(ld, result); e != NULL; e = ldap_next_entry(ld,e))
	   {
	      DN = ldap_get_dn(ld, e);

	      LDAP *ld2;
	      ld2=ldap_init(LDAP_HOST, LDAP_PORT);
	      rc = ldap_simple_bind_s(ld2, DN , password);
	      if (rc != LDAP_SUCCESS) {
		 std::cout << "\nWrong credentials. (" << checker+1 << "/3)" << std::endl;
		 std::cout << USERNAME << std::endl;
                 std::cout << password << std::endl;
		 checker++;
		 continue;
	      }

	      /* Now search through each entry */

	      for (attribute = ldap_first_attribute(ld,e,&ber); attribute!=NULL; attribute = ldap_next_attribute(ld,e,ber))
	      {
		 if ((vals=ldap_get_values(ld,e,attribute)) != NULL)
		 {
		    /* free memory used to store the attribute */
		    ldap_memfree(attribute);
		    succ = 1;
		    break;
		 }
	      }
	      /* free memory used to store the value structure */
	      if (ber != NULL) ber_free(ber,0);
	      printf("\n");
	   }
   }

   /* free memory used for result */
   ldap_msgfree(result);
   free(attribs[0]);
   free(attribs[1]);
   ldap_unbind(ld);

   if (checker > 2) {
      std::cout << "Banning your IP." << std::endl;
      return 1;
   }
   std::cout << "Success!" << std::endl;
   return 0;
}

void ShowStdinKeystrokes()
{
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void HideStdinKeystrokes()
{
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}
