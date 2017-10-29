#pragma once
#include "configs.h"

typedef struct {
    char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate; /*Velocidade de transmissão*/
    unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
    unsigned int timeout; /*Valor do temporizador*/
    unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
    unsigned char frame[PACKET_SIZE]; /*Trama*/
    unsigned char dataFrame[PACKET_SIZE]; /*Trama de dados*/
} linkLayer_t;

extern linkLayer_t* linkL;

/**
    * Initiates a new Link Layer struct and starts the connection
    @param port serial port used
    @param status indicates if the user is the Transmitter or Receiver
*/
int linkLayerInit(char* port, int status);

/**
    * Establishes the connection between the two computers
    @param fd serial port's file descriptor
*/
int llopen(int fd);

/**
    * Transmitter sends frames to the Receiver
    @param buffer data packet
    @param length data packet size
    @param fd serial port's file descriptor
*/
int llwrite(unsigned char* buffer, int length, int fd);

/**
    * Receiver reads frames from the Transmitter
    @param buffer data packet
    @param fd serial port's file descriptor
*/
int llread(unsigned char** buffer, int fd);

/**
    * Closes the connection
*/
int llclose(int fd);
