# $Id: Makefile,v 1.4 2003-10-17 16:08:06 peter Exp $

all:
	g++ -Wall -Wno-deprecated -O2 -o playlist playlist.cc
	strip playlist

install:
	install -c -m 755 playlist /usr/local/bin/playlist

clean:
	rm -f *~ *.core playlist
