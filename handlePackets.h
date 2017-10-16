#pragma once

#include "linkLayer.h"
#include "transferFile.h"
#include "definitions.h"


int writeControlPacket(int controlField);

int writeDataPacket(char* buffer, int noBytes, int seqNo);

int receiveControlPacket(int controlField, int* noBytes, char** filePath);

int receiveDataPacket(unsigned char ** buffer, int sequenceNumber);