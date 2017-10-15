#pragma once

#include "linkLayer.h"

void writeControlPacket(int controlField);

void writeDataPacket(char* buffer, int noBytes, int seqNo);

int receiveControlPacket(int controlField, int* noBytes, char** filePath);

int receiveDataPacket(unsigned char ** buffer, int sequenceNumber);