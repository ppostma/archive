# $Id: tweakers.tcl,v 1.1.1.1 2003-03-19 14:50:33 peter Exp $

# tweakers.tcl / Tweakers.net Nieuws script voor een eggdrop
# version 1.5 / 12/10/2002 / door Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.5: (12/10/02) [features/changes]
#  - extra check in de getdata procedure (voor HTTP errors zoals 404, etc..)
#  - triggers/functie toegevoegd om de aankondiger aan en uit te zetten.
#  - stukjes code netter en beter geschreven.
#  - meer gebruiksvriendelijke error en log berichten.
# 1.4a: (09/10/02) [bugfixes]
#  - bugje gefixed met het & teken. regsub zet &amp; om naar %titamp; 
#    ik heb geen idee waarom, maar de output is iig gefixed :-)  
#  - wat fouten gefixed in de getdata procedure. checks werken nu helemaal ok.
#  - timer opnieuw instellen wanneer er een http error is, is gefixed.
# 1.4: (08/10/02) [features/changes]
#  - methode om berichten te sturen is nu in te stellen.
#  - check op timestamp ipv. id ! (dit had ik eerder moeten bedenken >:) 
#  - extra layout opties toegevoegd.
#  - updates minimaal 300 seconden!
#  - checkt of tweakers.net down is. als dit niet wordt gecheckt dan
#    blijft het script hangen en moet de bot opnieuw gestart worden.
# 1.3: (06/10/02) [features/changes]
#  - triggers in te stellen.
#  - kanalen in te stellen waar de triggers gebruikt mogen worden.
#  - layout in te stellen.
#  - automatisch nieuws naar het kanaal sturen (kanalen in te stellen).
#  - automatische updates in te stellen (om de hoeveel seconden).
# 1.2: (03/10/02) [rewrote 1.1]
#  - helemaal opnieuw geschreven.
#  - betere code (regexp, regsub, http)
# 1.1: (21/08/02) [first version]
#  - eerste (goed werkende) versie. 
#  - script had alleen triggers, verder nog helemaal niets :-)
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
set tnet_flag "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set tnet_nopub "" 

# de triggers: [scheiden met een spatie]
set tnet_triggers "!tnet !tweakers"

# stuur berichten public of private wanneer er een trigger wordt gebruikt? 
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set tnet_method 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1] 
set tnet_headlines 2

# hieronder kun je de layout aanpassen:
# %tyd = tijd
# %cat = categorie
# %tit = titel
# %id  = id (wordt gebruikt in de nieuws url)
# %src = bron van het artikel
# %lnk = link naar de bron van het artikel
# %rea = aantal reacties
# %edt = editor, schrijver
# %b   = bold (dikgedrukte) tekst
# %u   = underlined (onderstreepte) tekst
set tnet_layout "\[%bT.Net%b\] (%cat) %tit - http://tweakers.net/nieuws/%id"

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set tnet_autonews 1

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
set tnet_autonews_chan "#kanaal1 #kanaal2"

# om de hoeveel seconden checken of er nieuws is? [minimaal 300]
# zet dit niet te laag, het zal load/verkeer op de servers vergroten.
set tnet_updates 600

# maximaal aantal berichten die worden getoond tijdens de automatische updates.
# hiermee kan je voorkomen dat de channel wordt ondergeflood als je de 
# updates hoog hebt staan (bv. langer dan een uur).
set tnet_automax 3

# trigger om het autonews aan te zetten. [string]
set tnet_auton_trig "!tneton"

# trigger om het autonews uit te zetten. [string]
set tnet_autoff_trig "!tnetoff"

# benodigde flags om de autonews[on/off] triggers te gebruiken [default=master]
set tnet_auto_trig_flag "m|m"

# log extra informatie (debug) naar de partyline? [0=nee / 1=ja]
set tnet_log 1

### Eind configuratie instellingen ###



### Begin TCL code ###

set tnet_version "1.5"

package require http

for {set i 0} {$i < [llength $tnet_triggers]} {incr i} {
  bind pub $tnet_flag [lindex $tnet_triggers $i] tnet:pub
  if {$tnet_log} { putlog "\[T.Net\] Trigger [lindex $tnet_triggers $i] added." }
}

bind pub $tnet_auto_trig_flag $tnet_autoff_trig tnet:autoff
bind pub $tnet_auto_trig_flag $tnet_auton_trig tnet:auton

