#pragma once
#include<stdio.h>
#include "linkLayer.h"

/**
	Contains the serial port file descriptor and if user is TRANSMITTER or RECEIVER 
*/
typedef struct {
	int fileDescriptor;	/* Descritor correspondente à porta série */
	int status;			/* TRANSMITTER = 0;  RECEIVER = 1 */
} applicationLayer_t;

extern applicationLayer_t* appL;

/**
	* Initiates an application layer struct
*/
int applicationLayerInit(int status);

/**
	* Closes serial port
	@param fd Serial Port's file descriptor
*/
int closeSerialPort(int fd);

/**
	* Sets a new termios struct
*/
int saveAndSetTermios();
