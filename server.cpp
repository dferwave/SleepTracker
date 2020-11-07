//The code used for Sockets was provided by: Dr. Rincon

#include <iostream>
#include <unistd.h>     //fork(), close(socket)
#include <stdio.h>      //fprintf(), stderr, perror()
#include <stdlib.h>     //exit() and atoi()
#include <string>
#include <string.h>     //bzero()
//#include <sys/socket.h>
//#include <sys/types.h>
#include <netinet/in.h> //struct sockaddr_in, INADDR_ANY, htons(),
//#include <netdb.h>
#include <sys/wait.h>   //waitpid(), signal(), SIGCHLD
#include <climits>
#include <cmath>

using namespace std;

void fireman(int)
{
    while(waitpid(-1, NULL, WNOHANG) > 0)
    {
        //cout << "A Child Process Ended" << endl; //Do Nothing
    }
    //Will wait for any 1 Child Process, WNOHANG (Will not block (hang), while waiting for the child process)
}

struct messageToServer
{
    int x;
	char string[256];
    //etc...
};
struct messageFromServer
{
    int y;
	char string[256];
    //etc...
};

void *calcValues(void* values_void_ptr)
{
    struct messageFromServer *values_ptr = (struct messageFromServer *)values_void_ptr;
    /*Calculations*/
    return NULL;
}

int main(int argc, char *argv[])
{
    signal(SIGCHLD, fireman);
    int sockfd, newsockfd, portno, clilen, n;
    struct sockaddr_in serv_addr, cli_addr;

    if(argc < 2) {
        fprintf(stderr, "ERROR no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    //This allows this socket to recieve requests from ANY address available on the Internet (depending on firewall)
    serv_addr.sin_port = htons(portno); //network datatype for "int" (since "int" can be different for different computers)
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
        //Will bind (attach) the server's empty socket w/ serv_addr
        perror("ERROR in binding");
        exit(1);
    }
    listen(sockfd, 5);  //Sets maximum number of connections server can recieve at any time
    clilen = sizeof(cli_addr);
    while(1)    //Infinite Loop, 1 = true
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        //accept() will accept a connect() function from a client using the sockfd socket
        //newsockfd socket will be the one used to send and recieve from the client
        if(fork() == 0)
        {
            if(newsockfd < 0) {
                perror("ERROR on accept");
                exit(1);
            }
            //bzero(buffer, 256);
            struct messageToServer msgFromC;
            n = read(newsockfd, &msgFromC, sizeof(struct messageToServer));
            if(n < 0) {
                perror("ERROR reading from socket");
                exit(1);
            }
            struct messageFromServer msgToC;
            /*START OF CALCULATION*/
        
            calcValues(&msgToC);
            
            n = write(newsockfd, &msgToC, sizeof(struct messageFromServer));
            if(n < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            close(newsockfd);
            exit(0);
        }
        close(newsockfd);
    }
    close(sockfd);
    return 0;
}