proc tnet:getdata {} {
  global tnet_id tnet_titel tnet_cat tnet_src tnet_tijd tnet_link tnet_reac tnet_ts tnet_edit tnet_log

  if {$tnet_log} { putlog "\[T.Net\] Updating data..." }

  set url "http://www.tweakers.net/turbotracker.dsp"
  set page [::http::config -useragent "Mozilla"]

  # check of connecten goed gaat
  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[T.Net\] Problem: $msg"
    return -1
  }
  
  # dit is voor errors zoals 'timeout'.. 
  if {[::http::status $page] != "ok"} {
    putlog "\[T.Net\] Problem: [::http::status $page]"
    return -1 
  } 

  # dit is voor errors zoals 404 etc..
  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[T.Net\] Problem: [::http::code $page]"
    return -1
  }

  set lines [split [::http::data $page] \n]
  set count 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regexp "<id>(.*?)</id>" $line trash tnet_id($count)
    regexp "<titel>(.*?)</titel>" $line trash tnet_titel($count)
    regexp "<editor>(.*?)</editor>" $line trash tnet_edit($count)
    regexp "<categorie>(.*?)</categorie>" $line trash tnet_cat($count)
    regexp "<bron>(.*?)</bron>" $line trash tnet_src($count)
    regexp "<link>(.*?)</link>" $line trash tnet_link($count)
    regexp "<tijd>(.*?)</tijd>" $line trash tnet_tijd($count)
    regexp "<timestamp>(.*?)</timestamp>" $line trash tnet_ts($count)
    if {[regexp "<reacties>(.*?)</reacties>" $line trash tnet_reac($count)]} { incr count }
  }
  return 0
}

proc tnet:pub {nick uhost hand chan text} {
  global lastbind tnet_log tnet_nopub tnet_headlines tnet_method
  if {[lsearch -exact $tnet_nopub [string tolower $chan]] >= 0} { return 0 }  

  if {$tnet_log} { putlog "\[T.Net\] Trigger: $lastbind in $chan by $nick" }

  if {[tnet:getdata] != -1} {
    for {set i 0} {$i < $tnet_headlines} {incr i} { tnet:put $chan $nick $i $tnet_method }
  } else {
    putserv "NOTICE $nick :\[T.Net\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
}

proc tnet:put {chan nick which method} {
  global tnet_id tnet_titel tnet_cat tnet_src tnet_link tnet_tijd tnet_layout tnet_reac tnet_ts tnet_edit
  set outchan $tnet_layout
  regsub -all "%rea" $outchan $tnet_reac($which) outchan
  regsub -all "%edt" $outchan $tnet_edit($which) outchan
  regsub -all "%tyd" $outchan $tnet_tijd($which) outchan
  regsub -all "%id"  $outchan $tnet_id($which) outchan
  regsub -all "%cat" $outchan $tnet_cat($which) outchan
  regsub -all "%src" $outchan $tnet_src($which) outchan
  regsub -all "%lnk" $outchan $tnet_link($which) outchan
  regsub -all "%tit" $outchan $tnet_titel($which) outchan
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

proc tnet:update {} {
  global tnet_ts tnet_updates tnet_lastitem tnet_log tnet_autonews_chan tnet_automax

  if {[tnet:getdata] != -1} {

    if {![info exists tnet_lastitem]} { 
      set tnet_lastitem $tnet_ts(0) 
      if {$tnet_log} { putlog "\[T.Net\] Last news item timestamp set to $tnet_ts(0)" }
    } else {
      if {$tnet_log} { putlog "\[T.Net\] Last news item timestamp is $tnet_ts(0)" }
    }

    if {$tnet_ts(0) > $tnet_lastitem} {
      if {$tnet_log} { putlog "\[T.Net\] There's news!" }
      for {set i 0} {$i < $tnet_automax} {incr i} {
        if {$tnet_ts($i) == $tnet_lastitem} { break }
        foreach chan [split $tnet_autonews_chan] { tnet:put $chan $chan $i 1 }
      }
    } else {
      if {$tnet_log} { putlog "\[T.Net\] No news." } 
    }
  
    set tnet_lastitem $tnet_ts(0)
  }

  if {$tnet_updates < 300} { 
    utimer 300 tnet:update
  } else {
    utimer $tnet_updates tnet:update
  }
}

proc tnet:autoff {nick uhost hand chan text} {
  global lastbind tnet_log tnet_nopub tnet_autonews tnet_lastitem
  if {[lsearch -exact $tnet_nopub [string tolower $chan]] >= 0} { return 0 }

  if {$tnet_log} { putlog "\[T.Net\] Trigger: $lastbind in $chan by $nick" }

  if {$tnet_autonews == 1} {
    set tnet_autonews 0;  unset tnet_lastitem
    set killtimer [utimerexists "tnet:update"]
    if {$killtimer != ""} { killutimer $killtimer }
    putlog "\[T.Net\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn tweakers.net nieuws aankondiger uitgezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn tweakers.net nieuws aankondiger staat al uit."
  }
}

proc tnet:auton {nick uhost hand chan text} {
  global lastbind tnet_log tnet_nopub tnet_autonews tnet_lastitem
  if {[lsearch -exact $tnet_nopub [string tolower $chan]] >= 0} { return 0 }

  if {$tnet_log} { putlog "\[T.Net\] Trigger: $lastbind in $chan by $nick" }

  if {$tnet_autonews == 0} {
    set tnet_autonews 1;  tnet:update
    putlog "\[T.Net\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn tweakers.net nieuws aankondiger aangezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn tweakers.net nieuws aankondiger staat al aan."
  }
}

set killtimer [utimerexists "tnet:update"]
if {$killtimer != ""} { killutimer $killtimer }

if {$tnet_autonews == 1} { tnet:update }

putlog "\[T.Net\] Nieuws script versie $tnet_version: Loaded!"
