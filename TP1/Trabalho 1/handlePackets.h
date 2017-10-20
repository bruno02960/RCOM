#pragma once

/**
	* Writes Control Packet
	@param controlField Start Byte or End Byte
	@param fd Serial port's file descriptor
*/
int writeControlPacket(int controlField, int fd);

/**
	* Writes Data Packet
	@param buffer Data to be sent
	@param noBytes Number of bytes to be sent
	@param seqNo Sequence number of the Data
	@param fd Serial port's file descriptor
*/
int writeDataPacket(unsigned char* buffer, int noBytes, int seqNo, int fd);

/**
	* Receives Control Packet
	@param controlField Start Byte or End Byte
	@param noBytes Number of bytes to be received
	@param filePath Path of file to be received
	@param fd Serial port's file descriptor
*/
int receiveControlPacket(int controlField, int* noBytes, unsigned char** filePath, int fd);

/**
	* Receives Data Packet
	@param buffer Data to be sent
	@param seqNo Sequence number of the Data
	@param fd Serial port's file descriptor
*/
int receiveDataPacket(unsigned char ** buffer, int sequenceNumber, int fd);
