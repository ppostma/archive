# $Id: di.tcl,v 1.2 2003-07-03 14:51:10 peter Exp $

# Digitally Imported Radio info script for the eggdrop
# version 0.2, 03/07/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 0.2: (03/07/03)
#  - added several configuration options:
#  - configurable flags
#  - nopub, where the bot doesn't speak
#  - configurable trigger(s)
#  - flood protection
#  - method to send message
#  - configurable layout
#  - added changelog & version bumped to 0.2
# 0.1: (30/06/03)
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
# %b          = bold text
# %u          = underlined text
set di(layout) "\[%bDI%b %chantitle\] %tracktitle, playing since %since"

### End Configuration settings ###


### Begin TCL code ###

package require http

set di(version) 0.2

if {[info tclversion] < 8.2} {
  putlog "Cannot load [file tail [info script]]: You need at least TCL version 8.2 and you have TCL version [info tclversion]."
  return 1
}

for {set i 0} {$i < [llength $di(triggers)]} {incr i} {
  bind pub $di(flags) [lindex $di(triggers) $i] pub:di
}
catch { unset i }

proc pub:di {nick uhost hand chan text} {
  global di lastbind

  if {[lsearch -exact $di(nopub) [string tolower $chan]] >= 0} { return 0 }

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
    regsub -all "%since"   $outchan [clock format $starttime -format %H:%M:%S] outchan
    regsub -all "%b" $outchan "\002" outchan
    regsub -all "%u" $outchan "\037" outchan
    switch -- $di(method) {
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
