#include <unistd.h>
#include <signal.h>
#include <stdio.h>

int flag=1, conta=1;

void handerALRM()					// atende alarme
{
	printf("alarme # %d\n", conta);
	flag=1;
	conta++;
}


void setAlarm() {
	(void) signal(SIGALRM, handerALRM);	// instala  rotina que atende interrupcao
	alarm(3);                 		// activa alarme de 3s							TIME
}

void stopAlarm() {
	(void) signal(SIGALRM, NULL);	// instala  rotina que atende interrupcao
	alarm(0);                 		// activa alarme de 3s							TIME
}