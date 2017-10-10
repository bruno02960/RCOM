#include <unistd.h>
#include <signal.h>
#include <stdio.h>

int alarmFlag = 0;

void alarmHandler()
{
	printf("Alarm!\n");
	alarmFlag = 1;
}


void setAlarm() {
	struct sigaction new;

	/* Set up the structure to specify the new action. */
	new.sa_handler = alarmHandler;
	sigemptyset (&new.sa_mask);
	new.sa_flags = 0;

	alarmFlag = 0;

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