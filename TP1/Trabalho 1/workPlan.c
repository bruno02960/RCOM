

int llwrite(int fd, char * buffer, int length) {
  int alarmCounter = 0;
  int written;
  char * received;
  int frType;

  while (alarmCounter < NO_TRIES /* TODO: Substituir nr. tentativas*/){
    if (alarmCounter == 0 || alarmFlag == 1) {
      setAlarm();
      writeData(); /* To be developed */
      
      /* escreve I -> writeCommand */

      alarmFlag = 0;
      alarmCounter++;
    }

    // Recebe resposta

    received=receiveFrame(&frType, NULL, NULL);

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
    break;

  /* 
  codigo do writenoncanonical.c 

  Escreve I
  Espera por RR / REJ
  */

  return written;
  /*  – número de caracteres escritos
    – valor negativo em caso de erro */
}





int llread(char ** buffer) {
  int read, disconnect, fSize, dataSize;
  char * frame;
  FrameType frType;
  FrameResponse fResp;

  while (!disconnect) {
    frame = receiveFrame(&frType, &fResp. &fSize);

    switch(frType) {
      case COMMAND:
        if(frame[2] == CTRL_DISC)
          disconnect = 1;
        break;
      case DATA:
        if(fResp == RR && ((frame[2]>>5) & BIT(0)) == linkL->sequenceNumber) {
          linkL->sequenceNumber = !linkL->sequenceNumber;
          dataSize = fSize - DATA_SIZE;
          *buffer = malloc(dataSize);
          memcpy(*buffer, &frame[4], dataSize);
          disconnect = 1;
        } 
        else
          if (fResp == REJ) {
            linkL->sequenceNumber = ((frame[2]>>5) & BIT(0));
          }

          sendCommand(fResp);
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

      if(receiveFrame(UA) == 0) /* Recebe UA */
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
    //processframe(buf,n);

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

  /* TODO: Chama llwrite, lidando com os erros */
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

  /* TODO: Chama llwrite, lidando com os erros */
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

  /* Handle possible error - In case of success*/
  writeControlPacket(CTRL_END);

  return 0;
}

int receiveControlPacket() {

}

int receiveDataPacket() {
  
}

int receiveFile() {
}