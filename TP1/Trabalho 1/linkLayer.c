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

    saveAndSetTermios();

    if(llopen()) {
     printf("Error in llopen!\n");
      exit(1);
    }

   switch(appL->status) {
      case TRANSMITTER:
        sendFile();
        break;
      case RECEIVER:
        receiveFile();
        break;
      default:
        exit(1); 
    }
   /*if(llclose()) {
      printf("Error in llclose!\n");
      exit(1);
    }*/

    closeSerialPort();

    return 0;
}


int llopen() {
    int alarmCounter = 0;
    FrameType frType;

    switch (appL->status) {
      case TRANSMITTER:
          while (alarmCounter < NO_TRIES) {
              if (alarmCounter == 0 || alarmFlag == 1) {
                  setAlarm();
                  writeCommand(SET);
                  alarmFlag = 0;
                  alarmCounter++;
              }

              receiveFrame(&frType, NULL, NULL);

              if (linkL->frame[2] == CTRL_UA) {
                  printf("UA received!\n");
                  break;
              }
          }
          stopAlarm();
          if (alarmCounter < NO_TRIES) 
              printf("Connection successfully done!\n");
          else {
              printf("Connection couldn't be done!\n");
              return 1;
            }
          break;

      case RECEIVER:
          receiveFrame(&frType, NULL, NULL);

          if (linkL->frame[2] == CTRL_SET) {
              writeCommand(UA);
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

int llwrite(unsigned char * buffer, int length) {
  int alarmCounter = 0;
  int written = 0;
  FrameType frType;

  printf("LLWRITE IN!\n");

  while (alarmCounter < NO_TRIES){
    if (alarmCounter == 0 || alarmFlag == 1) {
      setAlarm();
      writeDataFrame(buffer, length);
      alarmFlag = 0;
      alarmCounter++;
    }

    receiveFrame(&frType, NULL, NULL);

  printf("linkL->frame[2]=%02x\n",linkL->frame[2]);
  printf("((linkL->sequenceNumber<<5) | CTRL_RR)=%02x\n",((linkL->sequenceNumber<<5) | CTRL_RR));

   if (linkL->frame[2] == (CTRL_RR | (linkL->sequenceNumber<<5))) {
  printf("RR received\n");
       stopAlarm();
  break;
      } 
   else if (linkL->frame[2] == (CTRL_REJ | (linkL->sequenceNumber<<5))) {
  printf("REJ received\n");
       stopAlarm();
  break;
      }  
    } 

    if (alarmCounter < NO_TRIES)
      printf("Written!\n");
    else
      printf("Couldn't write!\n");

  return written;
}

int llread(unsigned char ** buffer) {
  int read = 0, disconnect = 0, fSize, dataSize, answered = 0;
  FrameType frType = 0;
  FrameResponse fResp = 0;

  while (disconnect == 0 && answered == 0) 
  {
	printf("Not disconnected nor answered!\n");
    receiveFrame(&frType, &fResp, &fSize);

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
            writeCommand(RR);
          linkL->sequenceNumber = !linkL->sequenceNumber;
          dataSize = fSize - DATA_SIZE;
          *buffer = malloc(dataSize);
          memcpy(*buffer, &linkL->frame[4], dataSize);
          answered = 1;
        } 
        else
          if (fResp == RESP_REJ) 
			{
            linkL->sequenceNumber = ((linkL->frame[2]>>5) & BIT(0));
            writeCommand(REJ);
          	}
		break;
      default:
        return 1;  
    }
	printf("disconnect=%d\n", disconnect);
  }

  return read;
}

int llclose() {
  int alarmCounter = 0;
  char * response;

  switch (appL -> status) {
  case TRANSMITTER:
    while (alarmCounter < NO_TRIES) {

      if (alarmCounter == 0 || alarmFlag == 1) {
        setAlarm();
        writeCommand(SET);
        alarmFlag = 0;
        alarmCounter++;
      }

      response = receiveFrame(NULL, NULL, NULL);

      if (response[2] == CTRL_DISC)
        writeCommand(UA);
    }
      if (alarmCounter < NO_TRIES)
        printf("Disconnection successfully done!\n");
      else {
        printf("Disconnection couldn't be done!\n");
        return 1;
      }
    stopAlarm();
    break;
  case RECEIVER:
    while (alarmCounter < NO_TRIES) {

      if (alarmCounter == 0 || alarmFlag == 1) {
        setAlarm();
        writeCommand(DISC);
        alarmFlag = 0;
        alarmCounter++;
      }

      receiveFrame(NULL, NULL, NULL);

      if (linkL->frame[2] == CTRL_UA)
        printf("Disconnection successfully done!\n");
    }

    stopAlarm();
    
    if (alarmCounter < NO_TRIES)
      printf("Disconnection successfully done!\n");
    else {
      printf("Disconnection couldn't be done!\n");
      return 1;
    }

    break;
  }

  return 0;
}



int writeCommand(Command command) {
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

	printf("Preparing to write command control %02x\n", buf[2]);
	printf("SIZEOF(BUF)=%d\n",sizeof(buf));

    if (write(appL->fileDescriptor, &buf, 5) != sizeof(buf)) {
        printf("Error on writting!\n");
        return -1;
    }
	else {
	printf("Command successfully written!\n");
}

    /* What's the adress byte? */
    /* What's RR & REJ BCC ? */
    return 0;

}

int sendFile() {
  char * packetBuffer = malloc(PACKET_SIZE * sizeof(char));
  int read, seqNo=0;
  
  if(writeControlPacket(CTRL_START)) {
    printf("Error on writing control packet in sendFile!\n");
    exit(1);
  }
  else {
    printf("Start control packet successfully written!\n");
  }

  /*while(*/(read=fread(packetBuffer, sizeof(char), PACKET_SIZE, traF->file)) > 0;// ){
  
  	printf("seqNo sent=%d\n", (seqNo % 255));
    if(writeDataPacket(packetBuffer, read, (seqNo % 255))) {    //seqNo is module 255
      printf("Error on writing data packet in sendFile!\n");
      exit(1);
    }

    seqNo++;
  //}

  /*transferFileClose();

  if(writeControlPacket(CTRL_END)) {
    printf("Error on writing control packet in sendFile!\n");
    exit(1);
  }*/

  return 0;
}

int receiveFile() {
  int fileSize;
  char *filePath = (char *) malloc(100);

  if(receiveControlPacket(START_BYTE, &fileSize, &filePath)) {
    printf("Error on receiving control packet in receiveFile!\n");
    exit(1);
  }
  else {
	printf("Control packet received!\n");
  }

  int read, noBytes = 0, seqNo = 0;
  unsigned char * buffer = malloc(PACKET_SIZE * sizeof(char));

	printf("Preparing to receive data packet...\n");

  //while(noBytes < fileSize) {
  	printf("seqNo expected=%d\n", (seqNo % 255));
    if((read = receiveDataPacket(&buffer, (seqNo % 255)))<0)
      exit(1);

    noBytes += read;

    fwrite(buffer, sizeof(char), read, traF->file);
    seqNo++;
  //}

  /*transferFileClose();

  if(receiveControlPacket(END_BYTE, &fileSize, &FILE_PATH)) {
    printf("Error on receiving control packet in receiveFile!\n");
    exit(1);
  }*/

  return 0;
}


int writeDataFrame(unsigned char* data, unsigned int length) {
    char *frame = malloc(1024);
    int size = length + DATA_SIZE;
    unsigned char bcc2 = 0;
    int dataInd = 0;


    frame[0] = FLAG;
    frame[1] = ADDR_S;
    frame[2] = linkL->sequenceNumber << 5;
    frame[3] = frame[1] ^ frame[2];

    memcpy(&frame[4], data, length);

//    printf("length=%d\n",length);
   printf("firstdata=%02x\n",data[dataInd]);  

    for(dataInd = 0; dataInd < (size-DATA_SIZE); dataInd++) {
        bcc2 ^= data[dataInd];
    }
   
  printf("dataInd=%d\n",dataInd);

  printf("lastdata[%d]=%02x\n",data[dataInd], dataInd); 

    printf("bcc2=%02x\n",bcc2);
//    printf("dataInd=%d\n",dataInd); 

    frame[4 + length] = bcc2;
    frame[5 + length] = FLAG;

   printf("bcc2 na frame=%02x\n",frame[4 + length]);

   printf("SIZE BEFORE STUFF=%d\n",size);
 //   printf("flag na frame=%02x\n",frame[5 + length]);
    printf("5+length=%d\n",(5+length));
    frame = stuffing(frame, &size); 
    
  printf("bcc2 na frame depois do stuffing=%02x\n",frame[size - 2]);
//    printf("flag na frame depois do stuffing=%02x\n",frame[5 + length]);

  int res = 0;
  int i = 0;
    for(i=0; i<size;i++)
  printf("written[%i]=%02x\n",i,frame[i]);  

    if((res=write(appL->fileDescriptor, frame, size)) != size) {
      printf("Error on writing data frame!\n");
      exit(1);
    }
    else {
  printf("%d bytes written!\n", res);
  }


    return 0;
}

char* receiveFrame(FrameType *fType, FrameResponse *fResp, int *fSize) {
    char c;
    int res, ind=0;
    ReceivingState rState=0;

    while (alarmFlag != 1 && rState!=STOP) {
        res = read(appL->fileDescriptor, &c, 1);

	printf("0x%02x!\n", c);
	printf("rState=%d!\n", rState);
	printf("ind=%d!\n", ind);
	

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
      int dataInd=4;

      int size = ind;

	int i=0;

	for(i=4; i<ind - 2; i++) {
		printf("linkL->frame[%i]=%02x\n", i, linkL->frame[i]);
	}

	char * destuffed;

	destuffed=destuffing(linkL->frame, &size);

	size = ind - DATA_SIZE;

      strcpy(linkL->frame, destuffed);

      /* Is there necessity to check BCC1? */

     printf("first data=%02x\n", linkL->frame[dataInd]);

      for(i = 0; i < size; i++) {
        bcc2 ^= destuffed[dataInd++];
      }

	  printf("last data %02x\n", linkL->frame[dataInd-1]);
	  printf("bcc2=%02x\n",bcc2);

      if(linkL->frame[4 + size] != bcc2) {
        printf("Error on BCC2!\n");
        (*fResp) = RESP_REJ;
      }

      if(*fResp == 0)
        (*fResp) = RESP_RR;

    (*fSize) = ind;
    }
	printf("After evaluation!\n");
	

	return NULL;
}


