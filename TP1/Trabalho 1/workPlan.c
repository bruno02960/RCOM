

int llwrite(int fd, char * buffer, int length) {
	int alarmCounter = 0;
	int written;

	while(alarmCounter < 3 /* TODO: Substituir nr. tentativas */) {
		if (alarmCounter == 0 || alarmFlag == 1) {
			setAlarm();
				/* escreve I */
			alarmFlag = 0;
			alarmCounter++;
		}

			/* Recebe resposta

			if ( RR ) {

		stopAlarm();
	
			} else if ( REJ ) {

		stopAlarm();
	
			} */
	}	

	/* Verificar sucesso / insucesso */

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

	while(1) {
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

	switch(appL->status) {
		case TRANSMITTER:
			while(alarmCounter < 3 /* TODO: Substituir nr. tentativas */) {

		if (alarmCounter == 0 || alarmFlag == 1) {
			setAlarm();
			writeCommand(SET);
			alarmFlag = 0;
			alarmCounter++;
		}

				/* Recebe DISC */
			writeCommand(UA);
	}	

						/* Recebe DISC */
						/* Envia UA */
			}	
			stopAlarm();

				/* Verificar sucesso / insucesso */
		    break; 
		case RECEIVER:
			while(alarmCounter < 3 /* TODO: Substituir nr. tentativas */) {

	if (alarmCounter == 0 || alarmFlag == 1) {
		setAlarm();
		writeCommand(SET);
		alarmFlag = 0;
		alarmCounter++;
	}

				/* Recebe UA */

							/* Recebe UA */
			}	

			stopAlarm();
						/* Verificar sucesso / insucesso */
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



/* Not finished AT ALL */
int saveAndSetTermios(){
	struct termios oldtio,newtio;		/*	Got to be saved on a global 

	/* save current port settings */
	if ( tcgetattr(fd,&oldtio) == -1)
	{ 
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = OPOST;

    /* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */


	if (tcflush(fd, TCIFLUSH) == -1)
	{
		perror("tcflush");
		exit(-1);
	}	

	if (tcsetattr(fd,TCSANOW,&newtio) == -1) 
	{
		perror("tcsetattr");
		exit(-1);
	}
}




/* To be finished */
int receive() {
	int res;
	ReceivingrState rrState = START;

	while((res=read(fd,&ch,1))>0) {

		switch(rrState) {
			case START:
				if (c == FLAG){
					ua[rrState]=c;
					rrState++;
				}
				break;
			case FLAG_RCV:
				if (c == ADDR){
					ua[rrState]=c;
					rrState++;
				}
				else {
					if (c==FLAG)
						rrState = FLAG_RCV;
					else
						rrState = START;
				}
				break;
			case A_RCV:
				if (c == CTRL_UA) {
					ua[rrState]=c;
					rrState++;
				}
				else {
					if (c==FLAG)
						rrState = FLAG_RCV;
					else
						rrState = START;
				}
				break;
			case C_RCV:
				if (c == (ua[1]^ua[2])) {
					ua[rrState]=c;
					rrState++;
				}		
				else {
					if (c==FLAG)
						rrState = FLAG_RCV;
					else
						rrState = START;
				}
				break;
			case BCC_OK:
				if (c == FLAG) {
					ua[rrState]=c;
					rrState++;
				}
				else
					rrState=0;
				break;
			case STOP:
				return 1;					
		}
	}
}

int frread(int fd, unsigned char * buf, int maxlen){
	int n=0;
	int ch;

	while(1){
		if((ch=read(fd, buf+n, 1)) <=0){
			return ch; // error...
		}

		if(n==0 && buf[n]!=FLAG){
			continue;
		}

		if(n==1 && buf[n]==FLAG){
			continue;
		}	

		n++;

		if(buf[n-1]!=FLAG && n==maxlen){
			n=0;
			continue;
		}

		if(buf[n-1]==FFLAG && n>2)
			//processrframe(buf,n);

		return n;
	}	
}
