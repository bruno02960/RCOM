/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//SET 
#define FLAG 0x7E
#define ADDR 0x03
#define CTRL_SET 0x03
#define CTRL_UA 0x07

typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP } stateMachine_t;

//volatile int STOP=FALSE;

int flag = 1, conta = 0;
stateMachine_t state = START;
char ua[5];


void atende()
{
	flag = 1;
	conta++;

	printf("Atendeu!\n");
}

int writeSet(int fd) {
	unsigned char set[]={FLAG, ADDR, CTRL_SET, ADDR^CTRL_SET, FLAG};

	tcflush(fd, TCOFLUSH);
	write(fd,&set,5);

	return 0;
}

int processBytes(unsigned char c) {
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
			if (c == CTRL_UA) {
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
		case STOP:
			return 1;					
		}

	return 0;
}

int receiveFrame(int fd) {
	char ch;
	int res;

	tcflush(fd, TCIFLUSH);

	while(state!=STOP && flag!=1) {

		if((res=read(fd,&ch,1))>0) {
			processBytes(ch);
		}
	}

	if (flag == 1)
		return 1;
	else
		return 0;
}

int main(int argc, char** argv)
{
	int fd;
	struct termios oldtio,newtio;

	if ( (argc < 2) || 
		((strcmp("/dev/ttyS0", argv[1])!=0) && 
			(strcmp("/dev/ttyS1", argv[1])!=0) ))
	{
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

	fd = open(argv[1], O_RDWR | O_NOCTTY );
	if (fd <0) {perror(argv[1]); exit(-1); }

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


	tcflush(fd, TCIFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) 
	{
		perror("tcsetattr");
		exit(-1);
	}

  (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

  while(conta < 3) {
  	if(flag == 1) {
  		alarm(3);
  		writeSet(fd);
  		flag = 0;
  	}

  	if(receiveFrame(fd)==0) {
  		printf("Correct response received!\n");
  		break;
  	}
  }

  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);

  return 0;
}
>>>>>>> b3a80dedccf0d6fb063afffe2a3ceeee161438b9
