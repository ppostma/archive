# $Id: Makefile,v 1.5 2003-08-16 14:19:00 peter Exp $

CC	= gcc
CFLAGS	= -DRESOLVE -Wall -O
#CFLAGS	= -DIPV4_DEFAULT -DRESOLVE -Wall -O

# For Solaris:
#LIBS	= -lsocket -lnsl
SRC	= ess.c
EXEC	= ess

default:
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC) $(LIBS)
	strip $(EXEC)

install:
	install -c -m 755 $(EXEC) /usr/local/bin/$(EXEC)

clean:
	rm -f *.core *~ $(EXEC)
