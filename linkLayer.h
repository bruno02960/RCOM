#pragma once

typedef struct {
	char port[20];					/*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;					/*Velocidade de transmissão*/
	unsigned int sequenceNumber;	/*Número de sequência da trama: 0, 1*/
	unsigned int timeout;			/*Valor do temporizador: 1 s*/
	unsigned int numTransmissions;	/*Número de tentativas em caso de falha*/
	char frame[5];			/*Trama*/
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

int llopen();
int llwrite(unsigned char * buffer, int length);
int llread(unsigned char ** buffer);
int llclose();

int writeCommand(Command command);

int sendFile();
int receiveFile();

int writeDataFrame(unsigned char* data, unsigned int length);
char* receiveFrame(FrameType *fType, FrameResponse *fResp, int *fSize);
