/*
 * Copyright (c) 2003 Peter Postma <peter@webdeveloping.nl>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS `AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: playlist.cc,v 1.1 2003-05-09 16:50:12 peter Exp $
 */

#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

static void usage(char *argv0)
{
	cerr << "Syntax: " << argv0 << " <m3u file>" << endl;
	exit(1);
}

static void print_header(void)
{
	cout <<
"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" >\n"
"<head>\n"
"  <title>My Playlist</title>\n"
"  <style type=\"text/css\">\n"
"  <!--\n"
"    body,td { background: #000040; font-family: Verdana, Helvetica, Arial; color: #FFFFFF; font-size: 10pt; }\n"
"    table { margin: auto; }\n"
"  //-->\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"<div style='color: #FFBF00; font-size: 14pt;'>\n"
"<b>My Playlist</b>\n"
"</div>\n";
}

static void print_footer(void)
{
	cout <<
"</body>\n"
"</html>\n";
}

string itos(int i)
{
	stringstream	s;

	s << i;

	return s.str();
}

string time2texts(long time)
{
	string	temp = "";
	int	days, hours, mins, secs;

	days  = time / 86400;
	time %= 86400;
	hours = time / 3600;
	time %= 3600;
	mins  = time / 60;
	secs  = time % 60;

	if (days > 0)
		temp += itos(days) + ":";
	if (hours > 0)
		temp += itos(hours) + ":";

	temp += itos(mins) + ":";

	if (secs < 10)
		temp += "0";
	
	temp += itos(secs);

	return (temp);
}

string time2textl(long time)
{
	string	temp = "";
	int	days, hours, mins, secs;

	days  = time / 86400;
	time %= 86400;
	hours = time / 3600;
	time %= 3600;
	mins  = time / 60;
	secs  = time % 60;

	if (days > 0)
		temp += itos(days) + " day" + ((days > 1) ? "s" : "") + " ";
	if (hours > 0)
		temp += itos(hours) + " hour" + ((hours > 1) ? "s" : "") + " ";
	if (mins > 0)
		temp += itos(mins) + " minute" + ((mins > 1) ? "s" : "") + " ";
	if (secs > 0)
		temp += itos(secs) + " second" + ((secs > 1) ? "s" : "");

	return (temp);
}

int main(int argc, char *argv[])
{
	string::size_type	pos1, pos2;
	string			temp, track, time;
	long			totaltime = 0;
	long			totaltracks = 0;
	long			unknownlength = 0;
	bool			extinfo = false;

	if (argc != 2)
		usage(argv[0]);

	ifstream input(argv[1], ios::in);
	if (!input) {
		cerr << "Cannot open file '" << argv[1] << "'." << endl;
		return (1);
	}

	getline(input, temp);
	if (temp != "#EXTM3U") {
		cerr << "Not an m3u playlist file." << endl;
		return (1);
	}

	print_header();
	cout << "<p>\n";

	while (getline(input, temp)) {
		if (temp.empty())
			continue;

		if (extinfo == true) {
			extinfo = false;
			continue;
		}

		pos1 = temp.find("#EXTINF");
		if (pos1 == string::npos)
			extinfo = false;
		else
			extinfo = true;

		if (extinfo == true) {
			pos1 = temp.find_first_of(":");
			if (pos1 == string::npos)
				continue;

			pos2 = temp.find_first_of(",");
			if (pos2 == string::npos)
				continue;

			time  = temp.substr(pos1+1, (pos2 - pos1) - 1);
			track = temp.substr(pos2+1, temp.length());
		} else {
			time.erase();

			pos1 = temp.find_last_of("/");
			if (pos1 == string::npos)
				continue;

			pos2 = temp.find(".mp3");
			if (pos2 == string::npos)
				track = temp.substr(pos1+1, temp.length());
			else
				track = temp.substr(pos1+1, (pos2 - pos1) - 1);
		}

		if (time.size() != 0)
			totaltime += atol(time.c_str());
		else
			unknownlength++;

		totaltracks++;

		cout << totaltracks << ". " << track;
		if (time.size() != 0)
			cout << " (" << time2texts(atoi(time.c_str())) << ")";

		cout << "<br />" << endl;
	}

	input.close();

	cout << "</p>\n<p>\n";

	cout << totaltracks << " tracks in playlist, average track length: ";
	cout << time2texts(totaltime / (totaltracks - unknownlength)) << endl;

	if (unknownlength > 0) {
		cout << "<br />\n";
		cout << unknownlength << " track";
		cout << ((unknownlength > 1) ? "s" : "");
		cout << " of unknown length" << endl;
	}

	cout << "<br />\n";
	cout << "Playlist length: " << time2textl(totaltime) << endl;
	cout << "</p>\n";

	print_footer();

	return (0);
}
