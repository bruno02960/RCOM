#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "alarm.h"
#include "definitions.h"
#include "configs.h"
#include "stuffing.h"
#include "handlePackets.h"
#include "linkLayer.h"
#include "applicationLayer.h"
#include "transferFile.h"
#include <errno.h>

#define NO_TRIES 3

linkLayer_t * linkL;

typedef enum {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
}
ReceivingState;

int linkLayerInit(char * port, int status) {
    linkL = (linkLayer_t * ) malloc(sizeof(linkLayer_t));

    strcpy(linkL->port, port);

    linkL->timeout = TIMEOUT;
    linkL->baudRate = BAUDRATE;                   /*Velocidade de transmissão*/
    linkL->sequenceNumber = 0;                    /*Número de sequência da trama: 0, 1*/
    linkL->numTransmissions = NUM_TRANSMISSIONS;  /*Número de tentativas em caso de falha*/
    //linkL->frame[MAX_SIZE];                     /*Trama */

    applicationLayerInit(status);
	int fd = appL->fileDescriptor;

    saveAndSetTermios();

    if(llopen(fd)) {
     printf("Error in llopen!\n");
      exit(1);
    }

   switch(appL->status) {
      case TRANSMITTER:
        sendFile(fd);
        break;
      case RECEIVER:
        receiveFile(fd);
        break;
      default:
        exit(1);
    }

   if(llclose(fd)) {
      printf("Error in llclose!\n");
      exit(1);
    }

	printf("File descriptor=%d\n", fd);
    closeSerialPort(fd);

    return 0;
}


int llopen(int fd) {
    int alarmCounter = 0;
    FrameType frType;

    switch (appL->status) {
      case TRANSMITTER:
          while (alarmCounter < NO_TRIES) {
              if (alarmCounter == 0 || alarmFlag == 1) {
                  setAlarm(fd);
                  writeCommand(SET, fd);
                  alarmFlag = 0;
                  alarmCounter++;
              }

              receiveFrame(&frType, NULL, NULL, fd);

              if (linkL->frame[2] == CTRL_UA) {
                  printf("UA received!\n");
                  break;
              }
          }
          stopAlarm(fd);
          if (alarmCounter < NO_TRIES)
              printf("Connection successfully done!\n");
          else {
              printf("Connection couldn't be done!\n");
              return 1;
            }
          break;

      case RECEIVER:
          receiveFrame(&frType, NULL, NULL, fd);

          if (linkL->frame[2] == CTRL_SET) {
              writeCommand(UA, fd);
              printf("Connection successfully done!\n");
          }
          else {
              printf("Connection couldn't be done!\n");
              return 1;
          }
          break;
    }

    return 0;
}

int llwrite(unsigned char * buffer, int length, int fd) {
  int alarmCounter = 0;
  FrameType frType;

  while (alarmCounter < NO_TRIES){
    if (alarmCounter == 0 || alarmFlag == 1) {
      setAlarm(fd);
      writeDataFrame(buffer, length, fd);
      alarmFlag = 0;
      alarmCounter++;
    }

    receiveFrame(&frType, NULL, NULL, fd);

    printf("received[2]=%02x\n",linkL->frame[2]);
    printf("local[2]=%02x\n",((linkL->sequenceNumber<<5) | CTRL_RR));

    if (linkL->frame[2] == (CTRL_RR | (linkL->sequenceNumber<<5))) {
   //printf("RR received\n");
       stopAlarm(fd);
	linkL->sequenceNumber=!linkL->sequenceNumber;
  break;
      }
   else if (linkL->frame[2] == (CTRL_REJ | (linkL->sequenceNumber<<5))) {
      }
    }

    if (alarmCounter < NO_TRIES) {
      printf("Written!\n");
return 0;
	}
    else {
      printf("Couldn't write!\n");
return 1;
	}

  return 1;
}

int llread(unsigned char ** buffer, int fd) {
  int read = 0, disconnect = 0, fSize, dataSize, answered = 0;
  FrameType frType = 0;
  FrameResponse fResp = 0;

  while (disconnect == 0 && answered == 0)
  {
	printf("Not disconnected nor answered!\n");
    receiveFrame(&frType, &fResp, &fSize, fd);

	printf("frType=%d\n", frType);
	printf("fResp=%d\n", fResp);

    switch(frType)
	{
      case COMMAND:
        if(linkL->frame[2] == CTRL_DISC)
          disconnect = 1;
        break;
      case DATA:
		printf("fResp=%d - RESP_RR=%d\n",fResp,RESP_RR);
		printf("lL->sN=%02x - lL->frame=%02x\n",linkL->sequenceNumber,((linkL->frame[2]>>5) & BIT(0)));
        if(fResp == RESP_RR && ((linkL->frame[2]>>5) & BIT(0)) == linkL->sequenceNumber)
		{
            writeCommand(RR, fd);
          linkL->sequenceNumber = !linkL->sequenceNumber;
          dataSize = fSize - DATA_SIZE;
          *buffer = malloc(dataSize);
          memcpy(*buffer, &linkL->dataFrame[4], dataSize);
          answered = 1;
        }
        else
          if (fResp == RESP_REJ)
			{
            linkL->sequenceNumber = ((linkL->frame[2]>>5) & BIT(0));
            writeCommand(REJ, fd);
          	}
		break;
      default:
        return 1;
    }
	printf("disconnect=%d\n", disconnect);
  }

  return read;
}

