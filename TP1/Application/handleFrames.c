#include "handleFrames.h"
#include "definitions.h"
#include "linkLayer.h"
#include "applicationLayer.h"
#include "stuffing.h"
#include "alarm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

#define FER 0

int writeNonDataFrame(Frame frame, int fd)
{
    unsigned char buf[COMMAND_SIZE];

    buf[0] = FLAG;
    buf[1] = ADDR_S;

    switch (frame) {
    case SET:
        buf[2] = CTRL_SET;
        break;
    case DISC:
        buf[2] = CTRL_DISC;
        break;
    case UA:
        buf[2] = CTRL_UA;
        break;
    case RR:
        buf[2] = CTRL_RR | (linkL->sequenceNumber << 5);
        break;
    case REJ:
        buf[2] = CTRL_REJ | (linkL->sequenceNumber << 5);
        break;
    default:
        break;
    }

    buf[3] = buf[1] ^ buf[2];
    buf[4] = FLAG;

    tcflush(appL->fileDescriptor, TCOFLUSH);

    int res;

    if ((res = write(fd, &buf, 5)) != sizeof(buf)) {
        printf("Error on writting!\n");
        return -1;
    }

    return 0;
}

int writeDataFrame(unsigned char* data, unsigned int length, int fd)
{
    unsigned char* frame = malloc(1024);
    int size = length + DATA_SIZE;
    unsigned char bcc2 = 0;
    int dataInd = 0;

    frame[0] = FLAG;
    frame[1] = ADDR_S;
    frame[2] = linkL->sequenceNumber << 5;
    frame[3] = frame[1] ^ frame[2];

    memcpy(&frame[4], data, length);

    int counter = 0;

    for (dataInd = 0; dataInd < length; dataInd++) {
        bcc2 ^= data[dataInd];
        counter++;
    }

    frame[4 + length] = bcc2;
    frame[5 + length] = FLAG;

    frame = stuffing(frame, &size);

    int res = 0;

    if ((res = write(fd, frame, size)) != size) {
        printf("Error on writing data frame!\n");
        exit(1);
    }

    return 0;
}

int receiveFrame(int* fSize, int fd)
{
    unsigned char c;
    int res, ind = 0;
    ReceivingState rState = 0;

    while (alarmFlag != 1 && rState != STOP) {
        res = read(fd, &c, 1);

        if (res > 0) {

            switch (rState) {
            case START:
                if (c == FLAG) {
                    linkL->frame[ind++] = c;
                    rState++;
                }
                break;
            case FLAG_RCV:
                if (c == ADDR_S || c == ADDR_R) {
                    linkL->frame[ind++] = c;
                    rState++;
                }
                else if (c != FLAG) {
                    rState = START;
                    ind = 0;
                }
                break;
            case A_RCV:
                if (c != FLAG) {
                    linkL->frame[ind++] = c;
                    rState++;
                }
                else if (c == FLAG) {
                    rState = FLAG_RCV;
                    ind = 1;
                }
                else {
                    rState = START;
                    ind = 0;
                }
                break;
            case C_RCV:
                if (c == (linkL->frame[1] ^ linkL->frame[2])) {
                    linkL->frame[ind++] = c;
                    rState++;
                }
                else {
                    if (c == FLAG) {
                        rState = FLAG_RCV;
                        ind = 1;
                    }
                    else {
                        rState = START;
                        ind = 0;
                    }
                }
                break;
            case BCC_OK:
                if (c == FLAG) {
                    linkL->frame[ind++] = c;
                    rState++;
                }
                else
                    linkL->frame[ind++] = c;
                break;
            case STOP:
                break;
            default:
                break;
            }
        }
    }

    (*fSize) = ind;

    if (ind > 5)
        return DATA;
    else
        return NON_DATA;
}

void processDataFrame(FrameResponse* fResp, int size)
{
    unsigned char bcc2 = 0;
    int dataInd;
    (*fResp) = 0;

    int destuffedSize = size;

    unsigned char* destuffed;

    destuffed = destuffing(linkL->frame, &destuffedSize);

    strcpy((char*)linkL->frame, (char*)destuffed);

    int counter = 0;

    for (dataInd = 4; dataInd < (destuffedSize - 2); dataInd++) {
        bcc2 ^= destuffed[dataInd];
        counter++;
    }

    if (destuffed[destuffedSize - 2] != bcc2 ) {
        printf("Error on BCC2!\n");
        (*fResp) = RESP_REJ;
    }

    if (*fResp == 0)
        (*fResp) = RESP_RR;

    memcpy(linkL->dataFrame, destuffed, size);
}

int error(float errorRatio){
  if((rand() % 100 + 1) <= errorRatio*100)
    return 1;
  return 0;
}
