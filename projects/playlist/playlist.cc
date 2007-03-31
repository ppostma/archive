/* $Id: playlist.cc,v 1.21 2007-03-31 18:17:47 peter Exp $ */

/*
 * Copyright (c) 2003 Peter Postma <peter@pointless.nl>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
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
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#define TITLE			"My MP3 Playlist"
#define HILITE(s)		"<span style='color: #FFBF00;'>" + s + "</span>"
#define DIV_STATS		"<div style='color: #409FFF; font-size: 8pt;'>"
#define DIV_LISTFILES		"<div style='color: #FFBF00; font-size: 16pt;'>"
#define DIV_LIST		"<div style='text-align: left; margin-left: 10px;'>"
#define DIV_CLOSE		"</div>"
#define HR			"<hr style='height: 1px; border-bottom: none;' />"
#define BR			"<br />"

using namespace std;

string itos(int i);		// int to string
string time2texts(long time);
string time2textl(long time);

class PlaylistEntry {
  public:
	string track;
	string time;

	PlaylistEntry *next;
	PlaylistEntry() { next = NULL; }
};

class Playlist {
  private:
	PlaylistEntry *head, *tail;

  public:
	Playlist();
	~Playlist();

	bool Append(string track, string time);
	void Output();
	void PrintHeader();
	void PrintFooter();
};

Playlist::Playlist()
{
	head = tail = NULL;
}

Playlist::~Playlist()
{
	while (head != NULL) {
		tail = head;
		head = head->next;
		delete tail;
	}
}

bool Playlist::Append(string track, string time)
{
	if (tail != NULL) {
		tail->next = new PlaylistEntry;
		tail = tail->next;
	} else
		head = tail = new PlaylistEntry;

	if (tail == NULL) return false;

	tail->track = track;
	tail->time  = time;

	return true;
}

void Playlist::Output()
{
	PlaylistEntry *list = head;

	cout << DIV_LISTFILES << "Playlist files:" << DIV_CLOSE << endl;
	cout << HR << endl;
	cout << DIV_LIST << endl;

	for (long i=1; i; i++) {
		cout << i << ". " << list->track;
		if (list->time.size() != 0)
			cout << " (" << time2texts(atol(list->time.c_str())) << ")";
		cout << BR << endl;

		if (list->next == NULL) break;
		list = list->next;
	}

	cout << DIV_CLOSE << endl;
	cout << HR << endl;
}

void Playlist::PrintHeader()
{
	cout <<
"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n"
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" >\n"
"<head>\n"
"  <title>" << TITLE << "</title>\n"
"  <style type=\"text/css\">\n"
"  <!--\n"
"    body,td { background: #000040; font-family: Verdana, Helvetica, Arial; color: #FFFFFF; font-size: 10pt; }\n"
"  //-->\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"<div style='color: #FFBF00; font-size: 18pt;'><b>" << TITLE << "</b></div>\n"
"<div style='text-align: left; margin-left: 20px;'>\n"
"<br />\n";
}

void Playlist::PrintFooter()
{
	cout <<
"</div>\n"
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
	string	temp;
	int	hours, mins, secs;

	hours = time / 3600;
	time %= 3600;
	mins  = time / 60;
	secs  = time % 60;

	if (hours > 0)
		temp += itos(hours) + ":";

	if (hours > 0 && mins < 10)
		temp += "0";

	temp += itos(mins) + ":";

	if (secs < 10)
		temp += "0";
	
	temp += itos(secs);

	return temp;
}

string time2textl(long time)
{
	string	temp;
	int	days, hours, mins, secs;

	days  = time / 86400;
	time %= 86400;
	hours = time / 3600;
	time %= 3600;
	mins  = time / 60;
	secs  = time % 60;

	if (days > 0)
		temp += HILITE(itos(days)) + " day" + ((days > 1) ? "s" : "") + " ";
	if (hours > 0)
		temp += HILITE(itos(hours)) + " hour" + ((hours > 1) ? "s" : "") + " ";
	if (mins > 0)
		temp += HILITE(itos(mins)) + " minute" + ((mins > 1) ? "s" : "") + " ";
	if (secs > 0)
		temp += HILITE(itos(secs)) + " second" + ((secs > 1) ? "s" : "");

	return temp;
}

int main(int argc, char *argv[])
{
	string::size_type	pos1, pos2;
	string			temp, track, time;
	long			totaltime = 0;
	long			totaltracks = 0;
	long			unknownlength = 0;
	Playlist		List;

	if (argc != 2) {
		cerr << "Syntax: " << argv[0] << " <m3u file>" << endl;
		return 1;
	}

	ifstream input(argv[1], ios::in);
	if (!input) {
		cerr << "Cannot open file '" << argv[1] << "'." << endl;
		return 1;
	}

	getline(input, temp);
	if (temp.find("#EXTM3U") == string::npos) {
		cerr << "Not a m3u playlist file." << endl;
		input.close();
		return 1;
	}

	while (getline(input, temp)) {
		if (temp.empty())
			continue;

		// remove \r and \n from the string
		pos1 = temp.find("\r");
		if (pos1 != string::npos)
			temp.erase(pos1);
		pos1 = temp.find("\n");
		if (pos1 != string::npos)
			temp.erase(pos1);

		// extended info?
		if (temp.find("#EXTINF") != string::npos) {
			pos1 = temp.find_first_of(":");
			if (pos1 == string::npos)
				continue;

			pos2 = temp.find_first_of(",");
			if (pos2 == string::npos)
				continue;

			time  = temp.substr(pos1+1, (pos2 - pos1) - 1);
			if (atol(time.c_str()) < 0)   // Ignore negative values
				time.erase();

			track = temp.substr(pos2+1, temp.length());

			// skip next line, we don't need the path info here
			getline(input, temp);

		} else {
			time.erase();

			pos1 = temp.find_last_of("/");		// Unix
			if (pos1 == string::npos) {
				pos1 = temp.find_last_of("\\");	// Windows
				if (pos1 == string::npos)
					pos1 = 0;		// No Path
			}

			pos2 = temp.find(".mp3");
			if (pos2 == string::npos)
				track = temp.substr(pos1+1, temp.length());
			else
				track = temp.substr(pos1+1, (pos2 - pos1) - 1);
		}

		if (List.Append(track, time) == false) {
			cerr << "Fatal: Can't reserve memory!" << endl;
			input.close();
			return 1;
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
		return 1;
	}

	List.PrintHeader();

	cout << DIV_STATS;
	cout << HILITE(itos(totaltracks)) << " tracks in playlist, average track length: ";
	if (totaltime > 0)
		cout << HILITE(time2texts(totaltime / (totaltracks - unknownlength)));
	cout << BR << endl;

	if (unknownlength > 0) {
		cout << HILITE(itos(unknownlength));
		cout << " track" << ((unknownlength > 1) ? "s" : "");
		cout << " of unknown length" << BR << endl;
	}

	cout << "Playlist length: " << time2textl(totaltime) << endl;
	cout << DIV_CLOSE << endl;
	cout << BR << endl;

	List.Output();
	List.PrintFooter();

	return 0;
}
