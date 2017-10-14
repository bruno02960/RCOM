#include "applicationLayer.h"
#include "linkLayer.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <strings.h>
#include <unistd.h>

applicationLayer_t* appL;
transferFile_t* traF;
struct termios oldtio, newtio;


int applicationLayerInit(int status) {
	appL = (applicationLayer_t*) malloc(sizeof(applicationLayer_t));

	if((appL->fileDescriptor = open(linkL->port/* What's the serial port to be oppened? */, O_RDWR | O_NOCTTY)) < 0)
	{
		printf("Error on oppening serial port!\n");
		exit(1);
	}

	appL->status = status;	/* What's the application layer status? */;

	transferFileInit();

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

	newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
	newtio.c_cc[VMIN] = 1; /* blocking read until 5 chars received */

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

int closeSerialPort(){
	if (tcsetattr(appL->fileDescriptor,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    return 1;
	}

	close(appL->fileDescriptor);

	return 0;
}

int transferFileInit() {
	struct stat st;

	traF = (transferFile_t*) malloc(sizeof(transferFile_t));

	if (appL->status == TRANSMITTER)
		if (!(traF->file = fopen(FILE_PATH, "rb")){
			printf("Unable to open file!\n");
			exit(1);
		}
	else
		if (!(traF->file = fopen(FILE_PATH, "wb")){
			printf("Unable to open file!\n");
			exit(1);
		}

	if(stat(FILE_PATH,&st)==0)
		traF->fileSize = st.st_size;
	else {
		printf("Unable to get file size!\n");
		exit(1);
	}

	return 0;	
}

int transferFileClose() {
	if(fclose(traF->file) < 0) {
		printf("Unable to close transfer file!\n");
		exit(1);
	}
}