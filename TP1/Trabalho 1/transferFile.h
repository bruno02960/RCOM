#pragma once

#include <sys/stat.h>
#include <stdio.h>

typedef struct {
	FILE* file;
	int fileSize;
} transferFile_t;

extern transferFile_t* traF;

extern char* FILE_PATH;

int transferFileInit(int status);

void transferFileClose();