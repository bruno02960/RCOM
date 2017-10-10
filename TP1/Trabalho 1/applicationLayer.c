#include "applicationLayer.h"

applicationLayer_t* appL;

int init() {
	appL = (applicationLayer_t*) malloc(sizeof(applicationLayer_t));
}