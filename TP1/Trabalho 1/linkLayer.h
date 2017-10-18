#pragma once
#include "configs.h"

typedef struct {
	char port[20];					/*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;					/*Velocidade de transmissão*/
	unsigned int sequenceNumber;	/*Número de sequência da trama: 0, 1*/
	unsigned int timeout;			/*Valor do temporizador: 1 s*/
	unsigned int numTransmissions;	/*Número de tentativas em caso de falha*/
	unsigned char frame[PACKET_SIZE];   		/*Trama*/
} linkLayer_t;


typedef enum {
    COMMAND,
    DATA
}
FrameType;

typedef enum {
    RESP_RR,
    RESP_REJ
}
FrameResponse;

typedef enum {
    SET,
    DISC,
    UA,
    RR,
    REJ
}
Command;

extern linkLayer_t* linkL;

int linkLayerInit(char* port, int status);

int llopen(int fd);
int llwrite(unsigned char * buffer, int length, int fd);
int llread(unsigned char ** buffer, int fd);
int llclose(int fd);

int writeCommand(Command command, int fd);

int sendFile(int fd);
int receiveFile(int fd);

int writeDataFrame(unsigned char* data, unsigned int length, int fd);
unsigned char* receiveFrame(FrameType *fType, FrameResponse *fResp, int *fSize, int fd);
