#pragma once

struct applicationLayer_t {
	int fileDescriptor;	/*Descritor correspondente à porta série*/
	int status;			/*TRANSMITTER | RECEIVER*/
}

extern applicationLayer_t* appL;