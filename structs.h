#ifndef HW7_STRUCTS_H
#define HW7_STRUCTS_H
#include <fstream>
#include <cstring>
using std::string;

#define MAX_DATA_SIZE 512
#define MAX_PACKET_SIZE 516
//**************************************************************************************
// struct for ACK messages
//**************************************************************************************
typedef struct ACKstruct{
    unsigned short opcode;
    unsigned short blockNum;
}__attribute__((packed)) ACK;

//**************************************************************************************
// struct for WRQ messages
//**************************************************************************************
typedef struct WRQstruct{
    unsigned short opcode;
    char wrqStrings[MAX_DATA_SIZE + sizeof(unsigned short)];
}__attribute__((packed)) WRQ;

//**************************************************************************************
// struct for DATA messages
//**************************************************************************************
typedef struct DATAstruct{
    unsigned short opcode;
    unsigned short blockNum;
    char data[MAX_DATA_SIZE];
}__attribute__((packed)) DATA;

#endif //HW7_STRUCTS_H
