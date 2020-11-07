// Please note this is a C program
// It compiles without warnings with gcc
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(char *msg)
{
    perror(msg);
    exit(0);
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
    int sockfd, portno, n;
    struct message msg1;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    msg1.account=1234;
    msg1.option=0;
    msg1.amount = 0;
    n = write(sockfd,&msg1,sizeof(struct message));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    struct message2 msg2;
    n = read(sockfd,&msg2,sizeof(struct message2));
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%d %s\n",msg2.value,msg2.buffer);
    return 0;
}
