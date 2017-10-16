# Makefile for Generator Program

COMPILER_TYPE = gnu

CC = gcc

PROG = program
SRCS = applicationLayer.c linkLayer.c main.c alarm.c handlePackets.c stuffing.c transferFile.c

CFLAGS = -Wall

$(PROG) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(PROG)