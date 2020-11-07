// A simple server in the internet domain using TCP
// The port nu1mber is passed as an argument

// Please note this is a C program
// It compiles without warnings with gcc

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

struct message
{
        int option;
        int account;
        double amount;
};

struct message2
{
    int value;
    char buffer[256];
};

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char buffer[256];
     struct message msg1;
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     n = read(newsockfd,&msg1,sizeof(struct message));
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: Option: %d, Account %d, Amount=%lf\n",msg1.option,msg1.account,msg1.amount);
     struct message2 m2;
     m2.value = 1000;
     strcpy(m2.buffer,"Message from the server\0");
     n = write(newsockfd,&m2,sizeof(struct message2));
     if (n < 0) error("ERROR writing to socket");
     return 0; 
}
