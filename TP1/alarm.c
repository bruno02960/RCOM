#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <termios.h>

int alarmFlag = 0;

void setVMIN (int noChars) {

	struct termios oldtio;

	/* save current port settings */
	if ( tcgetattr(fd,&oldtio) == -1)
	{ 
		perror("tcgetattr");
		exit(-1);
	}

  	oldtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

	tcflush(fd, TCIFLUSH);

	if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) 
	{
		perror("tcsetattr");
		exit(-1);
	}
}

void alarmHandler()
{
	printf("Alarm!\n");
	alarmFlag = 1;

	setVMIN(0);
}


void setAlarm() {
	struct sigaction new;

	/* Set up the structure to specify the new action. */
	new.sa_handler = alarmHandler;
	sigemptyset (&new.sa_mask);
	new.sa_flags = 0;

	alarmFlag = 0;

	setVMIN(1);

	alarm(linkL->timeout);
}

void stopAlarm() {
	struct sigaction new;

	/* Set up the structure to specify the new action. */
	new.sa_handler = NULL;
	sigemptyset (&new.sa_mask);
	new.sa_flags = 0;

	alarm(0);
}
