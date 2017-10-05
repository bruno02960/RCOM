/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//SET 
#define FLAG 0x7E
#define ADDR 0x03
#define CTRL 0x03

typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP } stateMachine_t;

volatile int STOP=FALSE;

int flag = 1, conta = 0;
stateMachine_t stMac = START;
char ua[5];


void atende()
{
	flag = 1;
	conta++;
}

int writeSet(int fd) {
	tcflush(*fd, TCOFLUSH);
	unsigned char set[]={FLAG, ADDR, CTRL, ADDR^CTRL, FLAG},
	write(*fd,&set,5);

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
			if (c == CTRL/* WHAT'S TO RECEIVE? */) {
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

int receiveFrame(int fd) {
	char c;
	int res;

	tcflush(*fd, TCIFLUSH);

	while(state!=STOP && flag!=1) {

		if((res=read(*fd,&ch,1))>0) {
			processBytes(c);
		}
	}

	if (flag == 1)
		return 1;
	else
		return 0;
}

int main(int argc, char** argv)
{
	int fd,c, res;
	struct termios oldtio,newtio;
	char buf[255];
	int i, sum = 0, speed = 0;

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

  int it = 0;
  char ch;

  /* GUIAO 2 */
  while(conta < 3) {
  	if(flag == 1) {
  		alarm(3);
  		writeSet(&fd);
  		flag = 0;
  	}

  	if(receiveFrame(&fd)==0) {
  		printf("Correct response received!\n");
  		break;
  	}
  }

  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);

  return 0;
}