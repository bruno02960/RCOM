#include "linkLayer.h"
#include "applicationLayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "alarm.h"
#include "definitions.h"
#include "configs.h"
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

char* receiveFrame(FrameType *fType, FrameResponse *fResp, int *fSize) {
    char c;
    int res, ind;
    char *ua = malloc(1024);
    ReceivingState rState;

    while (alarmFlag != 1) {
        res = read(appL->fileDescriptor, & c, 1);

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

    if (write(appL->fileDescriptor, & buf, 5) != sizeof(buf)) {
        printf("Error on writting!\n");
        return -1;
    }

    /* What's the adress byte? */
    /* What's RR & REJ BCC ? */
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

/* Reserve space for function return */
char* stuffing(char* buf) {
  char* stuffed = malloc(1024);
  unsigned int size = strlen(stuffed);

  stuffed[0] = buf[0];

  int i, j = 1;

  for (i = 1; i < (size - 1); i++, j++) {
    if(buf[i] == FLAG || buf[i] == ESC_BYTE) {
      size++;
      stuffed[j] = ESC_BYTE;
      j++;
      stuffed[j] = buf[i] ^ 0x20;
    }
    else
      stuffed[j] = buf[i];
  }

  stuffed[j] = buf[i];

  return stuffed;
}

/* Not finished AT ALL */
char* destuffing(char* buf) {
  char* destuffed = malloc(1024);

  int i, j=0;

  for (i = 0; i < strlen(buf); i++, j++) {
    if (buf[i] == ESC_BYTE) {
      i++;
      destuffed[j] = buf[i] ^ 0x20;
    }
    else
      destuffed[j] = buf[i];
  }

  return destuffed;
}

void writeControlPacket(int controlField) {
  char fileSize[14];  /* 10.7KB = 10 700B | log2(10 700)~=14 */
  sprintf(fileSize, "%d", traF->fileSize);

  int ctrlPkSize = 5 + strlen(fileSize) + strlen(FILE_PATH); /* 5 bytes
  from C, T1, L1, T2 and L2 */

  unsigned char controlPacket[ctrlPkSize];

  controlPacket[0] = controlField + '0';
  controlPacket[1] = FILE_SIZE + '0';
  controlPacket[2] = strlen(fileSize) + '0';

  int index = 3;
  int k;

  for(k = 0; k < strlen(fileSize); k++, index++)
    controlPacket[index] = fileSize[k];

  controlPacket[index++] = FILE_NAME + '0';
  controlPacket[index++] = strlen(FILE_PATH) + '0';

  for(k = 0; k < strlen(FILE_PATH); k++, index++)
    controlPacket[index] = fileSize[k];

  if(llwrite(controlPacket, ctrlPkSize) < 0) {
    printf("Error on llwrite!\n");
    exit(1);
  }
}

void writeDataPacket(char* buffer, int noBytes, int seqNo) {
  int dataPkSize = noBytes + 4; /* 4 bytes from C, N, L2 and L1 */

  unsigned char dataPacket[dataPkSize];

  dataPacket[0] = DATA_BYTE + '0';
  dataPacket[1] = seqNo + '0';

  /* K = 256 * dataPacket[2] + dataPacket[3] */
  dataPacket[2] = noBytes / 256;
  dataPacket[3] = noBytes % 256;
  memcpy(&dataPacket[4], buffer, noBytes);

  

  if(llwrite(dataPacket, dataPkSize) < 0) {
    printf("Error on llwrite!\n");
    exit(1);
  }
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

int receiveControlPacket(int controlField, int* noBytes, char** filePath) {
  unsigned char* controlPacket;

  /* Handle possible errors */
  llread(&controlPacket);

  if((controlPacket[0] - '0') != controlField) {
    printf("Unexpected control field!\n");
    exit(1);
  }

  if((controlPacket[1] - '0') != FILE_SIZE) {
    printf("Unexpected parameter!\n");
    exit(1);
  }

  int lengthSize = (controlPacket[2] - '0');
  int i, valueIndex = 3;
  char fileSize[STR_SIZE];

  for (i = 0; i < lengthSize; i++)
    fileSize[i] = controlPacket[valueIndex++];

  fileSize[valueIndex - 3] = '\0';
  (*noBytes) = atoi(fileSize);

  if((controlPacket[valueIndex++] - '0') != FILE_NAME)
    printf("Unexpected parameter!\n");

  int lengthPath = (controlPacket[valueIndex++] - '0');
  char path[STR_SIZE];

  for (i = 0; i < lengthPath; i++)
    path[i] = controlPacket[valueIndex++];

  path[i] = '\0';
  strcpy((*filePath), path);

  return 0;
}

int receiveDataPacket(unsigned char ** buffer, int sequenceNumber) {
  unsigned char* dataPacket;
  int read;

  /* Handle possible errors */
  llread(&dataPacket);

  int controlField = dataPacket[0] - '0';
  int seqNo = dataPacket[1] - '0';

  if(controlField != DATA_BYTE) {
    printf("Unexpected control field!\n");
    exit(1);
  }

  if(seqNo != sequenceNumber) {
    printf("Unexpected sequence number!\n");
    exit(1);
  }

  int l2 = dataPacket[2], l1 = dataPacket[3];
  read = 256 * l2 - l1;

  memcpy((*buffer), &dataPacket[4], read);
  free(dataPacket);

  return read;
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