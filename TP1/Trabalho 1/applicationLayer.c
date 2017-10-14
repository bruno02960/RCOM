#include "applicationLayer.h"
#include "linkLayer.h"
#include "configs.h"
#include "definitions.h"
#include "alarm.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>

applicationLayer_t* appL;
transferFile_t* traF;
struct termios oldtio, newtio;
char* FILE_PATH="./file";

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

    if (appL->status == TRANSMITTER) {
        if (!(traF->file = fopen(FILE_PATH, "rb"))) {
        printf("Unable to open file!\n");
            exit(1);
        }
        else if (!(traF->file = fopen(FILE_PATH, "wb"))) {
        printf("Unable to open file!\n");
            exit(1);
        }
    }

    if(stat(FILE_PATH,&st)==0)
    	traF->fileSize = st.st_size;
    else
    {
        printf("Unable to get file size!\n");
        exit(1);
     }

    return 0;
}

void transferFileClose() {
	if(fclose(traF->file) < 0) {
		printf("Unable to close transfer file!\n");
		exit(1);
	}
}

int writeDataFrame(unsigned char* data, unsigned int length) {
    char *frame = malloc(1024);
    int size = length + DATA_SIZE;
    unsigned char bcc2;
    int dataInd, i;

    frame[0] = FLAG;
    frame[1] = ADDR_S;
    frame[2] = linkL->sequenceNumber << 5;
    frame[3] = frame[1] ^ frame[2];
    memcpy(&frame[4], data, size);

    for(i = 0; i < size; i++) {
        bcc2 ^= data[dataInd++];
    }

    frame[4 + size] = bcc2;
    frame[5 + size] = FLAG;

    frame = stuffing(frame);

    if(write(appL->fileDescriptor, frame, size) != size) {
    	printf("Error on writing data frame!\n");
    	exit(1);
    }

    return 0;
}