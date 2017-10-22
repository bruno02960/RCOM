#pragma once

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
}
ReceivingState;

typedef enum {
    RESP_RR,
    RESP_REJ
}
FrameResponse;

typedef enum {
    SET,
    DISC,
    UA,
    RR,
    REJ
}
Frame;

int writeDataFrame(unsigned char* data, unsigned int length, int fd);

unsigned char* receiveFrame(FrameResponse *fResp, int *fSize, int fd);

int writeNonDataFrame(Frame frame, int fd);
