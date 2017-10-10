#include "linkLayer.h"

linkLayer_t linkL;

int init(){
	linkL = (linkLayer_t*) malloc(sizeof(linkLayer_t));

	strcpy(linkL->port, /* String porta série */);
	linkL->baudRate = /*Velocidade de transmissão*/
	linkL->sequenceNumber = 0;	/*Número de sequência da trama: 0, 1*/
	linkL->timeout = /*Valor do temporizador: 1 s*/
	

	//HOW? 
	//linkL->numTransmissions;	/*Número de tentativas em caso de falha*/
	//linkL->frame[MAX_SIZE];			/*Trama */
}

