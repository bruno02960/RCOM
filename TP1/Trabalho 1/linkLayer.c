#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include "alarm.h"
#include "definitions.h"
#include "configs.h"
#include "stuffing.h"
#include "handlePackets.h"
#include "linkLayer.h"
#include "applicationLayer.h"
#include "transferFile.h"
#include "handleFrames.h"

linkLayer_t * linkL;

int linkLayerInit(char * port, int status) {
    linkL = (linkLayer_t * ) malloc(sizeof(linkLayer_t));

    strcpy(linkL->port, port);

    linkL->timeout = TIMEOUT;
    linkL->baudRate = BAUDRATE;                   /*Velocidade de transmissão*/
    linkL->sequenceNumber = 0;                    /*Número de sequência da trama: 0, 1*/
    linkL->numTransmissions = NUM_TRANSMISSIONS;  /*Número de tentativas em caso de falha*/

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

    closeSerialPort(fd);

    return 0;
}


int llopen(int fd) {
    int alarmCounter = 0;

    switch (appL->status) {
      case TRANSMITTER:
          while (alarmCounter < NO_TRIES) {
              if (alarmCounter == 0 || alarmFlag == 1) {
                  setAlarm(fd);
                  writeCommandFrame(SET, fd);
                  alarmFlag = 0;
                  alarmCounter++;
              }

              receiveFrame(NULL, NULL, fd);

              if (linkL->frame[2] == CTRL_UA) {
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
          receiveFrame(NULL, NULL, fd);

          if (linkL->frame[2] == CTRL_SET) {
              writeCommandFrame(UA, fd);
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

  while (alarmCounter < NO_TRIES){
    if (alarmCounter == 0 || alarmFlag == 1) {
      setAlarm(fd);
      writeDataFrame(buffer, length, fd);
      alarmFlag = 0;
      alarmCounter++;
    }

    receiveFrame(NULL, NULL, fd);

    if (linkL->frame[2] == (CTRL_RR | (linkL->sequenceNumber<<5))) {
       stopAlarm(fd);
	linkL->sequenceNumber=!linkL->sequenceNumber;
  break;
      }
   else if (linkL->frame[2] == (CTRL_REJ | (linkL->sequenceNumber<<5))) {
      }
    }

    if (alarmCounter < NO_TRIES) {
	return 0;
	}
    else {
      printf("Couldn't write!\n");
return 1;
	}

  return 1;
}

int llread(unsigned char ** buffer, int fd) {
  int read = 0, fSize, dataSize, answered = 0;
  FrameResponse fResp = 0;

  while (answered == 0)
  {
    receiveFrame(&fResp, &fSize, fd);

		printf("fResp=%d - RESP_RR=%d\n",fResp,RESP_RR);
		printf("lL->sN=%02x - lL->frame=%02x\n",linkL->sequenceNumber,((linkL->frame[2]>>5) & BIT(0)));
        if(fResp == RESP_RR && ((linkL->frame[2]>>5) & BIT(0)) == linkL->sequenceNumber)
		{
            writeCommandFrame(RR, fd);
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
            writeCommandFrame(REJ, fd);
          	}
  }

  return read;
}

int llclose(int fd) {
  int alarmCounter = 0;
  int discReceived = 0;
  
  switch (appL -> status) {
  case TRANSMITTER:
    while (alarmCounter < NO_TRIES) {

      if (alarmCounter == 0 || alarmFlag == 1) {
        setAlarm(fd);
        writeCommandFrame(DISC, fd);
        alarmFlag = 0;
        alarmCounter++;
      }

      receiveFrame(NULL, NULL, fd);

      if (linkL->frame[2] == CTRL_DISC) {
        writeCommandFrame(UA, fd);
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
    while (alarmCounter < NO_TRIES) {
    
	 receiveFrame(NULL, NULL, fd);

      if (linkL->frame[2] == CTRL_DISC) {
        discReceived = 1;
        }	
		
      if (discReceived && (alarmCounter == 0 || alarmFlag == 1)) {
        setAlarm(fd);
        writeCommandFrame(DISC, fd);
        alarmFlag = 0;
        alarmCounter++;
      } 

      receiveFrame(NULL, NULL, fd);

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
	else {
	printf("End control packet successfully written!\n");
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