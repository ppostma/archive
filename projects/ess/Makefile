# $Id: Makefile,v 1.1 2003-07-19 18:42:35 peter Exp $

CC	= gcc
CFLAGS	= -DRESOLVE -Wall -O
SRC	= ss.c
EXEC	= ss

default:
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)
	strip $(EXEC)

solaris:
	$(CC) $(CFLAGS) -lsocket -lnsl -o $(EXEC) $(SRC)

clean:
	rm -f *.core *~ $(EXEC)
