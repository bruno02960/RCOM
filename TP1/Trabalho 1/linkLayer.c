#include "linkLayer.h"
#include "applicationLayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "alarm.h"
#include "definitions.h"
#define NO_TRIES 3

linkLayer_t * linkL;

typedef enum {
    SET,
    DISC,
    UA,
    RR,
    REJ
}
Command;

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
    COMMAND,
    DATA
}
FrameType;

typedef enum {
    RR,
    REJ
}
FrameResponse;

int linkLayerInit(char * port, int status) {

    linkL = (linkLayer_t * ) malloc(sizeof(linkLayer_t));

    strcpy(linkL->port, port);

    linkL->timeout = TIMEOUT;
    linkL->baudRate = BAUDRATE;                   /*Velocidade de transmissão*/
    //  linkL->sequenceNumber = 0;                /*Número de sequência da trama: 0, 1*/
    linkL->numTransmissions = NUM_TRANSMISSIONS   /*Número de tentativas em caso de falha*/

                              // HOW???
                              //linkL->frame[MAX_SIZE];     /*Trama */

                              applicationLayerInit(status);

    saveAndSetTermios();

    llopen();

    int fileSize = openFile();

    closeSerialPort();

    return 0;
}

char* receiveFrame(FrameType *fType, FrameResponse *fResp. int *fSize) {
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
                if (c == ADDR_S || c=ADDR_R) {
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

    if(fType == DATA) {
      unsigned char bcc2;
      int dataInd=4;

      ua = destuff(ua);

      /* Is there necessity to check BCC1? */

      size = ind - DATA_SIZE;

      int i;
      for(i = 0; i < size; i++) {
        bcc2 ^= ua[dataInd++];
      }

      if(ua[4 + size] != bcc2) {
        printf("Error on BCC2!\n");
        (*fResp) = REJ
      }

      if(*fResp == 0)
        (*fResp) == RR;
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
    int frType;

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