#ifndef HW7_STRUCTS_H
#define HW7_STRUCTS_H
#include <fstream>
#include <cstring>
using std::string;

#define MAX_DATA_SIZE 512 //TODO: make sure we know what is the right size

typedef struct ACKstruct{
    unsigned short opcode;
    unsigned short blockNum;
}__attribute__((packed)) ACK;

typedef struct WRQstruct{
    unsigned short opcode;
    const char *fileName; //todo: fix string
    string transmissionMode;
}__attribute__((packed)) WRQ;

typedef struct DATAstruct{
    unsigned short opcode;
    unsigned short blockNum;
    char data[MAX_DATA_SIZE];
}__attribute__((packed)) DATA;

#endif //HW7_STRUCTS_H
