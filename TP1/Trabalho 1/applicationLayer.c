#include "applicationLayer.h"

applicationLayer_t* appL;

int applicationLayerInit() {
	appL = (applicationLayer_t*) malloc(sizeof(applicationLayer_t));

	if((appL->fileDescriptor = open(/* What's the serial port to be oppened? */, O_RDWR | O_NOCTTY)) < 0)
	{
		printf("Error on oppening serial port!\n");
		exit(1);
	}

	appL->status = /* What's the application layer status? */;
}