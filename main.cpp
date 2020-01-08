#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <bits/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>

#define MAX_SIZE 516 //TODO: make sure we know what is the right size
#define ACK_SIZE 4*sizeof(char)
#define END_WRQ_SIZE 50
#define WRQ 2
#define DATA 3

using namespace std;

const int WAIT_FOR_PACKET_TIMEOUT = 3;
const int NUMBER_OF_FAILURES = 7;


//struct sockaddr{
//    unsigned  short sa_family; //address family
//    char sa_data[14]; //14 bytes of protocl address
//};
//struct in_addr{
//    unsigned long s_addr; //32-bit long, (4 bytes) ip address
//};
//struct sockaddr_in{
//    short int sin_family;       //Address family
//    unsigned short int sin_port;//port number
//    struct in_addr sin_addr;    //internet address(ip)
//    unsigned char sin_zero[8];  //for allignments
//}__attribute__((packed));

void error(char* msg){
    string msgErr = "ERROR: " + string(msg);
    perror(msgErr.c_str());
    exit(1);
}



int main(int argc, char* argv[]) {
    int sock,recvMsgSize;
    struct sockaddr_in my_addr={0}, client_addr= {0};
    unsigned int client_addr_len;
    unsigned short portno;
    char buffer[MAX_SIZE]; //TODO: change to byte
    //struct hostent *server;

    if(argc <2){
        fprintf(stderr, "ERROR: no port provided\n");
    }

    if((sock = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP)) < 0 ){
        error("creating socket failed");
    }

    memset(&my_addr,0,sizeof(my_addr));

    portno = atoi(argv[1]);
  //TODO:  if(portno == 0) error("Invalid port number");
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(portno);

    if(bind(sock, (struct sockaddr*)&my_addr,sizeof(my_addr)) < 0){
        error("on binding");
    }

    // block until message received from client
    if((recvMsgSize=recvfrom(sock,buffer,MAX_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len) < 0){
        error("recvfrom() failed");
    }

    if(strcmp(buffer[1],WRQ)){
        error("first message is not WRQ");
    }

    Encoding.ASCII.GetString(buffer,3, buffer.length-END_WRQ_SIZE).Trim('\0');




    //sent ack 0
    if((sendto(sock,buffer,ACK_SIZE, 0, (sockaddr*)&client_addr,sizeof(client_addr)) != ACK_SIZE){
        error("sendto() sent a different number of bytes than expected");
    }

    //fileStream.Write(buffer,3, )

    do
    {
        do
        {
            do
            {
                // TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears
                // for us at the socket (we are waiting for DATA)
                if ()// TODO: if there was something at the socket and
                    // we are here not because of a timeout
                {
                    // TODO: Read the DATA packet from the socket (at
                    // least we hope this is a DATA packet)
                    //send ack
                }
                if (...) // TODO: Time out expired while waiting for data
                // to appear at the socket
                {
                    //TODO: Send another ACK for the last packet
                    timeoutExpiredCount++;
                }
                if (timeoutExpiredCount>= NUMBER_OF_FAILURES)
                {
                    // FATAL ERROR BAIL OUT
                }
            }while (...) // TODO: Continue while some socket was ready
            // but recvfrom somehow failed to read the data
            if (...) // TODO: We got something else but DATA
            {
                // FATAL ERROR BAIL OUT
            }
            if (...) // TODO: The incoming block number is not what we have
            // expected, i.e. this is a DATA pkt but the block number
            // in DATA was wrong (not last ACK’s block number + 1)
            {
                // FATAL ERROR BAIL OUT
            }
        } while (FALSE);
        timeoutExpiredCount = 0;
        lastWriteSize = fwrite(...); // write next bulk of data
        // TODO: send ACK packet to the client
    }while (...); // Have blocks left to be read from client (not end of transmission)

    return 0;
}