# $Id: tv.tcl,v 1.5 2003-07-04 10:57:06 peter Exp $

# TV gids script voor eggdrop
# version 0.4, 04/07/2003, by Peter Postma <peter@webdeveloping.nl>

### Configuration settings ###

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set tv(flags) "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set tv(nopub) "" 

# de triggers: [scheiden met een spatie]
set tv(triggers) "!tv"

# flood protectie: aantal seconden tussen gebruik van de triggers
# voor geen flood protectie: zet 't op 0
set tv(antiflood) 10

# stuur berichten public of private wanneer er een trigger wordt gebruikt? 
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set tv(method) 1

# hieronder kun je de layout aanpassen:
# %channel = TV kanaal
# %program = Programma
# %time    = Begin tijd
# %b       = bold (dikgedrukte) text
# %u       = underlined (onderstreepte) text
set tv(layout) "%channel: %time - %program"

### End Configuration settings ###



### Begin TCL code ###

package require http

set tv(version) 0.4

if {[info tclversion] < 8.2} {
  putlog "Cannot load [file tail [info script]]: You need at least TCL version 8.2 and you have TCL version [info tclversion]."
  return 1
}

for {set i 0} {$i < [llength $tv(triggers)]} {incr i} {
  bind pub $tv(flags) [lindex $tv(triggers) $i] tv:pub
}

proc tv:pub {nick uhost hand chan text} {
  global tv lastbind

  if {[lsearch -exact $tv(nopub) [string tolower $chan]] >= 0} { return 0 }

  switch [string tolower [lindex $text 0]] {
    "nl1"      { set input "Nederland 1" }
    "nl2"      { set input "Nederland 2" }
    "nl3"      { set input "Nederland 3" }
    "rtl4"     { set input "RTL 4" }
    "rtl5"     { set input "RTL 5" }
    "sbs6"     { set input "SBS 6" }
    "net5"     { set input "Net 5" }
    "veronica" { set input "Veronica" }
    "yorin"    { set input "Yorin" }
    "v8"       { set input "V8" }
    "bvn"      { set input "BVN" }
    "vrt"      { set input "VRT TV 1" }
    "ketnet"   { set input "KETNET/Canvas" }
    "vtm"      { set input "VTM" }
    "kanaal2"  { set input "Kanaal 2" }
    "vt4"      { set input "VT4" }
    "bbc1"     { set input "BBC 1" }
    "bbc2"     { set input "BBC 2" }
    "bbcworld" { set input "BBC World" }
    "ard"      { set input "ARD" }
    "zdf"      { set input "ZDF" }
    "ndr"      { set input "NDR Fernsehen" }
    "wdr"      { set input "WDR Fernsehen" }
    "3sat"     { set input "3Sat" }
    "sat1"     { set input "Sat 1" }
    "rtl"      { set input "RTL" }
    "pro7"     { set input "PRO 7" }
    "rtbf1"    { set input "RTBF La 1" }
    "rtbf2"    { set input "RTBF La 2" }
    "tv5"      { set input "TV 5" }
    "raiuno"   { set input "Rai Uno" }
    "trt"      { set input "TRT int." }
    "at5"      { set input "AT 5" }
    "regional" { set input "TV Regionaal" }
    "c+red"    { set input "Canal+ Rood" }
    "c+blue"   { set input "Canal+ Blauw" }
    "tmf"      { set input "TMF" }
    "mtv"      { set input "MTV" }
    "mezzo"    { set input "Mezzo" }
    "cartoon"  { set input "Cartoon Network" }
    "cnn"      { set input "CNN" }
    "discovery" { set input "Discovery Channel" }
    "eurosport" { set input "Eurosport" }
    "geograph" { set input "National Geographic" }
    "tcm"      { set input "TCM" }
    "animal"   { set input "Animal Planet" }
    "tve"      { set input "TV E" }
    default {
      putquick "NOTICE $nick :Gebruik: $lastbind <kanaal>" 
      putquick "NOTICE $nick :Beschikbare kanalen zijn: veel."
      return 0
    }
  }

  if {[info exists tv(floodprot)]} {
    set diff [expr [clock seconds] - $tv(floodprot)]
    if {$diff < $tv(antiflood)} {
      putquick "NOTICE $nick :Trigger is net al gebruikt! Wacht aub. [expr $tv(antiflood) - $diff] seconden..."
      return 0
    }
    catch { unset diff }
  }
  set tv(floodprot) [clock seconds]

  if {![info exists tv(last)]} { set tv(last) 0 }

  if {![info exists tv(data)] || [expr [clock seconds] - $tv(last)] > 60} {
    set tv(url) "http://www.tvgids.nl/"
    set tv(min) [clock format [clock seconds] -format %M]
    set tv(hour) [clock format [clock seconds] -format %H]
    set tv(date) [clock format [clock seconds] -format %Y%m%d] 
    append tv(url) "nustraks.php?iUur=$tv(hour)&iMinuut=$tv(min)&sDatum=$tv(date)&alle=TRUE"

    if {[catch {set tv(page) [http::geturl $tv(url) -timeout 15000]} msg]} {
      putlog "tv.tcl: $msg"
      putquick "NOTICE $nick :$msg"
      return 0
    }
    if {![regexp -nocase {ok} [http::code $tv(page)]]} {
      putlog "tv.tcl: [http::code $tv(page)]"
      putquick "NOTICE $nick :[http::code $tv(page)]"
      return 0
    }
    set tv(last) [clock seconds]
    set tv(data) [http::data $tv(page)]
    catch { http::cleanup $tv(page) }
  }

  if {[regexp -nocase "\"alleprogrammas\[0-9\]+\.html\">$input</a>.*?<strong>(.*?)<.*?></strong>.*?\"details_programma\">(.+)</a>" $tv(data) t time program]} {
    set outchan $tv(layout)
    regsub -all "%program" $outchan $program outchan
    regsub -all "%channel" $outchan $input outchan
    regsub -all "%time" $outchan $time outchan
    regsub -all "%b" $outchan "\002" outchan
    regsub -all "%u" $outchan "\037" outchan
  } else {
    set outchan "Geen uitzending."
  }

  switch -- $tv(method) {
    0 { putquick "PRIVMSG $nick :$outchan" }
    1 { putquick "PRIVMSG $chan :$outchan" }
    2 { putquick "NOTICE $nick :$outchan" }
    3 { putquick "NOTICE $chan :$outchan" }
    default { putquick "PRIVMSG $chan :$outchan" }
  }

  return 0
}

putlog "TV gids script $tv(version) loaded!"
