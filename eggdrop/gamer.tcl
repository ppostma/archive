# $Id: gamer.tcl,v 1.1.1.1 2003-03-19 14:50:33 peter Exp $

# gamer.tcl / gamer.nl Nieuws script voor een eggdrop
# version 1.5 / 13/10/2002 / door Peter Postma <peter@webdeveloping.nl>
#
# Changelog: zelfde script als tweakers.tcl, dit script is van versie 1.5 
#            geconverteerd naar gamer.tcl.
#
# Dit script heeft alltools.tcl nodig! Zorg dat deze is geladen in het
# eggdrop configuratie bestand. Dit script gebruikt ook http.tcl.
# Oudere TCL versies kunnen deze nog niet (goed) gebruiken.
# U heeft dus minimaal TCL versie 8.2 ? nodig.
#
# Voor vragen/suggesties/bugs/etc: peter@webdeveloping.nl
#
# Pas aub. de configuratie hieronder aan:

### Configuratie instellingen ###

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set gmr_flag "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set gmr_nopub "" 

# de triggers: [scheiden met een spatie]
set gmr_triggers "!gamer"

# stuur berichten public of private wanneer er een trigger wordt gebruikt?
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set gmr_method 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1] 
set gmr_headlines 2

# hieronder kun je de layout aanpassen:
# %tyd = tijd
# %tit = titel
# %aut = auteur / schrijver
# %id  = id (wordt gebruikt in de nieuws url)
# %rea = aantal reacties
# %b   = bold (dikgedrukte) tekst
# %u   = underlined (onderstreepte) tekst
set gmr_layout "\[%bGamer.nl%b\] %tit - http://gamer.nl/nieuws/%id"

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set gmr_autonews 1

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
set gmr_autonews_chan "#HDM #n00b"

# om de hoeveel seconden checken of er nieuws is? (zet dit niet te laag, 
# het zal load/verkeer op de servers vergroten, 300 is wel ok).
set gmr_updates 900

# maximaal aantal berichten die worden getoond tijdens de automatische updates.
# hiermee kan je voorkomen dat de channel wordt ondergeflood als je de 
# updates hoog hebt staan (bv. langer dan een uur).
set gmr_automax 3

# trigger om het autonews aan te zetten. [string]
set gmr_auton_trig "!gameron"

# trigger om het autonews uit te zetten. [string]
set gmr_autoff_trig "!gameroff"

# benodigde flags om de autonews[on/off] triggers te gebruiken [default=master]
set gmr_auto_trig_flag "m|m"

# log extra informatie (debug) naar de partyline? [0=nee / 1=ja]
set gmr_log 1

### Eind configuratie instellingen ###



### Begin TCL code ###

set gmr_version "1.5"

package require http

for {set i 0} {$i < [llength $gmr_triggers]} {incr i} {
  bind pub $gmr_flag [lindex $gmr_triggers $i] gmr:pub
  if {$gmr_log} { putlog "\[Gamer.nl\] Trigger [lindex $gmr_triggers $i] added." }
}

bind pub $gmr_auto_trig_flag $gmr_autoff_trig gmr:autoff
bind pub $gmr_auto_trig_flag $gmr_auton_trig gmr:auton

proc gmr:getdata {} {
  global gmr_id gmr_titel gmr_tijd gmr_auteur gmr_reac gmr_log gmr_ts

  if {$gmr_log} { putlog "\[Gamer.nl\] Updating data..." }

  set url "http://www.gamer.nl/newstracker.xml"
  set page [::http::config -useragent "Mozilla"]
  
  # check of connecten goed gaat
  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[Gamer.nl\] Problem! Error: $msg"
    return -1
  }

  # dit is voor errors zoals 'timeout'..
  if {[::http::status $page] != "ok"} {
    putlog "\[Gamer.nl\] Problem: [::http::status $page]"
    return -1
  }

  # dit is voor errors zoals 404 etc..
  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Gamer.nl\] Problem: [::http::code $page]"
    return -1
  }
 
  set lines [split [::http::data $page] \n]
  set count 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regexp "<id>(.*?)</id>" $line trash gmr_id($count)
    regexp "<title>(.*?)</title>" $line trash gmr_titel($count)
    regexp "<author>(.*?)</author>" $line trash gmr_auteur($count)
    regexp "<time>(.*?)</time>" $line trash gmr_tijd($count)
    regexp "<unix_timestamp>(.*?)</unix_timestamp>" $line trash gmr_ts($count)
    if {[regexp "<comments>(.*?)</comments>" $line trash gmr_reac($count)]} { incr count }
  }
  return 0
}

