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

int linkLayerInit(char * port, int status) {

  linkL = (linkLayer_t * ) malloc(sizeof(linkLayer_t));

  strcpy(linkL->port, port);

  linkL->timeout = 3;

  printf("%s", linkL->port);
  //linkL->baudRate = /*Velocidade de transmissão*/
  //	linkL->sequenceNumber = 0;	/*Número de sequência da trama: 0, 1*/
  //	linkL->timeout = /*Valor do temporizador: 1 s*/
  //	linkL->numTransmissions = /*Número de tentativas em caso de falha*/

  // HOW???
  //linkL->frame[MAX_SIZE];			/*Trama */

  applicationLayerInit(status);

  saveAndSetTermios();
  
  llopen();

  closeSerialPort();

  return 0;
}

/* Not finished AT ALL */
int receiveFrame(Command command) {
  char c;
  int res;
  char ua[5];
  ReceivingState rState;

  tcflush(appL->fileDescriptor, TCIFLUSH);

  while (rState != STOP && alarmFlag != 1) {
    if ((res = read(appL->fileDescriptor, & c, 1)) > 0) {
      switch (rState) {
      case START:
        if (c == FLAG) {
          ua[rState] = c;
          rState++;
        }
        break;
      case FLAG_RCV:
        if (c == ADDR_S || c == ADDR_R) {
          ua[rState] = c;
          rState++;
        } else {
          if (c == FLAG)
            rState = FLAG_RCV;
          else
            rState = START;
        }
        break;
      case A_RCV:
        switch (command) {
        case SET:
          if (c == CTRL_SET) { /* Verifies all the possible CTRL? */
            ua[rState] = c;
            rState++;
          }
          break;
        case DISC:
          if (c == CTRL_DISC) { /* Verifies all the possible CTRL? */
            ua[rState] = c;
            rState++;
          }
          break;
        case UA:
          if (c == CTRL_UA) { /* Verifies all the possible CTRL? */
            ua[rState] = c;
            rState++;
          }
          break;
        case RR:
          if (c == CTRL_RR) { /* Verifies all the possible CTRL? */
            ua[rState] = c;
            rState++;
          }
          break;
        case REJ:
          if (c == CTRL_RR) { /* Verifies all the possible CTRL? */
            ua[rState] = c;
            rState++;
          }
          break;
        }
        if (c == FLAG)
          rState = FLAG_RCV;
        else
          rState = START;
        break;
      case C_RCV:
        if (c == (ua[1] ^ ua[2])) {
          ua[rState] = c;
          rState++;
        } else {
          if (c == FLAG)
            rState = FLAG_RCV;
          else
            rState = START;
        }
        break;
      case BCC_OK:
        if (c == FLAG) {
          ua[rState] = c;
          rState++;
        } else
          rState = 0;
        break;
      case STOP:
        break;
      }
    }
  }

  if (alarmFlag == 1)
    return 1;
  else
    return 0;
}

/* NOT FINISHED */
int writeCommand(Command command) {
  unsigned char buf[COMMAND_SIZE];

  buf[0] = FLAG;

  if (appL->status == TRANSMITTER)
    buf[1] = ADDR_S;
  else
    buf[1] = ADDR_R;

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

  switch (appL->status) {
  case TRANSMITTER:
    while (alarmCounter < NO_TRIES /* TODO: Substituir nr. tentativas */ ) {
      if (alarmCounter == 0 || alarmFlag == 1) {
        setAlarm();
        writeCommand(SET);
        alarmFlag = 0;
        alarmCounter++;
      }

      if (receiveFrame(UA) == 0) {
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
    if (receiveFrame(SET) == 0 /* Recebe SET */ ) {
      writeCommand(UA);
      printf("Connection successfully done!\n");
    }
    break;
  }

  return appL->fileDescriptor;
  /*	– identificador da ligação de dados
  	– valor negativo em caso de erro */
}