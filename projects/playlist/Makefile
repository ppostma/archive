# $Id: Makefile,v 1.2 2003-05-13 16:02:05 peter Exp $

default:
	g++ -Wall -Wno-deprecated -ansi -pedantic -O2 -o playlist playlist.cc
	strip playlist

install:
	install -c -m 755 playlist /usr/local/bin/playlist

clean:
	rm -f *~ *.core playlist
