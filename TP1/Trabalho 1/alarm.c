#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include "applicationLayer.h"
#include "linkLayer.h"

int alarmFlag = 0;
int fileDescriptor = 0;

void setVMIN (int noChars) {

	struct termios oldtio;

	/* save current port settings */
	if ( tcgetattr(appL->fileDescriptor,&oldtio) == -1)
	{ 
		perror("tcgetattr");
		exit(-1);
	}

  	oldtio.c_cc[VMIN]     = noChars;   /* blocking read until 5 chars received */

	tcflush(appL->fileDescriptor, TCIFLUSH);

	if ( tcsetattr(appL->fileDescriptor,TCSANOW,&oldtio) == -1) 
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


void setAlarm(int fd) {
	(void) signal(SIGALRM, alarmHandler);

	alarmFlag = 0;

	setVMIN(1);

	alarm(linkL->timeout);

	fileDescriptor=fd;
}

void stopAlarm(int fd) {
	(void) signal(SIGALRM, NULL);

	alarm(0);

	fileDescriptor=fd;
}
