/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
//UA 
#define FLAG 0x7E
#define ADDR 0x03
#define CTRL 0x07

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
	int fd,c, res;
	struct termios oldtio,newtio;
	char buf[10000];
	char ua[] = {FLAG, ADDR, CTRL, ADDR^CTRL, FLAG};

	if ( (argc < 2) || 
		((strcmp("/dev/ttyS0", argv[1])!=0) && 
			(strcmp("/dev/ttyS1", argv[1])!=0) )) 
	{
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

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
	newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 4;   /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	int it = 0;
	char ch;

	while (read(fd,&ch,1)) {

		printf("buf[%d] = %02x\n", it, ch);		
		buf[it++] = ch;		

		if (it == 5)
			break;
	}

	buf[it++]='\0';


	/*GUIAO 2*/
	


	newtio.c_oflag = OPOST;

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	res = write(fd,$ua,5); 
	printf("%d chars written\n", res); 
	sleep(1);

	//printf("SENT=%s\n", buf);

	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
	return 0;
}
