# $Id: Makefile,v 1.3 2003-08-13 11:12:43 peter Exp $

CC	= gcc
CFLAGS	= -DRESOLVE -Wall -O
#CFLAGS	= -DIPV4_DEFAULT -DRESOLVE -Wall -O
SRC	= ss.c
EXEC	= ss

default:
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)
	strip $(EXEC)

solaris:
	$(CC) $(CFLAGS) -lsocket -lnsl -o $(EXEC) $(SRC)

install:
	install -c -m 755 $(EXEC) /usr/local/bin/$(EXEC)

clean:
	rm -f *.core *~ $(EXEC)
