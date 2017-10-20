#pragma once

int writeDataFrame(unsigned char* data, unsigned int length, int fd);

unsigned char* receiveFrame(FrameResponse *fResp, int *fSize, int fd);

int writeCommandFrame(Command command, int fd);