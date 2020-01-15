#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "structs.h"

#define ACK_SIZE 4*sizeof(char)
#define HEADER_SIZE 4

using namespace std;
const unsigned short opcACK = 4;
const unsigned short opcWRQ = 2;
const unsigned short opcDATA = 3;
const int WAIT_FOR_PACKET_TIMEOUT = 3;
const int NUMBER_OF_FAILURES = 7;

//**************************************************************************************
// function name: error
// Description: prints the error message and errno and closes the file if it is open
// Parameters: msg - message to print, pFile - pointer to file we want to close if open
// Returns: None
//**************************************************************************************
void error(const string& msg, FILE* pFile, int sock){
    string msgErr = "TTFTP_ERROR: " + msg;
    perror(msgErr.c_str());
    if (pFile != NULL){
        if (fclose(pFile) != 0)
        {
            printf("TTFTP_ERROR: error closing file\n");
            printf("RECVFAIL\n");
        }
    }
    close(sock);
    exit(1);
}

//**************************************************************************************
// function name: setStringTerminator
// Description: sets all elemnts in a char array to '\0'
// Parameters: charToSet - array of chars to initialize to '\0'. Length - of the char array
// Returns: None
//***************************************************************************************
void setStringTerminator(char* charToSet, int length){
    if (charToSet == NULL)
        return;
    for(int i=0; i<length;i++){
        charToSet[i] = '\0';
    }
}

