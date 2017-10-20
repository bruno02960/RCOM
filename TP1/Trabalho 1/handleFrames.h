#pragma once

int writeDataFrame(unsigned char* data, unsigned int length, int fd);

unsigned char* receiveFrame(FrameType *fType, FrameResponse *fResp, int *fSize, int fd);

int writeCommandFrame(Command command, int fd);