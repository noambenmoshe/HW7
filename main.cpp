#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "structs.h"

#define ACK_SIZE 4*sizeof(char)
#define HEADER_SIZE 4

using namespace std;
const unsigned short opcACK = 4;
const unsigned short opcWRQ = 2;
const unsigned short opcDATA = 3;
const int WAIT_FOR_PACKET_TIMEOUT = 3;
const int NUMBER_OF_FAILURES = 7;

void error(const string& msg, FILE* pFile){
    string msgErr = "TTFTP_ERROR: " + msg;
    perror(msgErr.c_str());
    if (pFile != NULL){
        if (fclose(pFile) != 0)
        {
            printf("TTFTP_ERROR: error closing file\n");
            printf("RECVFAIL\n");
        }
    }
    exit(1);
}

int main(int argc, char* argv[]) {
    int sock,recvMsgSize;
    struct sockaddr_in my_addr={0}, client_addr= {0};
    unsigned int client_addr_len;
    unsigned short portno;
    unsigned short  lastBlock = 0;
    size_t lastWriteSize = 0;
    int timeoutExpiredCount = 0;
    DATA dataBuffer;
    WRQ wrqBuffer;
    ACK ack;
    FILE* pFile = NULL;
    int fdNum = 0;
    char fileName[MAX_DATA_SIZE], transmissionMode[MAX_DATA_SIZE];
    if(argc <2){
        fprintf(stderr, "ERROR: no port provided\n");
        exit(1);
    }

    if((sock = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP)) < 0 ){
        error("creating socket failed",pFile);
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
    while(true){
        lastBlock = 0;
        // block until message received from client
        recvMsgSize=recvfrom(sock,&wrqBuffer,MAX_PACKET_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len);
        if(recvMsgSize < 0){
            error("recvfrom() failed", NULL);
        }
        cout  << "6 wrqBuffer opcode "<< ntohs(wrqBuffer.opcode) << endl; //DEBUG
        if(ntohs(wrqBuffer.opcode) != opcWRQ){
            error("first message is not WRQ", NULL);
        }

        strcpy(fileName,wrqBuffer.wrqStrings);
        strcpy(transmissionMode, wrqBuffer.wrqStrings+strlen(fileName)+1);

        cout << "IN:WRQ,"<< fileName << ","<< transmissionMode << endl;
        /* fdToWriteTo=open(wrqBuffer.fileName,O_CREAT);//TODO: make sure this is the right flag we need
         if(fdToWriteTo == -1){
             //const string msg ="Opened file " + wrqBuffer.fileName +" failed";
             error("Open file failed");
         }
         */
        pFile = fopen(fileName,"w"); //TODO: make sure we want to open a new file
        if(pFile == NULL){
            error("Failed to open file", pFile);
        }

        //send ack 0

        ack.opcode = htons(opcACK);
        ack.blockNum = htons(0);
        cout << "OUT:ACK,"<< ack.blockNum << endl;
        if((sendto(sock,&ack,ACK_SIZE, 0, (sockaddr*)&client_addr, sizeof(client_addr)) != ACK_SIZE)){
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
                    FD_ZERO(&rfds);
                    FD_CLR(sock,&rfds);
                    FD_SET(sock, &rfds);
                    struct timeval tv{};
                    tv.tv_sec = WAIT_FOR_PACKET_TIMEOUT;
                    fdNum = select(sock+1,&rfds,NULL,NULL,&tv);
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

                        //send ack again
                        //ack.opcode = htons(opcACK);
                        cout << "DEBUG: lastBlock is "<<lastBlock << endl; //DEBUG
                        //ack.blockNum = htons(lastBlock);
                        cout << "OUT:ACK,"<< ack.blockNum << endl;
                        if((sendto(sock,&ack,ACK_SIZE, 0, (sockaddr*)&client_addr, sizeof(client_addr)) != ACK_SIZE)){
                            error("sendto() sent a different number of bytes than expected",pFile);
                        }
                        timeoutExpiredCount++;
                    }
                    if (timeoutExpiredCount >= NUMBER_OF_FAILURES)
                    {
                        // FATAL ERROR BAIL OUT
                        cout << "FLOWERROR: number of timeouts exceeds max value. Bailing." <<endl;
                        if (fclose(pFile) != 0)
                        {
                            printf("TTFTP_ERROR: error closing file\n");
                            printf("RECVFAIL\n");
                        }
                        exit(1);
                    }
                }while ((recvMsgSize == -1 &&  fdNum > 0) || fdNum == 0); // TODO: Continue while some socket was ready but recvfrom somehow failed to read the data
                cout  << "dataBuffer opcode "<< ntohs(dataBuffer.opcode) << endl; //DEBUG
                if (ntohs(dataBuffer.opcode) != opcDATA) // TODO: We got something else but DATA
                {
                    // FATAL ERROR BAIL OUT
                    cout << "FLOWERROR: packet received isn't DATA. Bailing." <<endl;
                    cout << "RECVFAIL" << endl;
                    if (fclose(pFile) != 0)
                    {
                        printf("TTFTP_ERROR: error closing file\n");
                        printf("RECVFAIL\n");
                    }
                    exit(1);
                }
                cout << "DEBUG: lastBlock is "<<lastBlock  << "received: "<<ntohs(dataBuffer.blockNum) << endl; //DEBUG
                if (ntohs(dataBuffer.blockNum) != lastBlock+1) // TODO: The incoming block number is not what we have expected, i.e. this is a DATA pkt but
                    // the block number in DATA was wrong (not last ACK’s block number + 1)
                {
                    // FATAL ERROR BAIL OUT
                    cout << "FLOWERROR: data received is different than previous + 1 . Bailing." <<endl;
                    cout << "RECVFAIL" << endl;
                    if (fclose(pFile) != 0)
                    {
                        printf("TTFTP_ERROR: error closing file\n");
                        printf("RECVFAIL\n");
                    }
                    exit(1);

                }
            }while (false);
            cout << "IN:DATA,"<< ntohs(dataBuffer.blockNum) << ","<< recvMsgSize << endl;
            lastWriteSize = fwrite(dataBuffer.data,sizeof(char),recvMsgSize-HEADER_SIZE,pFile); // write next bulk of data
            cout << "WRITING:" << recvMsgSize-HEADER_SIZE <<endl;
            timeoutExpiredCount = 0;
            // TODO: send ACK packet to the client

            //send ack
            ack.opcode =htons(opcACK);
            ack.blockNum = dataBuffer.blockNum;
            cout << "OUT:ACK,"<< ntohs(ack.blockNum) << endl;
            if((sendto(sock,&ack,ACK_SIZE, 0, (sockaddr*)&client_addr, sizeof(client_addr)) != ACK_SIZE)){
                error("sendto() sent a different number of bytes than expected",pFile);
            }
            lastBlock++;
            //sleep(1); //DEBUG
        }while (lastWriteSize == MAX_DATA_SIZE); // Have blocks left to be read from client (not end of transmission)
        cout << "RECVOK" << endl;
        if (fclose(pFile) != 0)
        {
            printf("TTFTP_ERROR: error closing file\n");
            printf("RECVFAIL\n");
        }
    }

    return 0;
}