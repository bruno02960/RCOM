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

typedef struct {
	FILE* file;
	int fileSize;
} transferFile_t;

extern transferFile_t* traF;

extern char* FILE_PATH;

/**
	Initiates an application layer struct
*/
int applicationLayerInit(int status);

int closeSerialPort();
int saveAndSetTermios();
int transferFileInit();
void transferFileClose();
int writeDataFrame(unsigned char* data, unsigned int length);
char* receiveFrame(FrameType *fType, FrameResponse *fResp, int *fSize);
