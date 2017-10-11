int llwrite(int fd, char * buffer, int length) {
  int alarmCounter = 0;
  int written;

  while (alarmCounter < NO_TRIES /* TODO: Substituir nr. tentativas */ ) {
    if (alarmCounter == 0 || alarmFlag == 1) {
      setAlarm();
      /* escreve I */
      alarmFlag = 0;
      alarmCounter++;
    }

     Recebe resposta

			if (receiveFrame(RR) == 0) {
				stopAlarm();
			} else if (receiveFrame(REF) == 0) {
				stopAlarm();
			}  
	}

    if (alarmCounter < NO_TRIES)
      printf("Connection successfully done!\n");
    else
      printf("Connection couldn't be done!\n");
    break;

  /* 
  codigo do writenoncanonical.c 

  Escreve I
  Espera por RR / REJ
  */

  return written;
  /*	– número de caracteres escritos
  	– valor negativo em caso de erro */
}

int llread(int fd, char * buffer) {
  int read;

  while (1) {
    /* Recebe I */

    /* Verifica info recebida e escreve RR / REJ */
  }

  /*
  codigo do noncanonical.c

  Espera por I
  Escreve RR / REJ
  */

  return read;
  /*	– comprimento do array
  	(número de caracteres lidos)
  	– valor negativo em caso de erro */
}

int llclose(int fd) {
  int alarmCounter = 0;

  switch (appL - > status) {
  case TRANSMITTER:
    while (alarmCounter < NO_TRIES /* TODO: Substituir nr. tentativas */ ) {

      if (alarmCounter == 0 || alarmFlag == 1) {
        setAlarm();
        writeCommand(SET);
        alarmFlag = 0;
        alarmCounter++;
      }

      if (receiveFrame(DISC) == 0)
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

      if(receiveFrame(UA) == 0)	/* Recebe UA */
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
  /*	– valor positivo em caso de sucesso
  	– valor negativo em caso de erro */

}

/* To be finished */
int receive() {
  int res;
  ReceivingrState rrState = START;

  while ((res = read(fd, & ch, 1)) > 0) {

    switch (rrState) {
    case START:
      if (c == FLAG) {
        ua[rrState] = c;
        rrState++;
      }
      break;
    case FLAG_RCV:
      if (c == ADDR) {
        ua[rrState] = c;
        rrState++;
      } else {
        if (c == FLAG)
          rrState = FLAG_RCV;
        else
          rrState = START;
      }
      break;
    case A_RCV:
      if (c == CTRL_UA) {
        ua[rrState] = c;
        rrState++;
      } else {
        if (c == FLAG)
          rrState = FLAG_RCV;
        else
          rrState = START;
      }
      break;
    case C_RCV:
      if (c == (ua[1] ^ ua[2])) {
        ua[rrState] = c;
        rrState++;
      } else {
        if (c == FLAG)
          rrState = FLAG_RCV;
        else
          rrState = START;
      }
      break;
    case BCC_OK:
      if (c == FLAG) {
        ua[rrState] = c;
        rrState++;
      } else
        rrState = 0;
      break;
    case STOP:
      return 1;
    }
  }
}

int frread(int fd, unsigned char * buf, int maxlen) {
  int n = 0;
  int ch;

  while (1) {
    if ((ch = read(fd, buf + n, 1)) <= 0) {
      return ch; // error...
    }

    if (n == 0 && buf[n] != FLAG) {
      continue;
    }

    if (n == 1 && buf[n] == FLAG) {
      continue;
    }

    n++;

    if (buf[n - 1] != FLAG && n == maxlen) {
      n = 0;
      continue;
    }

    if (buf[n - 1] == FFLAG && n > 2)
    //processrframe(buf,n);

      return n;
  }
}

/* Reserve space for function return */
char* suffing(char* buf) {
	char* stuffed;
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
	char* destuffed;

	int i, j;

	for (i = 0; i < strlen(buf); i++) {
		if (buf[i] == ESC_BYTE) {
			i++;
			destuffed[j] = buf[i] ^ 0x20;
		}
		else
			destuffed[j] = buf[i];
	}

	return buf;
}