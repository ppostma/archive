# $Id: di.tcl,v 1.1 2003-07-01 15:01:40 peter Exp $

# Digitally Imported Radio info script for the eggdrop
# version 0.1, 30/06/2003, by Peter Postma <peter@webdeveloping.nl>

package require Tcl 8.2
package require http

bind pub -|- "!di" pub:di

proc pub:di {nick uhost hand chan text} {
  global lastbind

  switch [string tolower [lindex $text 0]] {
    "vocaltrance"   { set chantitle "VocalTrance" }
    "hardtrance"    { set chantitle "HardTrance" }
    "trance"        { set chantitle "Trance" }
    "hardhouse"     { set chantitle "HardHouse" }
    "hardcore"      { set chantitle "Hardcore" }
    "goa"           { set chantitle "Goa" }
    "eurodance"     { set chantitle "EuroDance " }
    "djmixes"       { set chantitle "DJMixes" }
    "deephouse"     { set chantitle "DeepHouse" }
    "classictechno" { set chantitle "ClassicTechno" }
    "jazz"          { set chantitle "Jazz" }
    "classical"     { set chantitle "Classical" }
    "chillout"      { set chantitle "Chillout" }
    default {
      putquick "NOTICE $nick :Usage: $lastbind <channel_title>"
      putquick "NOTICE $nick :Available channel titles are: VocalTrance, HardTrance, Trance, HardHouse, Hardcore, Goa, EuroDance, DJMixes, DeepHouse, ClassicTechno, Jazz, Classical, Chillout"
      return 0
    }
  }

  set di(url) "http://www.di.fm/partners/xml/playlists.xml"
  if {[catch {set di(page) [http::geturl $di(url) -timeout 15000]} msg]} {
    putquick "NOTICE $nick :Can't connect ($msg)"
    return 0
  }
  set di(data) [http::data $di(page)]

  if {[regexp -nocase "<CHANNELTITLE>$chantitle</CHANNELTITLE>.*?<TRACKTITLE>(.+)</TRACKTITLE>.*?<STARTTIME>(.+)</STARTTIME>" $di(data) t tracktitle starttime]} {
    putquick "PRIVMSG $chan :\[DI $chantitle\] $tracktitle, playing since [clock format $starttime -format %H:%M:%S]"
  }
  catch { http::cleanup $di(page) }
  return 0
}

putlog "Digitally Imported Radio info script 0.1 loaded!"
