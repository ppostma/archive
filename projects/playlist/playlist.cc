/*
 * Copyright (c) 2003 Peter Postma <peter@webdeveloping.nl>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: playlist.cc,v 1.16 2003-10-13 03:10:09 peter Exp $
 */

#include <stdlib.h>

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

string itos(int i);
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

	return true;
}

bool Playlist::Append(string track, string time)
{
	PlaylistEntry *tail = head;

	if (tail == NULL) return Insert(track, time);

	while (tail->next != NULL) tail = tail->next;

	tail->next = new PlaylistEntry;
	if (tail->next == NULL) return false;

	tail->next->track = track;
	tail->next->time  = time;

	return true;
}

void Playlist::Output()
{
	PlaylistEntry *tail = head;

	cout << DIV_LISTFILES << "Playlist files:" << DIV_CLOSE << endl;
	cout << HR << endl;
	cout << DIV_LIST << endl;

	for (long i=1; i; i++) {
		cout << i << ". " << tail->track;
		if (tail->time.size() != 0)
			cout << " (" << time2texts(atol(tail->time.c_str())) << ")";
		cout << BR << endl;

		if (tail->next == NULL) break;
		tail = tail->next;
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
		exit(1);
	}

	ifstream input(argv[1], ios::in);
	if (!input) {
		cerr << "Cannot open file '" << argv[1] << "'." << endl;
		exit(1);
	}

	getline(input, temp);
	if (temp.find("#EXTM3U") == string::npos) {
		cerr << "Not a m3u playlist file." << endl;
		input.close();
		exit(1);
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
