#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <bits/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
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


void error(char* msg, FILE* pFile){
    string msgErr = "ERROR: " + string(msg);
    perror(msgErr.c_str());
    if (pFile != NULL)
        fclose(pFile);
    exit(1);
}

int main(int argc, char* argv[]) {
    int sock,recvMsgSize;
    struct sockaddr_in my_addr={0}, client_addr= {0};
    unsigned int client_addr_len;
    unsigned short portno;
    size_t lastWriteSize = 0;
    //char buffer[MAX_PACKET_SIZE]; //TODO: change to byte
    int timeoutExpiredCount = 0;
    unsigned short  lastBlock = 0;
    //struct hostent *server;
    DATA dataBuffer;
    WRQ wrqBuffer;
    //int fdToWriteTo;
    FILE* pFile = NULL;

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
        error("on binding", NULL);
    }

    // block until message received from client
    recvMsgSize=recvfrom(sock,&wrqBuffer,MAX_PACKET_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len);
    if(recvMsgSize < 0){
        error("recvfrom() failed", NULL);
    }

    if(wrqBuffer.opcode == opcWRQ){
        error("first message is not WRQ", NULL);
    }

   /* fdToWriteTo=open(wrqBuffer.fileName,O_CREAT);//TODO: make sure this is the right flag we need
    if(fdToWriteTo == -1){
        //const string msg ="Opened file " + wrqBuffer.fileName +" failed";
        error("Open file failed");
    }
    */
    pFile = fopen(wrqBuffer.fileName,"w"); //TODO: make suere we want to open a new file
    if(pFile == NULL){
        error("Failed to open file", pFile);
    }

    //send ack 0
    ACK ack0;
    ack0.opcode = opcACK;
    ack0.blockNum = 0;
    if((sendto(sock,&ack0,ACK_SIZE, 0, (sockaddr*)&client_addr, sizeof(client_addr)) != ACK_SIZE)){
       /* if(close(fdToWriteTo) == -1)
            error("failed to close file");
        */
       fclose(pFile);
        error("sendto() sent a different number of bytes than expected",pFile);
    }

    // new loops part
    do
    {
        do
        {
            do
            { // TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears
                // for us at the socket (we are waiting for DATA)

                fd_set rfds;
                struct timeval tv;
                tv.tv_sec = WAIT_FOR_PACKET_TIMEOUT;
                int fdNum = select(sock+1,&rfds,NULL,NULL,&tv);
                if (fdNum == -1) { // syscall select failed
                    error("SELECT failed: ", pFile);
                }

                if (fdNum > 0)// TODO: if there was something at the socket and we are here not because of a timeout
                {
                // TODO: Read the DATA packet from the socket (at least we hope this is a DATA packet)
                    recvMsgSize=recvfrom(sock,&dataBuffer,MAX_PACKET_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len);
                }
                if (fdNum == 0) // TODO: Time out expired while waiting for data to appear at the socket
                {
                    //TODO: Send another ACK for the last packet
                    timeoutExpiredCount++;
                }
                if (timeoutExpiredCount >= NUMBER_OF_FAILURES)
                {
                    // FATAL ERROR BAIL OUT
                    cout << "FLOWERROR: number of timeouts exceeds max value. Bailing." <<endl;
                    fclose(pFile);
                    exit(1);
                }
            }while (recvMsgSize == -1); // TODO: Continue while some socket was ready but recvfrom somehow failed to read the data

            if (dataBuffer.opcode != opcDATA) // TODO: We got something else but DATA
            {
                // FATAL ERROR BAIL OUT
                cout << "FLOWERROR: packet received isn't DATA. Bailing." <<endl;
                fclose(pFile);
                exit(1);
            }
            if (dataBuffer.blockNum != lastBlock+1) // TODO: The incoming block number is not what we have expected, i.e. this is a DATA pkt but
                        // the block number in DATA was wrong (not last ACK’s block number + 1)
            {
                // FATAL ERROR BAIL OUT
                cout << "FLOWERROR: data received is different than previous + 1 . Bailing." <<endl;
                fclose(pFile);
                exit(1);

            }
        }while (false);
        timeoutExpiredCount = 0;
        lastWriteSize = fwrite(dataBuffer.data,sizeof(char),recvMsgSize-HEADER_SIZE,pFile); // write next bulk of data
        // TODO: send ACK packet to the client
        lastBlock++;
    }while (lastWriteSize == MAX_DATA_SIZE); // Have blocks left to be read from client (not end of transmission)


    return 0;
}