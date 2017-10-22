#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include "transferFile.h"
#include "applicationLayer.h"
#include "linkLayer.h"
#include "configs.h"
#include "definitions.h"
#include "alarm.h"

applicationLayer_t* appL;
struct termios oldtio, newtio;

int applicationLayerInit(int status) {
	appL = (applicationLayer_t*) malloc(sizeof(applicationLayer_t));

	if((appL->fileDescriptor = open(linkL->port, O_RDWR | O_NOCTTY)) < 0)
	{
		printf("Error on oppening serial port!\n");
		exit(1);
	}
	else {
		printf("Serial port opened!\n");
	}

	appL->status = status;

	transferFileInit(appL->status);

	return 0;
}

int saveAndSetTermios() {
	/* save current port settings */
	if (tcgetattr(appL->fileDescriptor, &oldtio) == -1) {
	    perror("tcgetattr");
	    return 1;
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = OPOST;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME] = 0;		/* inter-character timer unused */
	newtio.c_cc[VMIN] = 1;		/* blocking read until a char is received */

	if (tcflush(appL->fileDescriptor, TCIFLUSH) == -1) {
	    perror("tcflush");
	    return 1;
	}

	if (tcsetattr(appL->fileDescriptor, TCSANOW, & newtio) == -1) {
	    perror("tcsetattr");
	    return 1;
	}

	return 0;
}

int closeSerialPort(int fd){
	if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    return 1;
	}

	close(fd);

	return 0;
}
