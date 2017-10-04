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

volatile int STOP=FALSE;

int flag = 1, conta = 1;

void atende()
{
  flag = 1;
  conta++;
	printf("\nAtendeu\n");
}

int main(int argc, char** argv)
{
  int fd,c, res;
  struct termios oldtio,newtio;
  char buf[255], set[]={FLAG, ADDR, CTRL, ADDR^CTRL, FLAG},
    ua[5];
  int i, sum = 0, speed = 0;

  (void) signal(SIGALRM, atende);  // instala  rotina que atende interrupcao

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

  int it = 0;
  char ch;
	int read = 0;

  /* GUIAO 2 */
  while(conta <= 3) {
    if(flag) {
      alarm(3);
      flag = 0;
    }

	if(!read)
    write(fd,&set,5);

	printf("HERE\n");

    newtio.c_oflag = 0;
  	tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) 
    {
      perror("tcsetattr");
      exit(-1);
    }
    
    while(read(fd,&ch,1) && !read){
      printf("ua[%d] = %02x \n",it,ch); 
      ua[it++]=ch;


		if (it == 4)
			read = 1;

    }

  }

  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);

  return 0;
}