proc gmr:pub {nick uhost hand chan text} {
  global lastbind gmr_log gmr_nopub gmr_headlines gmr_method
  if {[lsearch -exact $gmr_nopub [string tolower $chan]] >= 0} { return 0 }  

  if {$gmr_log} { putlog "\[Gamer.nl\] Trigger: $lastbind in $chan by $nick" }

  if {[gmr:getdata] != -1} {
    for {set i 0} {$i < $gmr_headlines} {incr i} { gmr:put $chan $nick $i $gmr_method }
  } else {
    putserv "NOTICE $nick :\[Gamer.nl\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
}

proc gmr:put {chan nick which method} {
  global gmr_id gmr_titel gmr_reac gmr_auteur gmr_tijd gmr_layout
  set outchan $gmr_layout
  regsub -all "%tyd" $outchan $gmr_tijd($which) outchan
  regsub -all "%id"  $outchan $gmr_id($which) outchan
  regsub -all "%rea" $outchan $gmr_reac($which) outchan
  regsub -all "%aut" $outchan $gmr_auteur($which) outchan
  regsub -all "%tit" $outchan $gmr_titel($which) outchan
  # waarom TCL er %titamp; van maakt weet ik niet, maar zo los ik het iig op:
  regsub -all "%titamp;" $outchan "\\\&" outchan
  regsub -all "&amp;" $outchan "\\\&" outchan
  regsub -all "%b"   $outchan "\002" outchan
  regsub -all "%u"   $outchan "\037" outchan
  switch -- $method {
    0 { putserv "PRIVMSG $nick :$outchan" }
    1 { putserv "PRIVMSG $chan :$outchan" }
    2 { putserv "NOTICE $nick :$outchan" }
    3 { putserv "NOTICE $chan :$outchan" }
    default { putserv "PRIVMSG $chan :$outchan" }
  }
}

proc gmr:update {} {
  global gmr_ts gmr_updates gmr_lastitem gmr_log gmr_autonews_chan gmr_automax

  if {[gmr:getdata] != -1} {

    if {![info exists gmr_lastitem]} { 
      set gmr_lastitem $gmr_ts(0) 
      if {$gmr_log} { putlog "\[Gamer.nl\] Last news item timestamp set to $gmr_ts(0)" }
    } else {
      if {$gmr_log} { putlog "\[Gamer.nl\] Last news item timestamp is $gmr_ts(0)" }
    }

    if {$gmr_ts(0) > $gmr_lastitem} {
      if {$gmr_log} { putlog "\[Gamer.nl\] There's news!" }
      for {set i 0} {$i < $gmr_automax} {incr i} {
        if {$gmr_ts($i) == $gmr_lastitem} { break }
        foreach chan [split $gmr_autonews_chan] { gmr:put $chan $chan $i 1 }
      }
    } else {
      if {$gmr_log} { putlog "\[Gamer.nl\] No news." } 
    }

    set gmr_lastitem $gmr_ts(0)
  }

  if {$gmr_updates < 300} {
    utimer 300 gmr:update
  } else {
    utimer $gmr_updates gmr:update
  }
}

proc gmr:autoff {nick uhost hand chan text} {
  global lastbind gmr_log gmr_nopub gmr_autonews gmr_lastitem
  if {[lsearch -exact $gmr_nopub [string tolower $chan]] >= 0} { return 0 }

  if {$gmr_log} { putlog "\[Gamer.nl\] Trigger: $lastbind in $chan by $nick" }

  if {$gmr_autonews == 1} {
    set gmr_autonews 0;  unset gmr_lastitem
    set killtimer [utimerexists "gmr:update"]
    if {$killtimer != ""} { killutimer $killtimer }
    putlog "\[Gamer.nl\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn gamer.nl nieuws aankondiger uitgezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn gamer.nl nieuws aankondiger staat al uit!"
  }
}

proc gmr:auton {nick uhost hand chan text} {
  global lastbind gmr_log gmr_nopub gmr_autonews gmr_lastitem
  if {[lsearch -exact $gmr_nopub [string tolower $chan]] >= 0} { return 0 }

  if {$gmr_log} { putlog "\[Gamer.nl\] Trigger: $lastbind in $chan by $nick" }

  if {$gmr_autonews == 0} {
    set gmr_autonews 1;  gmr:update
    putlog "\[Gamer.nl\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn gamer.nl nieuws aankondiger aangezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn gamer.nl nieuws aankondiger staat al aan!"
  }
}

set killtimer [utimerexists "gmr:update"]
if {$killtimer != ""} { killutimer $killtimer }

if {$gmr_autonews == 1} { gmr:update }

putlog "\[Gamer.nl\] Nieuws script versie $gmr_version: Loaded!"
