#include "linkLayer.h"

linkLayer_t linkL;

int linkLayerInit(char[] port){

	printf("%s", port);
	linkL = (linkLayer_t*) malloc(sizeof(linkLayer_t));

	strcpy(linkL->port, port);
	//linkL->baudRate = /*Velocidade de transmissão*/
//	linkL->sequenceNumber = 0;	/*Número de sequência da trama: 0, 1*/
//	linkL->timeout = /*Valor do temporizador: 1 s*/
//	linkL->numTransmissions = /*Número de tentativas em caso de falha*/
	

	// HOW???
	//linkL->frame[MAX_SIZE];			/*Trama */

	return 0;
}

