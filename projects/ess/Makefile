# $Id: Makefile,v 1.4 2003-08-16 14:08:35 peter Exp $

CC	= gcc
CFLAGS	= -DRESOLVE -Wall -O
#CFLAGS	= -DIPV4_DEFAULT -DRESOLVE -Wall -O
SRC	= ess.c
EXEC	= ess

default:
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)
	strip $(EXEC)

solaris:
	$(CC) $(CFLAGS) -lsocket -lnsl -o $(EXEC) $(SRC)

install:
	install -c -m 755 $(EXEC) /usr/local/bin/$(EXEC)

clean:
	rm -f *.core *~ $(EXEC)
