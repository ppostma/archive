# $Id: di.tcl,v 1.8 2003-08-02 14:21:05 peter Exp $

# Digitally Imported Radio info script for the eggdrop
# version 0.3, 10/07/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 0.2: (04/07/2003)
#  - added several configuration options:
#  - configurable flags
#  - nopub, where the bot doesn't speak
#  - configurable trigger(s)
#  - flood protection
#  - method to send message
#  - configurable layout
#  - added changelog & version bumped to 0.2
# 0.1: (30/06/2003)
#  - first version 
#

### Configuration settings ###

# flags needed to use the trigger [default=everyone]
set di(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set di(nopub) "" 

# the triggers: [seperate with spaces]
set di(triggers) "!di"

# flood protection: seconds between use of the triggers
# to disable: set it to 0
set di(antiflood) 10

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set di(method) 1

# below you can change the layout of the output:
# %chantitle  = channel title
# %tracktitle = track title
# %since      = shows when the track started to play
# %plsurl     = url to the playlist (128k winamp stream)
# %b          = bold text
# %u          = underlined text
set di(layout) "\[%bDI%b %chantitle\] %tracktitle, playing since %since"

### End Configuration settings ###


### Begin Tcl code ###

package require http

set di(version) 0.3

if {[info tclversion] < 8.1} {
  putlog "Cannot load [file tail [info script]]: You need at least Tcl version 8.1 and you have Tcl version [info tclversion]."
  return 1
}

foreach trigger [split $di(triggers)] { bind pub $di(flags) $trigger pub:di }
catch { unset trigger }

proc pub:di {nick uhost hand chan text} {
  global di lastbind

  if {[lsearch -exact $di(nopub) [string tolower $chan]] >= 0} { return 0 }

  switch [string tolower [lindex $text 0]] {
    "vocaltrance" {
      set chantitle "VocalTrance"
      set plsurl "http://di.fm/mp3/vocaltrance128k.pls"
    }
    "hardtrance" {
      set chantitle "HardTrance"
      set plsurl "http://di.fm/mp3/hardtrance128k.pls"
    }
    "trance" {
      set chantitle "Trance"
      set plsurl "http://di.fm/mp3/trance128k.pls"
    }
    "hardhouse" {
      set chantitle "HardHouse"
      set plsurl "http://di.fm/mp3/hardhouse128k.pls"
    }
    "hardcore" {
      set chantitle "Hardcore"
      set plsurl "http://di.fm/mp3/hardcore128k.pls"
    }
    "goa" {
      set chantitle "Goa"
      set plsurl "http://di.fm/mp3/goapsy128k.pls"
    }
    "eurodance" {
      set chantitle "EuroDance"
      set plsurl "http://di.fm/mp3/eurodance128k.pls"
    }
    "djmixes" {
      set chantitle "DJMixes"
      set plsurl "http://di.fm/mp3/djmixes128k.pls"
    }
    "deephouse" {
      set chantitle "DeepHouse"
      set plsurl "http://di.fm/mp3/deephouse128k.pls"
    }
    "classictechno" {
      set chantitle "ClassicTechno"
      set plsurl "http://di.fm/mp3/classictechno128k.pls"
    }
    "jazz" {
      set chantitle "Jazz"
      set plsurl "http://di.fm/mp3/jazz128k.pls"
    }
    "classical" {
      set chantitle "Classical"
      set plsurl "http://www.mostlyclassical.com/mp3/classical128k.pls"
    }
    "chillout" {
      set chantitle "Chillout"
      set plsurl "http://di.fm/mp3/chillout128k.pls"
    }
    default {
      putquick "NOTICE $nick :Usage: $lastbind <channel_title>"
      putquick "NOTICE $nick :Available channel titles are: VocalTrance, HardTrance, Trance, HardHouse, Hardcore, Goa, EuroDance, DJMixes, DeepHouse, ClassicTechno, Jazz, Classical, Chillout"
      return 0
    }
  }

  if {[info exists di(floodprot)]} {
    set diff [expr [clock seconds] - $di(floodprot)]
    if {$diff < $di(antiflood)} {
      putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $di(antiflood) - $diff] seconds..."
      return 0
    }
    catch { unset diff }
  }
  set di(floodprot) [clock seconds]

  set di(url) "http://www.di.fm/partners/xml/playlists.xml"
  if {[catch {set di(page) [http::geturl $di(url) -timeout 15000]} msg]} {
    putquick "NOTICE $nick :Can't connect ($msg)"
    return 0
  }
  set di(data) [http::data $di(page)]

  if {[regexp -nocase "<CHANNELTITLE>$chantitle</CHANNELTITLE>.*?<TRACKTITLE>(.+)</TRACKTITLE>.*?<STARTTIME>(.+)</STARTTIME>" $di(data) t tracktitle starttime]} {
    set outchan $di(layout)
    regsub -all "%chantitle"  $outchan $chantitle outchan
    regsub -all "%tracktitle" $outchan $tracktitle outchan
    regsub -all "%plsurl" $outchan $plsurl outchan
    regsub -all "%since" $outchan [clock format $starttime -format %H:%M:%S] outchan
    regsub -all "%b" $outchan "\002" outchan
    regsub -all "%u" $outchan "\037" outchan
    switch $di(method) {
      0 { putquick "PRIVMSG $nick :$outchan" }
      1 { putquick "PRIVMSG $chan :$outchan" }
      2 { putquick "NOTICE $nick :$outchan" }
      3 { putquick "NOTICE $chan :$outchan" }
      default { putquick "PRIVMSG $chan :$outchan" }
    }
    catch { unset outchan }
  } else {
    putquick "NOTICE $nick :Unable to parse data :("
  }

  catch { http::cleanup $di(page) }
  return 0
}

putlog "Digitally Imported Radio info script $di(version) loaded!"
