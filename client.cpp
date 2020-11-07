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

struct message
{
	char ipAddr[16];     //4 Octets (At Most: 3 characters each) + 3 '.' + '\0' = 16 characters total
	char subnetMask[16];   //Same For the Subnet Mask (Max of 16 Characters)
};
struct ipAddrOctets
{
    u_int8_t ipAddr = 0, subnetMask = 0;
    u_int8_t networkAddr = 0, broadcastAddr = 0; //, minHostAddrOctet, maxHostAddrOctet;
};
struct ipAddrValues
{
    struct ipAddrOctets octets[4];
    u_int8_t minHostAddrOctet = 0, maxHostAddrOctet = 0;    //Only Last Octets Change (NetworkAddr+1,  BroadcastAddr-1)
    int hostNum = 0;                                        //2^(SubnetMask_0Bits)-2
};
struct message_socket
{
    struct message m;
    int sockfd, n;
    struct ipAddrValues calc;   //Calculated Values returned from the Server
};

void *calcValues_server (void *msg_server_void_ptr)
{
    struct message_socket *msg_server_ptr = (struct message_socket *)msg_server_void_ptr;
    //printf("<In calcValues_server...\tsockfd = %d\n\t\tipAddr: ", msg_server_ptr->sockfd);   //DEBUG
    msg_server_ptr->n = write(msg_server_ptr->sockfd, &msg_server_ptr->m, sizeof(struct message)); //Send Message (Input) to Server
    //cout << msg_server_ptr->m.ipAddr << "\tsubnetMask: " << msg_server_ptr->m.subnetMask << endl << "\t\t\tn = " << msg_server_ptr->n << ">" << endl; //DEBUG
    if(msg_server_ptr->n < 0) {
        //printf("<sockfd: %d\t>", msg_server_ptr->sockfd); //DEBUG
        perror("ERROR writing to socket");
        exit(0);
    }
    msg_server_ptr->n = read(msg_server_ptr->sockfd, &msg_server_ptr->calc, sizeof(struct ipAddrValues));  //Receive Calculation from Server
    if(msg_server_ptr->n < 0) {
        perror("ERROR reading from socket");
        exit(0);
    }
    /*DEBUG
    printf("IP Address: %d.%d.%d.%d\n", msg_server_ptr->calc.octets[0].ipAddr, msg_server_ptr->calc.octets[1].ipAddr,
        msg_server_ptr->calc.octets[2].ipAddr, msg_server_ptr->calc.octets[3].ipAddr);
    printf("Subnet: %d.%d.%d.%d\n", msg_server_ptr->calc.octets[0].subnetMask, msg_server_ptr->calc.octets[1].subnetMask,
        msg_server_ptr->calc.octets[2].subnetMask, msg_server_ptr->calc.octets[3].subnetMask);
    printf("Network: %d.%d.%d.%d\n", msg_server_ptr->calc.octets[0].networkAddr, msg_server_ptr->calc.octets[1].networkAddr,
        msg_server_ptr->calc.octets[2].networkAddr, msg_server_ptr->calc.octets[3].networkAddr);
    printf("Broadcast: %d.%d.%d.%d\n", msg_server_ptr->calc.octets[0].broadcastAddr, msg_server_ptr->calc.octets[1].broadcastAddr,
        msg_server_ptr->calc.octets[2].broadcastAddr, msg_server_ptr->calc.octets[3].broadcastAddr);
    printf("HostMin: %d.%d.%d.%d\n", msg_server_ptr->calc.octets[0].networkAddr, msg_server_ptr->calc.octets[1].networkAddr,
        msg_server_ptr->calc.octets[2].networkAddr, msg_server_ptr->calc.minHostAddrOctet);
    printf("HostMax: %d.%d.%d.%d\n", msg_server_ptr->calc.octets[0].broadcastAddr, msg_server_ptr->calc.octets[1].broadcastAddr,
        msg_server_ptr->calc.octets[2].broadcastAddr, msg_server_ptr->calc.maxHostAddrOctet);
    printf("# Hosts: %d\n", msg_server_ptr->calc.hostNum);*/
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
    
    /*Put Input into Message that will be Sent to Server*/
    
    //bzero(buffer, 256);
    string input;
    vector<string> v;
    int totalAddrGiven = 0; //Number of IP Addresses Given (Host IP Addresses + Subnet Masks)
    while(cin >> input)
    {
        v.push_back(input);
        totalAddrGiven++;
    }
    /*printf("Enter Message (IP Address): ");
    cin >> input;
    strcpy(msg.ipAddr, input.c_str());
    //printf("Enter Message (Subnet Mask): ");
    cin >> input;
    strcpy(msg.subnetMask, input.c_str());
    cout << input << endl;*/

    const int inputLines = totalAddrGiven/2;    //Each Input Line: IP Address + Subnet Mask
    //struct ipAddrValues values[inputLines];     //Calculated Values
    struct message_socket msgs_server[inputLines];
    pthread_t tid[inputLines];

    for(int i = 0, j = 0; i < inputLines; i++)
    {
        struct message msg;
        bzero(msg.ipAddr, sizeof(msg.ipAddr));
        bzero(msg.subnetMask, sizeof(msg.subnetMask));
        strcpy(msg.ipAddr, v.operator[](j++).c_str());
        strcpy(msg.subnetMask, v.operator[](j++).c_str());
        //printf("Message [%d]:\t", i);    //DEBUG
        //cout << "ipAddr: " << v.operator[](j-2) << "\tsubnetMask: " << v.operator[](j-1) << "\t";   //DEBUG
        //struct message_socket msg_server;
        msgs_server[i].m = msg;
        //msg_server.sockfd = sockfd;
        //msg_server.calc = values[i];
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

        /*int n = write(msg_server.sockfd, &msg, sizeof(struct message));
        cout << "\t\tn = " << n << endl;*/
        if(pthread_create(&tid[i], NULL, calcValues_server, &msgs_server[i]))
        {
            fprintf(stderr, "Error: pthread_create\n");
            return 1;
        }
        //values[i] = msgs_server[i].calc;
        //close(sockfd);
    }

    for(int i = 0; i < inputLines; i++)
        pthread_join(tid[i], NULL);

    /*n = write(sockfd, &msg, sizeof(struct message)); //Send Message (Input) to Server
    if(n < 0) {
        perror("ERROR writing to socket");
        exit(0);
    }*/
    
    /*struct ipAddrValues msg2;
    n = read(sockfd, &msg2, sizeof(struct ipAddrValues));  //Receive Calculation from Server
    if(n < 0) {
        perror("ERROR writing to socket");
        exit(0);
    }*/
    //Print Results:
    /*printf("IP Address: %d.%d.%d.%d\n", msg2.octets[0].ipAddr, msg2.octets[1].ipAddr,
        msg2.octets[2].ipAddr, msg2.octets[3].ipAddr);
    printf("Subnet: %d.%d.%d.%d\n", msg2.octets[0].subnetMask, msg2.octets[1].subnetMask,
        msg2.octets[2].subnetMask, msg2.octets[3].subnetMask);
    printf("Network: %d.%d.%d.%d\n", msg2.octets[0].networkAddr, msg2.octets[1].networkAddr,
        msg2.octets[2].networkAddr, msg2.octets[3].networkAddr);
    printf("Broadcast: %d.%d.%d.%d\n", msg2.octets[0].broadcastAddr, msg2.octets[1].broadcastAddr,
        msg2.octets[2].broadcastAddr, msg2.octets[3].broadcastAddr);
    printf("HostMin: %d.%d.%d.%d\n", msg2.octets[0].networkAddr, msg2.octets[1].networkAddr,
        msg2.octets[2].networkAddr, msg2.minHostAddrOctet);
    printf("HostMax: %d.%d.%d.%d\n", msg2.octets[0].broadcastAddr, msg2.octets[1].broadcastAddr,
        msg2.octets[2].broadcastAddr, msg2.maxHostAddrOctet);
    printf("# Hosts: %d\n", msg2.hostNum);*/

    for(int i = 0; i < inputLines; i++)
    {
        printf("IP Address: %d.%d.%d.%d\n", msgs_server[i].calc.octets[0].ipAddr, msgs_server[i].calc.octets[1].ipAddr,
            msgs_server[i].calc.octets[2].ipAddr, msgs_server[i].calc.octets[3].ipAddr);
        printf("Subnet: %d.%d.%d.%d\n", msgs_server[i].calc.octets[0].subnetMask, msgs_server[i].calc.octets[1].subnetMask,
            msgs_server[i].calc.octets[2].subnetMask, msgs_server[i].calc.octets[3].subnetMask);
        printf("Network: %d.%d.%d.%d\n", msgs_server[i].calc.octets[0].networkAddr, msgs_server[i].calc.octets[1].networkAddr,
            msgs_server[i].calc.octets[2].networkAddr, msgs_server[i].calc.octets[3].networkAddr);
        printf("Broadcast: %d.%d.%d.%d\n", msgs_server[i].calc.octets[0].broadcastAddr, msgs_server[i].calc.octets[1].broadcastAddr,
            msgs_server[i].calc.octets[2].broadcastAddr, msgs_server[i].calc.octets[3].broadcastAddr);
        printf("HostMin: %d.%d.%d.%d\n", msgs_server[i].calc.octets[0].networkAddr, msgs_server[i].calc.octets[1].networkAddr,
            msgs_server[i].calc.octets[2].networkAddr, msgs_server[i].calc.minHostAddrOctet);
        printf("HostMax: %d.%d.%d.%d\n", msgs_server[i].calc.octets[0].broadcastAddr, msgs_server[i].calc.octets[1].broadcastAddr,
            msgs_server[i].calc.octets[2].broadcastAddr, msgs_server[i].calc.maxHostAddrOctet);
        printf("# Hosts: %d\n", msgs_server[i].calc.hostNum);
        if(i < inputLines-1)
            cout << endl;
    }
    return 0;
}