#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <bits/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "structs.h"

#define MAX_PACKET_SIZE 516
#define ACK_SIZE 4*sizeof(char)
#define END_WRQ_SIZE 50
#define HEADER_SIZE 4
//#define opcACK 4
//#define opcWRQ 2
//#define opcDATA 3

using namespace std;
const unsigned short opcACK = 4;
const unsigned short opcWRQ = 2;
const unsigned short opcDATA = 3;
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

bool opcodeValidate(const unsigned short expected, const char current[2]){
    unsigned short shortCurrent = *(unsigned short *)current;
    return expected == shortCurrent;
}

void extractFromData(const char srcArray[], char subArray[], int n)
{
    for (int i = 0; i < n; i++)
    {
        subArray[i] = srcArray[i];
    }
}

int main(int argc, char* argv[]) {
    int sock,recvMsgSize;
    struct sockaddr_in my_addr={0}, client_addr= {0};
    unsigned int client_addr_len;
    unsigned short portno;
    size_t lastWriteSize = 0;
    //char buffer[MAX_PACKET_SIZE]; //TODO: change to byte
    int timeoutExpiredCount = 0;
    //struct hostent *server;
    DATA dataBuffer;
    WRQ wrqBuffer;

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
    recvMsgSize=recvfrom(sock,&wrqBuffer,MAX_PACKET_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len);
    if(recvMsgSize < 0){
        error("recvfrom() failed");
    }

    if(wrqBuffer.opcode == opcWRQ){
        error("first message is not WRQ");
    }

    Encoding.ASCII.GetString(buffer,3, buffer.length-END_WRQ_SIZE).Trim('\0');


    //send ack 0
    ACK ack0;
    ack0.opcode = opcACK;
    ack0.blockNum = 0;
    if((sendto(sock,&ack0,ACK_SIZE, 0, (sockaddr*)&client_addr, sizeof(client_addr)) != ACK_SIZE)){
        error("sendto() sent a different number of bytes than expected");
    }

    //fileStream.Write(buffer,3, )

    do
    {
        do
        {
            do
            {
                // Waiting WAIT_FOR_PACKET_TIMEOUT to see if something appears for us at the socket (we are waiting for DATA)
                fd_set rfds;
                struct timeval tv;
                tv.tv_sec = WAIT_FOR_PACKET_TIMEOUT;
                int fdNum = select(sock+1,&rfds,NULL,NULL,&tv);
                if (fdNum == -1) { // syscall select failed
                    error("SELECT failed: ");
                }
                else if (fdNum > 0) // there was something at the socket. we are here not because of a timeout
                {
                    // TODO: Read the DATA packet from the socket (at least we hope this is a DATA packet)
                    recvMsgSize=recvfrom(sock,&dataBuffer,MAX_PACKET_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len);
                    if (recvMsgSize > 0){
                        // checking if the opcode is DATA
                        if (dataBuffer.opcode == opcDATA){

                        } else {
                            //todo: FATAL. not data
                        }
                    }
                    // TODO: send ack
                }
                else if (fdNum == 0) // TODO: Time out expired while waiting for data to appear at the socket
                {
                    //TODO: Send another ACK for the last packet
                    timeoutExpiredCount++;
                }
                if (timeoutExpiredCount>= NUMBER_OF_FAILURES)
                {
                    // FATAL ERROR BAIL OUT
                }
            }while (change) // TODO: Continue while some socket was ready but recvfrom somehow failed to read the data
            if (change) // TODO: We got something else but DATA
            {
                // FATAL ERROR BAIL OUT
            }
            if (change) // TODO: The incoming block number is not what we have
            //expected, i.e. this is a DATA pkt but the block number
            // in DATA was wrong (not last ACK’s block number + 1)
            {
                // FATAL ERROR BAIL OUT
            }
        } while (FALSE);
        timeoutExpiredCount = 0;
        lastWriteSize = fwrite(change); // write next bulk of data
        // TODO: send ACK packet to the client
    }while (change); // Have blocks left to be read from client (not end of transmission)


    // new loops part
    do
    {
        do
        {
            do
            { // TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears
                // for us at the socket (we are waiting for DATA)
                if (change)// TODO: if there was something at the socket and we are here not because of a timeout
                {
                // TODO: Read the DATA packet from the socket (at least we hope this is a DATA packet)
                }
                if (change) // TODO: Time out expired while waiting for data to appear at the socket
                {
                //TODO: Send another ACK for the last packet
                    timeoutExpiredCount++;
                }
                if (timeoutExpiredCount>= NUMBER_OF_FAILURES)
                {
                // FATAL ERROR BAIL OUT
                }
            }while (change) // TODO: Continue while some socket was ready but recvfrom somehow failed to read the data
            if (change) // TODO: We got something else but DATA
            {
            // FATAL ERROR BAIL OUT
            }
            if (change) // TODO: The incoming block number is not what we have expected, i.e. this is a DATA pkt but
                        // the block number in DATA was wrong (not last ACK’s block number + 1)
            {
            // FATAL ERROR BAIL OUT
            }
        }while (FALSE);
        timeoutExpiredCount = 0;
        lastWriteSize = fwrite(dataBuffer.data,sizeof(char),recvMsgSize-HEADER_SIZE,pFile); // write next bulk of data
        // TODO: send ACK packet to the client
    }while (lastWriteSize == MAX_DATA_SIZE); // Have blocks left to be read from client (not end of transmission)


    return 0;
}