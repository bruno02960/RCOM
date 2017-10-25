# Makefile for Generator Program

COMPILER_TYPE = gnu

CC = gcc

PROG = fileTransfer
SRCS = applicationLayer.c linkLayer.c main.c alarm.c handlePackets.c stuffing.c transferFile.c handleFrames.c

CFLAGS = -Wall -g

$(PROG) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f program
	rm -f .depend
	rm -f *.d
	rm -f *.o
