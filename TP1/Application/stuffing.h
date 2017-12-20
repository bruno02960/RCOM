#pragma once

/**
	Handles stuffing technique
	@param buf Data to be stuffed
	@param size Data buffer size
*/
unsigned char* stuffing(unsigned char* buf, int* size);

/**
	Handles destuffing technique
	@param buf Data to be stuffed
	@param size Data buffer size
*/
unsigned char* destuffing(unsigned char* buf, int* size);
