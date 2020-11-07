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
        //cout << "A Child Process Ended" << endl; //Do Nothing?
    }
    //Will wait for any 1 Child Process, WNOHANG (Will not block, while waiting for the child process)
}

struct message  //input
{
	char ipAddr[16];
	char subnetMask[16];
};
struct ipAddrOctets
{
    u_int8_t ipAddr = 0, subnetMask = 0;
    u_int8_t networkAddr = 0, broadcastAddr = 0;
};
struct ipAddrValues
{
    struct ipAddrOctets octets[4];
    u_int8_t minHostAddrOctet = 0, maxHostAddrOctet = 0;    //Only Last Octets Change (NetworkAddr+1,  BroadcastAddr-1)
    int hostNum = 0;                                        //2^(SubnetMask_0Bits)-2
};

void *calcValues(void* values_void_ptr)
{
    struct ipAddrValues *values_ptr = (struct ipAddrValues *)values_void_ptr;
    for(int i = 0; i < 4; i++)  //4 Octets
    {
        values_ptr->octets[i].networkAddr = values_ptr->octets[i].ipAddr & values_ptr->octets[i].subnetMask;
        values_ptr->octets[i].broadcastAddr = values_ptr->octets[i].networkAddr | ~values_ptr->octets[i].subnetMask;
        u_int8_t temp = values_ptr->octets[i].subnetMask;
        for(int j = 0; j < CHAR_BIT * sizeof temp; j++, temp >>= 1)
            if((temp & 1) == 0)
                values_ptr->hostNum++; //Counts amount of 0s in Each Octet of the Subnet Mask

        if(i == 3)
        {
            values_ptr->hostNum = pow(2, values_ptr->hostNum) - 2;
            values_ptr->minHostAddrOctet = values_ptr->octets[i].networkAddr + 1;
            values_ptr->maxHostAddrOctet = values_ptr->octets[i].broadcastAddr - 1;
        }
    }
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
    serv_addr.sin_port = htons(portno); //network datatype for "int"
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
        //Will bind the server's empty socket w/ serv_addr
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
            struct message msg;
            n = read(newsockfd, &msg, sizeof(struct message));
            if(n < 0) {
                perror("ERROR reading from socket");
                exit(1);
            }
            struct ipAddrValues m2;
            /*START OF CALCULATION*/
            /*DEBUG:
            printf("m2.ipAddr: %s\nm2.subnetMask: %s\n", msg.ipAddr, msg.subnetMask);
            for(int i = 0; i < 16; i++)
            {
                if(msg.ipAddr[i] == 0)
                {
                    printf("NULL\t");
                }
                printf("%d\t%d\n", msg.ipAddr[i], msg.subnetMask[i]);
            }*/
            //Get IP Address (Octets):
            for(int o = 0, i = 0; i < 16 && msg.ipAddr[i] != 0; i++)
            {
                string value = "";
                for(; msg.ipAddr[i] != '.' && msg.ipAddr[i] != 0; i++) {
                    value += msg.ipAddr[i];
                }
                m2.octets[o++].ipAddr = stoi(value);
                //printf("value %d: %d\n", o, stoi(value));
            }
            //Get Subnet Mask (Octets):
            for(int o = 0, i = 0; i < 16 && msg.subnetMask[i] != 0; i++)
            {
                string value = "";
                for(; msg.subnetMask[i] != '.' && msg.subnetMask[i] != 0; i++) {
                    value += msg.subnetMask[i];
                }
                m2.octets[o++].subnetMask = stoi(value);
                //printf("value %d: %d\n", o, stoi(value));
            }
        
            calcValues(&m2);
            /*DEBUG
            printf("IP Address: %d.%d.%d.%d\n", m2.octets[0].ipAddr, m2.octets[1].ipAddr,
                m2.octets[2].ipAddr, m2.octets[3].ipAddr);
            printf("Subnet: %d.%d.%d.%d\n", m2.octets[0].subnetMask, m2.octets[1].subnetMask,
                m2.octets[2].subnetMask, m2.octets[3].subnetMask);
            printf("Network: %d.%d.%d.%d\n", m2.octets[0].networkAddr, m2.octets[1].networkAddr,
                m2.octets[2].networkAddr, m2.octets[3].networkAddr);
            printf("Broadcast: %d.%d.%d.%d\n", m2.octets[0].broadcastAddr, m2.octets[1].broadcastAddr,
                m2.octets[2].broadcastAddr, m2.octets[3].broadcastAddr);
            printf("HostMin: %d.%d.%d.%d\n", m2.octets[0].networkAddr, m2.octets[1].networkAddr,
                m2.octets[2].networkAddr, m2.minHostAddrOctet);
            printf("HostMax: %d.%d.%d.%d\n", m2.octets[0].broadcastAddr, m2.octets[1].broadcastAddr,
                m2.octets[2].broadcastAddr, m2.maxHostAddrOctet);
            printf("# Hosts: %d\n", m2.hostNum);*/
            
            n = write(newsockfd, &m2, sizeof(struct ipAddrValues));
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