//**************************************************************************************
// function name: main
// Description: Makes everything happen, starts a connection with client, waits for packets
//              sends acks when relevant according to tftp protocol
// Parameters: argc - number of parameters, argv[] array of inputs
// Returns: int 0 when done
//***************************************************************************************
int main(int argc, char* argv[]) {
    int sock, recvMsgSize;
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
    bool needToContinue = false; // flag for when breaking after a fatal error

    // checking if the program was called right: only with a parameter for port number
    if(argc <2){
        fprintf(stderr, "ERROR: no port provided\n");
        exit(1);
    }

    // initializing socket
    if((sock = socket(PF_INET, SOCK_DGRAM,IPPROTO_UDP)) < 0 ){
        error("creating socket failed",pFile, sock);
    }
    memset(&my_addr,0,sizeof(my_addr));

    portno = atoi(argv[1]);
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(portno);
    if(bind(sock, (struct sockaddr*)&my_addr,sizeof(my_addr)) < 0){
        error("on binding", NULL, sock);
    }

    // main while: waiting for WRQ in each iteration
    while(true){
        lastBlock = 0;
        needToContinue = false;
        // block until message received from client
        setStringTerminator(wrqBuffer.wrqStrings, MAX_DATA_SIZE + sizeof(unsigned short));
        recvMsgSize=recvfrom(sock,&wrqBuffer,MAX_PACKET_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len);
        if(recvMsgSize < 0){
            error("recvfrom() failed", NULL, sock);
        }

        if(ntohs(wrqBuffer.opcode) != opcWRQ){
            cout << "FLOWERROR: first message is not WRQ" << endl;
            needToContinue = true;
            goto FLOWERRORContinue;
        }

        strcpy(fileName,wrqBuffer.wrqStrings);
        strcpy(transmissionMode, wrqBuffer.wrqStrings+strlen(fileName)+1);

        cout << "IN:WRQ,"<< fileName << ","<< transmissionMode << endl;

        pFile = fopen(fileName,"w"); //TODO: make sure we want to open a new file
        if(pFile == NULL){
            error("Failed to open file", pFile, sock);
        }

        //send ack 0
        ack.opcode = htons(opcACK);
        ack.blockNum = htons(0);
        cout << "OUT:ACK,"<< ack.blockNum << endl;
        if((sendto(sock,&ack,ACK_SIZE, 0, (sockaddr*)&client_addr, sizeof(client_addr)) != ACK_SIZE)){
            error("sendto() sent a different number of bytes than expected",pFile, sock);
        }

        // handelling receiving the data itself
        do
        {
            do
            {
                do
                { // TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears for us at the socket (we are waiting for DATA)

                    fd_set rfds;
                    FD_ZERO(&rfds);
                    FD_CLR(sock,&rfds);
                    FD_SET(sock, &rfds);
                    struct timeval tv{};
                    tv.tv_sec = WAIT_FOR_PACKET_TIMEOUT;
                    fdNum = select(sock+1,&rfds,NULL,NULL,&tv);
                    if (fdNum == -1) { // syscall select failed
                        error("SELECT failed: ", pFile, sock);
                    }

                    if (fdNum > 0)// TODO: if there was something at the socket and we are here not because of a timeout
                    {
                        // TODO: Read the DATA packet from the socket (at least we hope this is a DATA packet)
                        setStringTerminator(dataBuffer.data, MAX_DATA_SIZE);
                        recvMsgSize=recvfrom(sock,&dataBuffer,MAX_PACKET_SIZE,0,(struct sockaddr*)&client_addr,&client_addr_len);
                    }
                    if (fdNum == 0) // TODO: Time out expired while waiting for data to appear at the socket
                    {
                        //TODO: Send another ACK for the last packet

                        //send ack again
                        cout << "OUT:ACK,"<< ntohs(ack.blockNum) << endl;
                        if((sendto(sock,&ack,ACK_SIZE, 0, (sockaddr*)&client_addr, sizeof(client_addr)) != ACK_SIZE)){
                            error("sendto() sent a different number of bytes than expected",pFile, sock);
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
                            exit(1);
                        }
                        needToContinue = true;
                        goto FLOWERRORContinue;
                    }
                }while ((recvMsgSize == -1 &&  fdNum > 0) || fdNum == 0); // TODO: Continue while some socket was ready but recvfrom somehow failed to read the data
                //cout  << "dataBuffer opcode "<< ntohs(dataBuffer.opcode) << endl; //DEBUG
                if (ntohs(dataBuffer.opcode) != opcDATA) // TODO: We got something else but DATA
                {
                    // FATAL ERROR BAIL OUT
                    cout << "FLOWERROR: packet received isn't DATA. Bailing." <<endl;
                    if (fclose(pFile) != 0)
                    {
                        printf("TTFTP_ERROR: error closing file\n");
                        printf("RECVFAIL\n");
                        exit(1);
                    }
                    needToContinue = true;
                    goto FLOWERRORContinue;
                }
                //cout << "DEBUG: lastBlock is "<<lastBlock  << "received: "<<ntohs(dataBuffer.blockNum) << endl; //DEBUG
                if (ntohs(dataBuffer.blockNum) != lastBlock+1) // TODO: The incoming block number is not what we have expected, i.e. this is a DATA pkt but
                    // the block number in DATA was wrong (not last ACK’s block number + 1)
                {
                    // FATAL ERROR BAIL OUT
                    cout << "FLOWERROR: data received is different than previous + 1 . Bailing." <<endl;
                    if (fclose(pFile) != 0)
                    {
                        printf("TTFTP_ERROR: error closing file\n");
                        printf("RECVFAIL\n");
                        exit(1);
                    }
                    needToContinue = true;
                    goto FLOWERRORContinue;

                }
            }while (false);
            cout << "IN:DATA,"<< ntohs(dataBuffer.blockNum) << ","<< recvMsgSize << endl;

            // writing to the file
            lastWriteSize = fwrite(dataBuffer.data,sizeof(char),recvMsgSize-HEADER_SIZE,pFile); // write next bulk of data
            cout << "WRITING:" << recvMsgSize-HEADER_SIZE <<endl;

            timeoutExpiredCount = 0;
            // TODO: send ACK packet to the client

            //send ack
            ack.opcode =htons(opcACK);
            ack.blockNum = dataBuffer.blockNum;
            cout << "OUT:ACK,"<< ntohs(ack.blockNum) << endl;
            if((sendto(sock,&ack,ACK_SIZE, 0, (sockaddr*)&client_addr, sizeof(client_addr)) != ACK_SIZE)){
                error("sendto() sent a different number of bytes than expected",pFile, sock);
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
        FLOWERRORContinue: ;
        if(needToContinue){
            timeoutExpiredCount = 0;
            printf("RECVFAIL\n");
        }
    }
    return 0;
}