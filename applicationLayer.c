#include "applicationLayer.h"
#include "linkLayer.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

applicationLayer_t* appL;

int applicationLayerInit(int status) {
	appL = (applicationLayer_t*) malloc(sizeof(applicationLayer_t));

	if((appL->fileDescriptor = open(linkL->port/* What's the serial port to be oppened? */, O_RDWR | O_NOCTTY)) < 0)
	{
		printf("Error on oppening serial port!\n");
		exit(1);
	}

	appL->status = status;/* What's the application layer status? */;

	printf("Status = %d", appL->status);

	return 0;
}
