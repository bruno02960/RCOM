#pragma once


int writeControlPacket(int controlField, int fd);

int writeDataPacket(unsigned char* buffer, int noBytes, int seqNo, int fd);

int receiveControlPacket(int controlField, int* noBytes, unsigned char** filePath, int fd);

int receiveDataPacket(unsigned char ** buffer, int sequenceNumber, int fd);
