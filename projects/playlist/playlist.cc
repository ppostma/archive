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
 * $Id: playlist.cc,v 1.5 2003-05-12 01:31:41 peter Exp $
 */

#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

class PlaylistEntry {
  public:
	string track;
	string time;

	PlaylistEntry *next;
	PlaylistEntry() { next = NULL; }
};

class Playlist {
  private:
	PlaylistEntry *head;

  public:
	Playlist();
	~Playlist();

	bool Append(string track, string time);
	bool Insert(string track, string time);
	void Output();
	void PrintHeader();
	void PrintFooter();
};

Playlist::Playlist()
{
	head = NULL;
}

Playlist::~Playlist()
{
	PlaylistEntry *tail;

	while (head != NULL) {
		tail = head;
		head = head->next;
		delete tail;
	}
}

bool Playlist::Insert(string track, string time)
{
	PlaylistEntry *tail;
	tail = new PlaylistEntry;

	if (tail == NULL) return false;

	tail->track = track;
	tail->time  = time;
	tail->next  = head;
	head = tail;

	return (true);
}

bool Playlist::Append(string track, string time)
{
	PlaylistEntry *tail = head;

	if (tail == NULL) return Insert(track, time);

	while (tail->next != NULL) tail = tail->next;

	tail->next = new PlaylistEntry;
	if (tail->next == NULL) return (false);

	tail->next->track = track;
	tail->next->time  = time;

	return (true);
}

void Playlist::Output()
{
	PlaylistEntry *tail = head;

	for (long i=1; i; i++) {
		cout << i << ". " << tail->track;
		if (tail->time.size() != 0)
			cout << " (" << tail->time << ")";
		cout << "<br />" << endl;

		if (tail->next == NULL) break;
		tail = tail->next;
	}
}

void Playlist::PrintHeader()
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
"  //-->\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"<div style='color: #FFBF00; font-size: 14pt;'>\n"
"<b>My Playlist</b>\n"
"</div>\n"
"<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\n"
"<tr>\n"
"<td style='width: 20px;'></td>\n"
"<td>\n";
}

void Playlist::PrintFooter()
{
	cout <<
"</td>\n"
"</tr>\n"
"</table>\n"
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
	Playlist		List;

	if (argc != 2) {
		cerr << "Syntax: " << argv[0] << " <m3u file>" << endl;
		exit(1);
	}

	ifstream input(argv[1], ios::in);
	if (!input) {
		cerr << "Cannot open file '" << argv[1] << "'." << endl;
		exit(1);
	}

	getline(input, temp);
	if (temp != "#EXTM3U") {
		cerr << "Not a m3u playlist file." << endl;
		input.close();
		exit(1);
	}

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

			pos1 = temp.find_last_of("/");		// Unix
			if (pos1 == string::npos) {
				pos1 = temp.find_last_of("\\");	// Windows
				if (pos1 == string::npos)
					continue;
			}

			pos2 = temp.find(".mp3");
			if (pos2 == string::npos)
				track = temp.substr(pos1+1, temp.length());
			else
				track = temp.substr(pos1+1, (pos2 - pos1) - 1);
		}

		if (List.Append(track, time2texts(atol(time.c_str()))) == false) {
			cerr << "Can't reserve memory!" << endl;
			input.close();
			exit(1);
		}

		if (time.size() != 0)
			totaltime += atol(time.c_str());
		else
			unknownlength++;

		totaltracks++;
	}

	input.close();

	if (totaltracks < 1) {
		cerr << "No tracks in the playlist." << endl;
		exit(1);
	}

	List.PrintHeader();

	cout << "<p>\n";
	cout << totaltracks << " tracks in playlist, average track length: ";
	cout << time2texts(totaltime / (totaltracks - unknownlength));
	cout << "<br />" << endl;

	if (unknownlength > 0) {
		cout << unknownlength << " track";
		cout << ((unknownlength > 1) ? "s" : "");
		cout << " of unknown length" << "<br />" << endl;
	}

	cout << "Playlist length: " << time2textl(totaltime) << endl;
	cout << "</p>\n";

	List.Output();
	List.PrintFooter();

	return (0);
}
