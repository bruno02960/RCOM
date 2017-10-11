#include "alarm.h"

typedef enum {
	SET, DISC, UA, RR, REJ
} Command

int llopen(int porta, int flag) {
	int alarmCounter = 0;

	/* Guarda flag para uso futuro */
	appL->status = flag;

	switch(appL->status) {
		case TRANSMITTER:
			while(alarmCounter < 3 /* TODO: Substituir nr. tentativas */) {
				if (alarmCounter == 0 || alarmFlag == 1) {
					setAlarm();
					writeCommand(SET);
					alarmFlag = 0;
					alarmCounter++;
				}

						/* Recebe UA */
			}

			receiveFrame(appL->fileDescriptor); /* Indicar qual se pretende receber ou verificar após ser recebido? */
			/* Recebe UA */

			stopAlarm();

			/* Verificar sucesso / insucesso do SET*/
		break;
	case RECEIVER:
			if( /* Recebe SET */){
				/* Escreve UA */	
		}
		break;
	
	return appL->fileDescriptor;
		/*	– identificador da ligação de dados
			– valor negativo em caso de erro */
}

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
int receiveFrame(int fd) {
	char c;
	int res;

	tcflush(*fd, TCIFLUSH);

	while(state!=STOP && flag!=1) {

		if((res=read(*fd,&ch,1))>0) {
			switch(state) {
				case START:
				if (c == FLAG){
					ua[state]=c;
					state++;
				}
				break;
				case FLAG_RCV:
				if (c == ADDR){
					ua[state]=c;
					state++;
				}
				else {
					if (c==FLAG)
						state = FLAG_RCV;
					else
						state = START;
				}
				break;
				case A_RCV:
				if (c == CTRL) {
					ua[state]=c;
					state++;
				}
				else {
					if (c==FLAG)
						state = FLAG_RCV;
					else
						state = START;
				}
				break;
				case C_RCV:
				if (c == (ua[1]^ua[2])) {
					ua[state]=c;
					state++;
				}		
				else {
					if (c==FLAG)
						state = FLAG_RCV;
					else
						state = START;
				}
				break;
				case BCC_OK:
				if (c == FLAG) {
					ua[state]=c;
					state++;
				}
				else
					state=0;
				break;					
			}
		}
	}

	if (flag == 1)
		return 1;
	else
		return 0;
}

/* Not finished AT ALL */
int writeSet(int fd) {
	tcflush(*fd, TCOFLUSH);
	unsigned char set[]={FLAG, ADDR, CTRL, ADDR^CTRL, FLAG},
	
	if(write(*fd,&set,5)!=sizeof(set)) {
		printf("Error on writting!\n");
		return ERROR;
	}

	return 0;
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


/* NOT FINISHED */
int writeCommand(Command command) {
	unsigned char buf[COMMAND_SIZE];

	buf[0] = FLAG;

	if (appL->status == TRANSMITTER)
		buf[1] = ADDR_S;
	else
		buf[1] = ADDR_R;

	switch(command) {
		case SET:
			buf[2] = CTRL_SET;
			break;
		case DISC:
			buf[2] = DISC_SET;
			break;
		case UA:
			buf[2] = UA_SET;
			break;
		case RR:
			buf[2] = RR_SET;
			break;
		case REJ:
			buf[2] = REJ_SET;
			break;
		default:
			break;	
	}
	buf[3] = buf[1]^buf[2]; //BCC

	buf[4] = FLAG;

	tcflush(appL->fileDescriptor, TCOFLUSH);

	if(write(appL->fileDescriptor,&buf,5)!=sizeof(buf)) {
		printf("Error on writting!\n");
		return ERROR;
	}

	/* What's the adress byte? */
	/* What's RR & REJ BCC ? */
	return 0;

}

typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP } ReceivingState;


/* To be finished */
int receive() {
	int res;
	ReceivingState rState = START;

	while((res=read(fd,&ch,1))>0) {

		switch(rState) {
			case START:
				if (c == FLAG){
					ua[rState]=c;
					rState++;
				}
				break;
			case FLAG_RCV:
				if (c == ADDR){
					ua[rState]=c;
					rState++;
				}
				else {
					if (c==FLAG)
						rState = FLAG_RCV;
					else
						rState = START;
				}
				break;
			case A_RCV:
				if (c == CTRL_UA) {
					ua[rState]=c;
					rState++;
				}
				else {
					if (c==FLAG)
						rState = FLAG_RCV;
					else
						rState = START;
				}
				break;
			case C_RCV:
				if (c == (ua[1]^ua[2])) {
					ua[rState]=c;
					rState++;
				}		
				else {
					if (c==FLAG)
						rState = FLAG_RCV;
					else
						rState = START;
				}
				break;
			case BCC_OK:
				if (c == FLAG) {
					ua[rState]=c;
					rState++;
				}
				else
					rState=0;
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

		if(buf[n-1]==FFLAG && n>2){
			//processrframe(buf,n);

		return n;
		{
	}	
}