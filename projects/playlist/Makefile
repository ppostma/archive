# $Id: Makefile,v 1.3 2003-07-14 10:09:18 peter Exp $

default:
	g++ -Wall -Wno-deprecated -O2 -o playlist playlist.cc
	strip playlist

install:
	install -c -m 755 playlist /usr/local/bin/playlist

clean:
	rm -f *~ *.core playlist
