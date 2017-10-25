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
                  writeNonDataFrame(SET, fd);
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
              printf("Connection done!\n");
          else {
              printf("Connection couldn't be done!\n");
              return 1;
            }
          break;

      case RECEIVER:
          receiveFrame(NULL, NULL, fd);

          if (linkL->frame[2] == CTRL_SET) {
              writeNonDataFrame(UA, fd);
              printf("Connection done!\n");
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
      stopAlarm(fd);
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

		if(fResp == RESP_RR && ((linkL->frame[2]>>5) & BIT(0)) == linkL->sequenceNumber)
		{
            writeNonDataFrame(RR, fd);
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
            writeNonDataFrame(REJ, fd);
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
        writeNonDataFrame(DISC, fd);
        alarmFlag = 0;
        alarmCounter++;
      }

      receiveFrame(NULL, NULL, fd);

      if (linkL->frame[2] == CTRL_DISC) {
        writeNonDataFrame(UA, fd);
        break;
        }
    }

    stopAlarm(fd);

      if (alarmCounter < NO_TRIES) {
        printf("Disconnection done!\n");
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
        writeNonDataFrame(DISC, fd);
        alarmFlag = 0;
        alarmCounter++;
      }

      sleep(1);
      receiveFrame(NULL, NULL, fd);

      if (linkL->frame[2] == CTRL_UA) {
        break;
        }
    }

    stopAlarm(fd);

    if (alarmCounter < NO_TRIES) {
      printf("Disconnection done!\n");
      return 0;
      }
    else {
      printf("Disconnection couldn't be done!\n");
      return 1;
    }

  }

  return 0;
}
