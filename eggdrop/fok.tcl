# $Id: fok.tcl,v 1.1.1.1 2003-03-19 14:50:33 peter Exp $

# fok.tcl / fok.nl Nieuws script voor een eggdrop
# version 1.5 / 12/10/2002 / door Peter Postma <peter@webdeveloping.nl>
#
# Changelog: zelfde als tweakers.tcl
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
set fok_flag "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set fok_nopub "" 

# de triggers: [scheiden met een spatie]
set fok_triggers "!fok fok!"

# stuur berichten public of private wanneer er een trigger wordt gebruikt?
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set fok_method 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1] 
set fok_headlines 2

# hieronder kun je de layout aanpassen:
# %tyd = tijd
# %tit = titel
# %id  = id (wordt gebruikt in de nieuws url)
# %rea = aantal reacties
# %b   = bold (dikgedrukte) tekst
# %u   = underlined (onderstreepte) tekst
set fok_layout "\[%bFok!%b\] %tit - http://fok.nl/?id=%id"

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set fok_autonews 1

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
set fok_autonews_chan "#kanaal1 #kanaal2"

# om de hoeveel seconden checken of er nieuws is? (zet dit niet te laag, 
# het zal load/verkeer op de servers vergroten, 300 is wel ok).
set fok_updates 600

# maximaal aantal berichten die worden getoond tijdens de automatische updates.
# hiermee kan je voorkomen dat de channel wordt ondergeflood als je de 
# updates hoog hebt staan (bv. langer dan een uur).
set fok_automax 3

# trigger om het autonews aan te zetten. [string]
set fok_auton_trig "!fokon"

# trigger om het autonews uit te zetten. [string]
set fok_autoff_trig "!fokoff"

# benodigde flags om de autonews[on/off] triggers te gebruiken [default=master]
set fok_auto_trig_flag "m|m"

# log extra informatie (debug) naar de partyline? [0=nee / 1=ja]
set fok_log 1

### Eind configuratie instellingen ###



### Begin TCL code ###

set fok_version "1.5"

package require http

for {set i 0} {$i < [llength $fok_triggers]} {incr i} {
  bind pub $fok_flag [lindex $fok_triggers $i] fok:pub
  if {$fok_log} { putlog "\[Fok!\] Trigger [lindex $fok_triggers $i] added." }
}

bind pub $fok_auto_trig_flag $fok_autoff_trig fok:autoff
bind pub $fok_auto_trig_flag $fok_auton_trig fok:auton

proc fok:getdata {} {
  global fok_id fok_titel fok_tijd fok_reac fok_log fok_ts

  if {$fok_log} { putlog "\[Fok!\] Updating data..." }

  set url "http://www.athena.fokzine.net/~danny/remote.xml"
  set page [::http::config -useragent "Mozilla"]
  
  # check of connecten goed gaat
  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[Fok!\] Problem! Error: $msg"
    return -1
  }

  # dit is voor errors zoals 'timeout'..
  if {[::http::status $page] != "ok"} {
    putlog "\[Fok!\] Problem: [::http::status $page]"
    return -1
  }

  # dit is voor errors zoals 404 etc..
  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Fok!\] Problem: [::http::code $page]"
    return -1
  }
 
  set lines [split [::http::data $page] \n]
  set count 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regexp "<id>(.*?)</id>" $line trash fok_id($count)
    regexp "<titel>(.*?)</titel>" $line trash fok_titel($count)
    regexp "<time>(.*?)</time>" $line trash fok_tijd($count)
    regexp "<timestamp>(.*?)</timestamp>" $line trash fok_ts($count)
    if {[regexp "<reacties>(.*?)</reacties>" $line trash fok_reac($count)]} { incr count }
  }
  return 0
}

proc fok:pub {nick uhost hand chan text} {
  global lastbind fok_log fok_nopub fok_headlines fok_method
  if {[lsearch -exact $fok_nopub [string tolower $chan]] >= 0} { return 0 }  

  if {$fok_log} { putlog "\[Fok!\] Trigger: $lastbind in $chan by $nick" }

  if {[fok:getdata] != -1} {
    for {set i 0} {$i < $fok_headlines} {incr i} { fok:put $chan $nick $i $fok_method }
  } else {
    putserv "NOTICE $nick :\[Fok!\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
}

proc fok:put {chan nick which method} {
  global fok_id fok_titel fok_reac fok_tijd fok_layout
  set outchan $fok_layout
  regsub -all "%tyd" $outchan $fok_tijd($which) outchan
  regsub -all "%id"  $outchan $fok_id($which) outchan
  regsub -all "%rea" $outchan $fok_reac($which) outchan
  regsub -all "%tit" $outchan $fok_titel($which) outchan
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

proc fok:update {} {
  global fok_ts fok_updates fok_lastitem fok_log fok_autonews_chan fok_automax

  if {[fok:getdata] != -1} {

    if {![info exists fok_lastitem]} { 
      set fok_lastitem $fok_ts(0) 
      if {$fok_log} { putlog "\[Fok!\] Last news item timestamp set to $fok_ts(0)" }
    } else {
      if {$fok_log} { putlog "\[Fok!\] Last news item timestamp is $fok_ts(0)" }
    }

    if {$fok_ts(0) > $fok_lastitem} {
      if {$fok_log} { putlog "\[Fok!\] There's news!" }
      for {set i 0} {$i < $fok_automax} {incr i} {
        if {$fok_ts($i) == $fok_lastitem} { break }
        foreach chan [split $fok_autonews_chan] { fok:put $chan $chan $i 1 }
      }
    } else {
      if {$fok_log} { putlog "\[Fok!\] No news." } 
    }

    set fok_lastitem $fok_ts(0)
  }

  if {$fok_updates < 300} {
    utimer 300 fok:update
  } else {
    utimer $fok_updates fok:update
  }
}

proc fok:autoff {nick uhost hand chan text} {
  global lastbind fok_log fok_nopub fok_autonews fok_lastitem
  if {[lsearch -exact $fok_nopub [string tolower $chan]] >= 0} { return 0 }

  if {$fok_log} { putlog "\[Fok!\] Trigger: $lastbind in $chan by $nick" }

  if {$fok_autonews == 1} {
    set fok_autonews 0;  unset fok_lastitem
    set killtimer [utimerexists "fok:update"]
    if {$killtimer != ""} { killutimer $killtimer }
    putlog "\[Fok!\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn fok.nl nieuws aankondiger uitgezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn fok.nl nieuws aankondiger staat al uit!"
  }
}

proc fok:auton {nick uhost hand chan text} {
  global lastbind fok_log fok_nopub fok_autonews fok_lastitem
  if {[lsearch -exact $fok_nopub [string tolower $chan]] >= 0} { return 0 }

  if {$fok_log} { putlog "\[Fok!\] Trigger: $lastbind in $chan by $nick" }

  if {$fok_autonews == 0} {
    set fok_autonews 1;  fok:update
    putlog "\[Fok!\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn fok.nl nieuws aankondiger aangezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn fok.nl nieuws aankondiger staat al aan!"
  }
}

set killtimer [utimerexists "fok:update"]
if {$killtimer != ""} { killutimer $killtimer }

if {$fok_autonews == 1} { fok:update }

putlog "\[Fok!\] Nieuws script versie $fok_version: Loaded!"
