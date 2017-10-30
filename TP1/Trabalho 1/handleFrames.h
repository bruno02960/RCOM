#pragma once

/* Reception state machine */
typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} ReceivingState;

/* Frame responses */
typedef enum {
    RESP_RR,
    RESP_REJ
} FrameResponse;

/* Non data frames */
typedef enum {
    SET,
    DISC,
    UA,
    RR,
    REJ
} Frame;

/* Frame type */
typedef enum {
    DATA,
    NON_DATA
} FrameType;

/**
* Writes a data frame to the serial port
* @param data Data to be written
* @param length Length of data
* @param fd Serial port file descriptor
*/
int writeDataFrame(unsigned char* data, unsigned int length, int fd);

/**
* Receives a frame from the serial port
* @param fSize Returns the frame size
* @param fd Serial port file descriptor
*/
int receiveFrame(int* fSize, int fd);

/**
* Writes a non data frame to the serial port
* @param frame Frame type to be written
* @param fd Serial port file descriptor
*/
int writeNonDataFrame(Frame frame, int fd);

/**
* Processes a data frame received from the serial port
* @param fResp Returns the frame response
* @param size Frame size
*/
void processDataFrame(FrameResponse* fResp, int size);
