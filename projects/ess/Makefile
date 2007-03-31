# $Id: Makefile,v 1.11 2007-03-31 18:19:43 peter Exp $

CC	= gcc
CFLAGS	= -DRESOLVE -Wall -O2
#CFLAGS	= -DIPV4_DEFAULT -DRESOLVE -Wall -O

PREFIX	= /usr/local

SRC	= ess.c
EXEC	= ess
MAN	= ess.1

all: ess

ess:
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

strip: ess
	strip $(EXEC)

install:
	if [ ! -d $(PREFIX)/bin ] ; then \
		mkdir -p $(PREFIX)/bin; \
	fi
	install -c -m 755 $(EXEC) $(PREFIX)/bin/$(EXEC)
	if [ ! -d $(PREFIX)/man/man1 ] ; then \
		mkdir -p $(PREFIX)/man/man1; \
	fi
	install -c -m 644 $(MAN) $(PREFIX)/man/man1/$(MAN)

clean:
	rm -f *.core *~ $(EXEC)
