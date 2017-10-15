#include <stdio.h>
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
    //  linkL->sequenceNumber = 0;                /*Número de sequência da trama: 0, 1*/
    linkL->numTransmissions = NUM_TRANSMISSIONS;   /*Número de tentativas em caso de falha*/

                              // HOW???
                              //linkL->frame[MAX_SIZE];     /*Trama */

    applicationLayerInit(status);

    saveAndSetTermios();

    /* Handle errors */
    llopen();

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

    /* Handle errors */
    llclose();

    closeSerialPort();

    return 0;
}


int llopen() {
    int alarmCounter = 0;
    char* received;
    FrameType frType;

    switch (appL->status) {
      case TRANSMITTER:
          while (alarmCounter < NO_TRIES /* TODO: Substituir nr. tentativas */ ) {
              if (alarmCounter == 0 || alarmFlag == 1) {
                  setAlarm();
                  writeCommand(SET);
                  alarmFlag = 0;
                  alarmCounter++;
              }

              received=receiveFrame(&frType, NULL, NULL);

              if (received[2] == CTRL_UA) {
                  printf("UA received!\n");
                  break; /* Indicar qual se pretende receber ou verificar após ser recebido? */
              }
              /* Recebe UA */
          }

          stopAlarm();

          if (alarmCounter < NO_TRIES)
              printf("Connection successfully done!\n");
          else
              printf("Connection couldn't be done!\n");
          break;

      case RECEIVER:
          received=receiveFrame(&frType, NULL, NULL);

          if (received[2] == CTRL_SET /* Recebe SET */ ) {
              writeCommand(UA);
              printf("Connection successfully done!\n");
          }
          else
              printf("Connection couldn't be done!\n");
          break;
    }

    return appL->fileDescriptor;
    /*  – identificador da ligação de dados
      – valor negativo em caso de erro */
}

int llwrite(unsigned char * buffer, int length) {
  int alarmCounter = 0;
  int written = 0;
  char * received;

  while (alarmCounter < NO_TRIES){
    if (alarmCounter == 0 || alarmFlag == 1) {
      setAlarm();
      writeDataFrame(buffer, length);
      alarmFlag = 0;
      alarmCounter++;
    }

    received=receiveFrame(NULL, NULL, NULL);

   if (received[2] == CTRL_RR) {
       stopAlarm();
      } 
   else if (received[2] == CTRL_REJ) {
       stopAlarm();
      }  
    } 

    if (alarmCounter < NO_TRIES)
      printf("Written!\n");
    else
      printf("Couldn't write!\n");

  /* 
  codigo do writenoncanonical.c 

  Escreve I
  Espera por RR / REJ
  */

  return written;
  /*  – número de caracteres escritos
    – valor negativo em caso de erro */
}

int llread(unsigned char ** buffer) {
  int read = 0, disconnect = 0, fSize, dataSize;
  char * frame;
  FrameType frType;
  FrameResponse fResp;

  while (!disconnect) {
    frame = receiveFrame(&frType, &fResp, &fSize);

    switch(frType) {
      case COMMAND:
        if(frame[2] == CTRL_DISC)
          disconnect = 1;
        break;
      case DATA:
        if(fResp == RESP_RR && ((frame[2]>>5) & BIT(0)) == linkL->sequenceNumber) {
          linkL->sequenceNumber = !linkL->sequenceNumber;
          dataSize = fSize - DATA_SIZE;
          *buffer = malloc(dataSize);
          memcpy(*buffer, &frame[4], dataSize);
          disconnect = 1;
        } 
        else
          if (fResp == RESP_REJ) {
            linkL->sequenceNumber = ((frame[2]>>5) & BIT(0));
          }

          if(fResp == RESP_REJ)
            writeCommand(REJ);
          else
            writeCommand(RR);

      default:
        exit(1);   
    }

    return 0;
    /* Recebe I */

    /* Verifica info recebida e escreve RR / REJ */

    /* Fica alerta para a possibilidade de DISC */
  }

  /*
  codigo do noncanonical.c

  Espera por I
  Escreve RR / REJ
  */

  return read;
  /*  – comprimento do array
    (número de caracteres lidos)
    – valor negativo em caso de erro */
}

int llclose() {
  int alarmCounter = 0;
  char * response;

  switch (appL -> status) {
  case TRANSMITTER:
    while (alarmCounter < NO_TRIES /* TODO: Substituir nr. tentativas */ ) {

      if (alarmCounter == 0 || alarmFlag == 1) {
        setAlarm();
        writeCommand(SET);
        alarmFlag = 0;
        alarmCounter++;
      }

      response = receiveFrame(NULL, NULL, NULL);

      if (response[2] == CTRL_DISC)
        writeCommand(UA); /* Recebe DISC */
    }
    stopAlarm();
    break;
  case RECEIVER:
    while (alarmCounter < NO_TRIES /* TODO: Substituir nr. tentativas */ ) {

      if (alarmCounter == 0 || alarmFlag == 1) {
        setAlarm();
        writeCommand(DISC);
        alarmFlag = 0;
        alarmCounter++;
      }

      response = receiveFrame(NULL, NULL, NULL);

      if (response[2] == CTRL_UA)
        printf("Disconnection successfully done!\n");
    }

    stopAlarm();
    
    if (alarmCounter < NO_TRIES)
      printf("Connection successfully done!\n");
    else
      printf("Connection couldn't be done!\n");

    break;
  }

  /*
  Escreve DISC
  Recebe DISC
  Escreve UA
  */

  return 1;
  /*  – valor positivo em caso de sucesso
    – valor negativo em caso de erro */

}



