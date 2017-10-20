#pragma once

#include <sys/stat.h>
#include <stdio.h>

typedef struct {
	FILE* file;
	int fileSize;
} transferFile_t;

extern transferFile_t* traF;

extern char* FILE_PATH;

/**
	Opens file to be sent
	@param status Transmitter or Receiver
*/
int transferFileInit(int status);

/**
	Closes file
*/
void transferFileClose();

/**
	Sends file
	@param fd Serial Port's file descriptor
*/
int sendFile(int fd);

/**
	Receives file
	@param fd Serial Port's file descriptor
*/
int receiveFile(int fd);