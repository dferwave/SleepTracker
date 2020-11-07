//The code used in this program for Sockets was provided by: Dr. Rincon

#include <iostream>
#include <unistd.h>     //fork(), close(socket)
#include <stdio.h>      //standard input/output | fprintf(), stderr, perror()
#include <stdlib.h>     //standard library | exit() and atoi()
#include <string.h>     //bzero() and bcopy()
//#include <sys/socket.h> //socket implementation
#include <netinet/in.h> //for network addresses
#include <netdb.h>      //network databases? | gethostbyname(), struct hostent
#include <vector>

using namespace std;

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
struct message_socket
{
    struct messageToServer msgToS;
    int sockfd, n;
    struct messageFromServer msgFromS;
};

void *calcValues_server (void *msg_server_void_ptr)
{
    struct message_socket *msg_server_ptr = (struct message_socket *)msg_server_void_ptr;
    //printf("<In calcValues_server...\tsockfd = %d\n\t\tipAddr: ", msg_server_ptr->sockfd);   //DEBUG
    msg_server_ptr->n = write(msg_server_ptr->sockfd, &msg_server_ptr->msgToS, sizeof(struct messageToServer)); //Send Message (Input) to Server
    //cout << msg_server_ptr->m.ipAddr << "\tsubnetMask: " << msg_server_ptr->m.subnetMask << endl << "\t\t\tn = " << msg_server_ptr->n << ">" << endl; //DEBUG
    if(msg_server_ptr->n < 0) {
        //printf("<sockfd: %d\t>", msg_server_ptr->sockfd); //DEBUG
        perror("ERROR writing to socket");
        exit(0);
    }
    msg_server_ptr->n = read(msg_server_ptr->sockfd, &msg_server_ptr->msgFromS, sizeof(struct messageFromServer));  //Receive Message from Server
    if(msg_server_ptr->n < 0) {
        perror("ERROR reading from socket");
        exit(0);
    }
    close(msg_server_ptr->sockfd);
    return NULL;
};

int main(int argc, char *argv[]) //argc = argument count, argv = values of the arguments/argument vector (argv[0] = main)
{
    int portno; //sockfd, n
    struct sockaddr_in serv_addr;   //in = connects via internet
    struct hostent *server;         //hostent = host entity

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);     //argument to integer?
    server = gethostbyname(argv[1]);
    if (server == NULL) {   //host does not exist
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr)); //fills (char *) var with '0's, that is, (\0)'s
    serv_addr.sin_family = AF_INET;  //set family of serv_addr to INET
    bcopy((char *) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
    //copy server address to serv_addr's s_addr
    serv_addr.sin_port = htons(portno);
    /*set port of serv_addr to htons (a family of functions,
      that transforms the portno into a network datatype [int is not always the same for all computers])
    */
    
    /*Put Input into Message that will be Sent to Server:*/
    /*printf("Enter Message (IP Address): ");
    cin >> input;
    strcpy(msg.ipAddr, input.c_str());
    //printf("Enter Message (Subnet Mask): ");
    cin >> input;
    strcpy(msg.subnetMask, input.c_str());
    cout << input << endl;*/
    string input;
    vector<string> v;
    int totalGiven = 0; //Number of IP Addresses Given (Host IP Addresses + Subnet Masks)
    while(cin >> input)
    {
        v.push_back(input);
        totalGiven++;
    }
    const int inputLines = totalGiven;
    struct message_socket msgs_server[inputLines];
    pthread_t tid[inputLines];

    for(int i = 0, j = 0; i < inputLines; i++)
    {
        struct messageToServer msg;
        //printf("Message [%d]:\t", i ...);    //DEBUG
        msgs_server[i].msgToS = msg;
        //msg_server.etc = ;
        msgs_server[i].sockfd = socket(AF_INET, SOCK_STREAM, 0); //empty socket
        /* Parameters: (Family/Internet (INET) Protocols,
                    Socket Type (SOCK_STREAM),
                    Option [Normally 0]
        */
        if (msgs_server[i].sockfd < 0) {   //Should be nonnegative
            perror("ERROR opening socket");
            exit(0);
        }
        if(connect(msgs_server[i].sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { //connect w/ server
            perror("ERROR connecting");
            exit(0);
        }
        //printf("sockfd: %d\n", msgs_server[i].sockfd);  //DEBUG

        if(pthread_create(&tid[i], NULL, calcValues_server, &msgs_server[i]))
        {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }
    for(int i = 0; i < inputLines; i++)
        pthread_join(tid[i], NULL);
    
    return 0;
}
