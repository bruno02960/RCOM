#pragma once

typedef struct {
	char port[20];					/*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;					/*Velocidade de transmissão*/
	unsigned int sequenceNumber;	/*Número de sequência da trama: 0, 1*/
	unsigned int timeout;			/*Valor do temporizador: 1 s*/
	unsigned int numTransmissions;	/*Número de tentativas em caso de falha*/
	char frame[5];			/*Trama*/
} linkLayer_t;

extern linkLayer_t* linkL;

int linkLayerInit(char* port);

int llopen();
