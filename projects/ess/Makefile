# $Id: Makefile,v 1.8 2003-11-02 19:13:16 peter Exp $

CC	= gcc
CFLAGS	= -DRESOLVE -Wall -O2
#CFLAGS	= -DIPV4_DEFAULT -DRESOLVE -Wall -O

# For Solaris:
#LIBS	= -lsocket -lnsl

SRC	= ess.c
EXEC	= ess

all:
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC) $(LIBS)
	strip $(EXEC)

install:
	install -c -m 755 $(EXEC) /usr/local/bin/$(EXEC)

clean:
	rm -f *.core *~ $(EXEC)
