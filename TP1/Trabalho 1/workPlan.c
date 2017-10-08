#define ERROR -1
#define TRANSMITTER 0
#define RECEIVER 1

int llopen(int porta, int flag) {
	int fd;

	/* Guarda flag para uso futuro */

	switch(flag){
		case TRANSMITTER:
		/* codigo do writenoncanonical.c */
		/* Escreve SET */
		/* Recebe UA */
		break;

		case RECEIVER:
		/* codigo do noncanonical.c */
		/* Recebe SET */
		/* Escreve UA */
		break;

		default:
		break;
	}

	return fd;
	/*	– identificador da ligação de dados
		– valor negativo em caso de erro */
}

int llwrite(int fd, char * buffer, int length) {
	int written;
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

int openSerialPort(char *port) {
	return open(port, O_RDWR | O_NOCTTY );
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