int writeCommand(Command command) {
    unsigned char buf[COMMAND_SIZE];

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
        buf[2] = CTRL_RR;
        break;
    case REJ:
        buf[2] = CTRL_REJ;
        break;
    default:
        break;
    }
    buf[3] = buf[1] ^ buf[2]; //BCC

    buf[4] = FLAG;

    tcflush(appL->fileDescriptor, TCOFLUSH);

    if (write(appL->fileDescriptor, &buf, 5) != sizeof(buf)) {
        printf("Error on writting!\n");
        return -1;
    }

    /* What's the adress byte? */
    /* What's RR & REJ BCC ? */
    return 0;

}

int sendFile() {
  char * packetBuffer = malloc(PACKET_SIZE * sizeof(char));
  int read, seqNo;
  
  /* Handle possible error */
  writeControlPacket(CTRL_START);

  while((read=fread(packetBuffer, sizeof(char), PACKET_SIZE, traF->file)) > 0) {
    /* Handle possible error */
    writeDataPacket(packetBuffer, read, (seqNo % 255)); /* seqNo is module 255 */

    seqNo++;
  }

  transferFileClose();

  /* Handle possible error - In case of success*/
  writeControlPacket(CTRL_END);

  return 0;
}


int receiveFile() {
  int fileSize;

  /* Handle errors */
  receiveControlPacket(START_BYTE, &fileSize, &FILE_PATH);

  int read, noBytes, seqNo;
  unsigned char * buffer = malloc(PACKET_SIZE * sizeof(char));

  while(noBytes < fileSize) {
    if((read = receiveDataPacket(&buffer, seqNo % 255))<0)
      exit(1);

    noBytes += read;

    fwrite(buffer, sizeof(char), read, traF->file);
    seqNo++;
  }

  transferFileClose();

  /* Handle errors */
  receiveControlPacket(END_BYTE, &fileSize, &FILE_PATH);

  return 0;
}


int writeDataFrame(unsigned char* data, unsigned int length) {
    char *frame = malloc(1024);
    int size = length + DATA_SIZE;
    unsigned char bcc2;
    int dataInd, i;

    frame[0] = FLAG;
    frame[1] = ADDR_S;
    frame[2] = linkL->sequenceNumber << 5;
    frame[3] = frame[1] ^ frame[2];
    memcpy(&frame[4], data, size);

    for(i = 0; i < size; i++) {
        bcc2 ^= data[dataInd++];
    }

    frame[4 + size] = bcc2;
    frame[5 + size] = FLAG;

    frame = stuffing(frame);

    if(write(appL->fileDescriptor, frame, size) != size) {
      printf("Error on writing data frame!\n");
      exit(1);
    }

    return 0;
}

char* receiveFrame(FrameType *fType, FrameResponse *fResp, int *fSize) {
    char c;
    int res, ind;
    char *ua = malloc(1024);
    ReceivingState rState;

    while (alarmFlag != 1) {
        res = read(appL->fileDescriptor, &c, 1);

        if (res > 0) {
            switch(rState) {
            case START:
                if (c == FLAG) {
                    ua[ind++]=c;
                    rState++;
                }
                break;
            case FLAG_RCV:
                if (c == ADDR_S || c == ADDR_R) {
                    ua[ind++]=c;
                    rState++;
                }
                else if (c!=FLAG) {
                    rState = START;
                    ind = 0;
                }
                break;
            case A_RCV:
                if (c != FLAG) {
                    ua[ind++]=c;
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
                if (c == (ua[1]^ua[2])) {
                    ua[ind++]=c;
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
                    ua[ind++]=c;
                    rState++;

                    if(ind > 5)
                        (*fType) = DATA;
                }
                else
                    ua[ind++] = c;
                break;
            case STOP:
                break;
            default:
                break;
            }
        }
        else
            return ua;
    }

    if((*fType) == DATA) {
      unsigned char bcc2;
      int dataInd=4;

      ua = destuffing(ua);

      /* Is there necessity to check BCC1? */

      int size = ind - DATA_SIZE;

      int i;
      for(i = 0; i < size; i++) {
        bcc2 ^= ua[dataInd++];
      }

      if(ua[4 + size] != bcc2) {
        printf("Error on BCC2!\n");
        (*fResp) = RESP_REJ;
      }

      if(*fResp == 0)
        (*fResp) = RESP_RR;
    }

    (*fSize) = ind;

    return ua;
}