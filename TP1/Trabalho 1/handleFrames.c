#include "handleFrames.h"

int writeCommandFrame(Command command, int fd) {
    unsigned char buf[COMMAND_SIZE];

	printf("Preparing to write %d\n", command);

    buf[0] = FLAG;

//  if (appL->status == TRANSMITTER)
    buf[1] = ADDR_S;
    /*  else
        buf[1] = ADDR_R;*/

    switch (command) {
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
        buf[2] = CTRL_RR | (linkL->sequenceNumber<<5);
        break;
    case REJ:
        buf[2] = CTRL_REJ | (linkL->sequenceNumber<<5);
        break;
    default:
        break;
    }
    buf[3] = buf[1] ^ buf[2];

    buf[4] = FLAG;

    tcflush(appL->fileDescriptor, TCOFLUSH);

	int res;

	printf("appL->fileDescriptor=%d\n", fd);
    if ((res=write(fd, &buf, 5)) != sizeof(buf)) {
        printf("Error on writting!\n");
    printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
        return -1;
    }
	else {
	printf("Command successfully written!\n");
}

    /* What's the adress byte? */
    /* What's RR & REJ BCC ? */
    return 0;

}

int writeDataFrame(unsigned char* data, unsigned int length, int fd) {
    unsigned char *frame = malloc(1024);
    int size = length + DATA_SIZE;
    unsigned char bcc2 = 0;
    int dataInd = 0;


    frame[0] = FLAG;
    frame[1] = ADDR_S;
    frame[2] = linkL->sequenceNumber << 5;
    frame[3] = frame[1] ^ frame[2];

    memcpy(&frame[4], data, length);

int counter = 0;

    for(dataInd = 0; dataInd < length; dataInd++) {
        bcc2 ^= data[dataInd];
        counter++;
    }

    frame[4 + length] = bcc2;
    frame[5 + length] = FLAG;

    frame = stuffing(frame, &size);

  int res = 0;

    if((res=write(fd, frame, size)) != size) {
      printf("Error on writing data frame!\n");
      exit(1);
    }
    else {
  printf("%d bytes written!\n", res);
  }


    return 0;
}

unsigned char* receiveFrame(FrameType *fType, FrameResponse *fResp, int *fSize, int fd) {
    unsigned char c;
    int res, ind=0;
    ReceivingState rState=0;

    while (alarmFlag != 1 && rState!=STOP) {
        res = read(fd, &c, 1);

        if (res > 0) {

            switch(rState) {
            case START:
                if (c == FLAG) {
                    linkL->frame[ind++]=c;
                    rState++;
                }
                break;
            case FLAG_RCV:
                if (c == ADDR_S || c == ADDR_R) {
                    linkL->frame[ind++]=c;
                    rState++;
                }
                else if (c!=FLAG) {
                    rState = START;
                    ind = 0;
                }
                break;
            case A_RCV:
                if (c != FLAG) {
                    linkL->frame[ind++]=c;
                    rState++;
                }
                else if (c==FLAG) {
                    rState = FLAG_RCV;
                    ind = 1;
                }
                else {
                    rState=START;
                    ind = 0;
                }
                break;
            case C_RCV:
                if (c == (linkL->frame[1]^linkL->frame[2])) {
                    linkL->frame[ind++]=c;
                    rState++;
                }
                else {
                    if (c==FLAG) {
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
                    linkL->frame[ind++]=c;
                    rState++;

                    if(ind > 5)
                        (*fType) = DATA;
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

    if((*fType) == DATA) {
      unsigned char bcc2 = 0;
      int dataInd;

      int size = ind;

    unsigned char * destuffed;

    destuffed=destuffing(linkL->frame, &size);

      strcpy((char*)linkL->frame, (char*)destuffed);

int counter = 0;

      for(dataInd = 4; dataInd < (size-2); dataInd++) {
        bcc2 ^= destuffed[dataInd];
counter++;
      }

      if(destuffed[size - 2] != bcc2) {
        printf("Error on BCC2!\n");
        (*fResp) = RESP_REJ;
      }

      if(*fResp == 0)
        (*fResp) = RESP_RR;

    (*fSize) = ind;
    memcpy(linkL->dataFrame, destuffed, size);
    }


    return NULL;
}
