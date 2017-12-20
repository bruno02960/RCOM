# Makefile for Generator Program

COMPILER_TYPE = gnu

CC = gcc

PROG = download
SRCS = ftp.c

CFLAGS = -g

$(PROG) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f program
	rm -f .depend
	rm -f *.d
	rm -f *.o
