#pragma once

extern struct applicationLayer_t* appL;
/**
	Contains the serial port file descriptor and if user is TRANSMITTER or RECEIVER 
*/
struct applicationLayer_t {
	int fileDescriptor;	/* Descritor correspondente à porta série */
	int status;			/* TRANSMITTER = 0;  RECEIVER = 1 */
};


/**
	Initiates an application layer struct
*/
int applicationLayerInit(int status);
