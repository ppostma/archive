# $Id: Makefile,v 1.1 2003-03-21 01:27:53 peter Exp $

#CC	= cc
CC	= gcc
CFLAGS	= -Wall -O

all:
	$(CC) $(CFLAGS) -o scan scan.c
	$(CC) $(CFLAGS) -o scan6 scan6.c
	$(CC) $(CFLAGS) -o banner banner.c
	$(CC) $(CFLAGS) -o banner6 banner6.c
	strip scan
	strip scan6
	strip banner
	strip banner6

clean:
	rm -f scan scan6 banner banner6
