#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "applicationLayer.h" 
#include "linkLayer.h"

int main(int argc, char** argv) {
	/* Args handling */

	/* Init:
		- status;
		- port;
		- numTransmissions;
		- timeout;
		- baudrate;
		- filename? packetSize?
		*/	

	// reads config file and saves shit on linklayer

	//main args are port and status

 	//receiver or transmitter?
	//0 -> T
	//1-> R

	if ( (argc < 3) || 
		((strcmp("/dev/ttyS0", argv[1])!=0) && 
			(strcmp("/dev/ttyS1", argv[1])!=0) ))
		{
			printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
			exit(1);
		}


	char port[20];
	strcpy(port, argv[1]);
	int status = atoi(argv[2]);

	linkLayerInit(port, status);

	return 0;
}