int llclose(int fd) {
  int alarmCounter = 0;
  FrameType frType=0;
  int discReceived = 0;
  
  switch (appL -> status) {
  case TRANSMITTER:
    while (alarmCounter < NO_TRIES) {

      if (alarmCounter == 0 || alarmFlag == 1) {
        setAlarm(fd);
        writeCommand(DISC, fd);
        alarmFlag = 0;
        alarmCounter++;
      }

      receiveFrame(&frType, NULL, NULL, fd);

      if (linkL->frame[2] == CTRL_DISC) {
        writeCommand(UA, fd);
        break;
        }
    }
    
    stopAlarm(fd);
    
      if (alarmCounter < NO_TRIES) {
        printf("Disconnection successfully done!\n");
        return 0;
       }
      else {
        printf("Disconnection couldn't be done!\n");
        return 1;
      }
    break;
    
  case RECEIVER:
		printf("HERE1!\n");
		printf("alarmCounter=%d\n", alarmCounter);
    while (alarmCounter < NO_TRIES) {
    
	 receiveFrame(&frType, NULL, NULL, fd);
	  printf("HERE2-c!\n");

      if (linkL->frame[2] == CTRL_DISC) {
        printf("DISC RECEIVED!\n");
        discReceived = 1;
        }	
		
      if (discReceived && (alarmCounter == 0 || alarmFlag == 1)) {
        setAlarm(fd);
        writeCommand(DISC, fd);
        alarmFlag = 0;
        alarmCounter++;
      } 

printf("HERE2-a!\n");
      receiveFrame(&frType, NULL, NULL, fd);
printf("HERE2-b!\n");

      if (linkL->frame[2] == CTRL_UA) {
        printf("Disconnection successfully done 1!\n");
        break;
        }
    }

    stopAlarm(fd);
    
    printf("alarmCounter=%d\n", alarmCounter);

    if (alarmCounter < NO_TRIES) {
      printf("Disconnection successfully done 2!\n");
      return 0;
      }
    else {
      printf("Disconnection couldn't be done!\n");
      return 1;
    }
    
  }

  return 0;
}



int writeCommand(Command command, int fd) {
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

int sendFile(int fd) {
  unsigned char * packetBuffer = malloc(PACKET_SIZE * sizeof(unsigned char));
  int read, seqNo=0;

  if(writeControlPacket(CTRL_START, fd)) {
    printf("Error on writing control packet in sendFile!\n");
    exit(1);
  }
  else {
    printf("Start control packet successfully written!\n");
  }

  while((read=fread(packetBuffer, sizeof(unsigned char), PACKET_SIZE, traF->file)) > 0 ){

    printf("seqNo sent=%d\n", (seqNo % 255));
    if(writeDataPacket(packetBuffer, read, (seqNo % 255), fd)) {    //seqNo is module 255
      printf("Error on writing data packet in sendFile!\n");
      exit(1);
    }

    seqNo++;
  }

  transferFileClose();

  if(writeControlPacket(CTRL_END, fd)) {
    printf("Error on writing control packet in sendFile!\n");
    exit(1);
  }

  return 0;
}


int receiveFile(int fd) {
  int fileSize;
  unsigned char *filePath = (unsigned char *) malloc(100);

  if(receiveControlPacket(START_BYTE, &fileSize, &filePath, fd)) {
    printf("Error on receiving control packet in receiveFile!\n");
    exit(1);
  }
  else {
	printf("Control packet received!\n");
  }

  int read, noBytes = 0, seqNo = 0, written = 0;
  unsigned char * buffer = malloc(PACKET_SIZE * sizeof(char));

	printf("Preparing to receive data packet...\n");
	printf("File size=%d\n",fileSize);

  while(noBytes < fileSize) {
  	printf("seqNo expected=%d\n", (seqNo % 255));
    if((read = receiveDataPacket(&buffer, (seqNo % 255), fd))<0)
      exit(1);

    noBytes += read;
	printf("noBytes=%d\n",noBytes);

    if((written=fwrite(buffer, sizeof(char), read, traF->file))>0) {
		printf("%d bytes written to file!\n", written);
	}
	else {
		return 1;
	}

    seqNo++;
  }

	printf("Before closing file!\n");
  transferFileClose();

  if(receiveControlPacket(END_BYTE, &fileSize, &filePath, fd)) {
    printf("Error on receiving control packet in receiveFile!\n");
    exit(1);
  }
	else {
	printf("Control packet successfully received!\n");
	}

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
					printf("HERE!\n");
                    linkL->frame[ind++]=c;
					printf("HERE!\n");
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
	printf("rState=%d!\n", rState);

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

	printf("Before evaluation!\n");

    if((*fType) == DATA) {
      unsigned char bcc2 = 0;
      int dataInd;

      int size = ind;

	/*int i=0;

	for(i=4; i<ind; i++) {
		printf("linkL->frame[%i]=%02x\n", i, linkL->frame[i]);
	}*/

	unsigned char * destuffed;

	destuffed=destuffing(linkL->frame, &size);

	//size = ind - DATA_SIZE;

      strcpy((char*)linkL->frame, (char*)destuffed);

      /* Is there necessity to check BCC1? */

int counter = 0;

      for(dataInd = 4; dataInd < (size-2); dataInd++) {
        bcc2 ^= destuffed[dataInd];
counter++;
      }

	printf("bcc2 used %d bytes\n",counter);

	  printf("last data %02x\n", linkL->frame[dataInd-1]);
	  printf("bcc2=%02x\n",bcc2);

	printf("4 + size = %d\n",(4+size));

      if(destuffed[size - 2] != bcc2) {
        printf("Error on BCC2!\n");
        (*fResp) = RESP_REJ;
      }

      if(*fResp == 0)
        (*fResp) = RESP_RR;

    (*fSize) = ind;
    memcpy(linkL->dataFrame, destuffed, size);
    }
	printf("After evaluation!\n");


	return NULL;
}
