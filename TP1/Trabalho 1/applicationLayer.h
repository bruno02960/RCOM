#pragma once

struct applicationLayer {
	int fileDescriptor;	/*Descritor correspondente à porta série*/
	int status;			/*TRANSMITTER | RECEIVER*/
}