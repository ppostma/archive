# $Id: tweakers.tcl,v 1.4 2003-05-17 15:33:03 peter Exp $

# tweakers.tcl / Tweakers.net Nieuws script voor een eggdrop
# version 1.6 / 17/05/2003 / door Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.6: (17/05/03) [bugfixes]
#  - fix memory leak!!! (ongebruikte variabelen unsetten en 
#    de belangrijkste: de ::http::cleanup functie!
#  - veel kleine TCL changes/fixes
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
# Dit script maakt gebruik van een functie uit alltools.tcl.
# Zorg ervoor dat alltools.tcl geladen wordt in je eggdrop configuratie!
#
# Dit script gebruikt ook http.tcl. Deze moet op je systeem aanwezig zijn.
# Zet http.tcl *niet* in je eggdrop configuratie!
#
# Het tweakers.tcl script werkt het best met TCL versies vanaf 8.2.
#
# Voor vragen/suggesties/bugs/etc: peter@webdeveloping.nl
# 
# Pas aub. de configuratie hieronder aan:

### Configuratie instellingen ###

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set tnet(flags) "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set tnet(nopub) "" 

# de triggers: [scheiden met een spatie]
set tnet(triggers) "!tnet !tweakers"

# stuur berichten public of private wanneer er een trigger wordt gebruikt? 
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set tnet(method) 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1] 
set tnet(headlines) 2

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
set tnet(layout) "\[%bT.Net%b\] (%cat) %tit - http://tweakers.net/nieuws/%id"

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set tnet(autonews) 1

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
set tnet(autonewschan) "#kanaal1 #kanaal2"

# om de hoeveel minuten checken of er nieuws is? [minimaal 5]
# zet dit niet te laag, het zal load/verkeer op de servers vergroten.
set tnet(updates) 5

# maximaal aantal berichten die worden getoond tijdens de automatische updates.
# hiermee kan je voorkomen dat de channel wordt ondergeflood als je de 
# updates hoog hebt staan (bv. langer dan een uur).
set tnet(automax) 3

# trigger om het autonews aan te zetten. [string]
set tnet(autontrigger) "!tneton"

# trigger om het autonews uit te zetten. [string]
set tnet(autofftrigger) "!tnetoff"

# benodigde flags om de autonews[on/off] triggers te gebruiken [default=master]
set tnet(autotriggerflag) "m|m"

# log extra informatie (debug) naar de partyline? [0=nee / 1=ja]
set tnet(log) 1

### Eind configuratie instellingen ###



### Begin TCL code ###

set tnet(version) "1.6"

package require http

for {set i 0} {$i < [llength $tnet(triggers)]} {incr i} {
  bind pub $tnet(flags) [lindex $tnet(triggers) $i] tnet:pub
  if {$tnet(log)} { putlog "\[T.Net\] Trigger [lindex $tnet(triggers) $i] added." }
}
unset i

bind pub $tnet(autotriggerflag) $tnet(autofftrigger) tnet:autoff
bind pub $tnet(autotriggerflag) $tnet(autontrigger) tnet:auton

proc tnet:getdata {} {
  global tnet tnetdata

  if {$tnet(log)} { putlog "\[T.Net\] Updating data..." }
  if {[info exists tnetdata]} { unset tnetdata }

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
    regexp "<id>(.*?)</id>" $line trash tnetdata(id,$count)
    regexp "<titel>(.*?)</titel>" $line trash tnetdata(titel,$count)
    regexp "<editor>(.*?)</editor>" $line trash tnetdata(edit,$count)
    regexp "<categorie>(.*?)</categorie>" $line trash tnetdata(cat,$count)
    regexp "<bron>(.*?)</bron>" $line trash tnetdata(src,$count)
    regexp "<link>(.*?)</link>" $line trash tnetdata(link,$count)
    regexp "<tijd>(.*?)</tijd>" $line trash tnetdata(tijd,$count)
    regexp "<timestamp>(.*?)</timestamp>" $line trash tnetdata(ts,$count)
    if {[regexp "<reacties>(.*?)</reacties>" $line trash tnetdata(reac,$count)]} { incr count }
  }
  ::http::cleanup $page

  unset url page msg count lines
  if {[info exists line]} { unset line }
  if {[info exists trash]} { unset trash }

  return 0
}

proc tnet:pub {nick uhost hand chan text} {
  global lastbind tnet tnetdata
  if {[lsearch -exact $tnet(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$tnet(log)} { putlog "\[T.Net\] Trigger: $lastbind in $chan by $nick" }

  if {[tnet:getdata] != -1} {
    for {set i 0} {$i < $tnet(headlines)} {incr i} { tnet:put $chan $nick $i $tnet(method) }
    unset i
  } else {
    putserv "NOTICE $nick :\[T.Net\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
  if {[info exists tnetdata]} { unset tnetdata }
}

proc tnet:put {chan nick which method} {
  global tnet tnetdata

  set outchan $tnet(layout)
  regsub -all "%rea" $outchan $tnetdata(reac,$which) outchan
  regsub -all "%edt" $outchan $tnetdata(edit,$which) outchan
  regsub -all "%tyd" $outchan $tnetdata(tijd,$which) outchan
  regsub -all "%id"  $outchan $tnetdata(id,$which) outchan
  regsub -all "%cat" $outchan $tnetdata(cat,$which) outchan
  regsub -all "%src" $outchan $tnetdata(src,$which) outchan
  regsub -all "%lnk" $outchan $tnetdata(link,$which) outchan
  regsub -all "%tit" $outchan $tnetdata(titel,$which) outchan
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
  unset outchan
}

proc tnet:update {} {
  global tnet tnetdata tnet_lastitem

  if {[tnet:getdata] != -1} {

    if {![info exists tnetdata(ts,0)]} {
      putlog "\[T.Net\] Er iets iets fout gegaan tijdens het updaten..."
      return -1
    }

    if {![info exists tnet_lastitem]} {
      set tnet_lastitem $tnetdata(ts,0)
      if {$tnet(log)} { putlog "\[T.Net\] Last news item timestamp set to $tnetdata(ts,0)" }
    } else {
      if {$tnet(log)} { putlog "\[T.Net\] Last news item timestamp is $tnetdata(ts,0)" }
    }

    if {$tnetdata(ts,0) > $tnet_lastitem} {
      if {$tnet(log)} { putlog "\[T.Net\] There's news!" }
      for {set i 0} {$i < $tnet(automax)} {incr i} {
        if {$tnetdata(ts,0) == $tnet_lastitem} { break }
        foreach chan [split $tnet(autonewschan)] { tnet:put $chan $chan $i 1 }
        unset chan
      }
      unset i
    } else {
      if {$tnet(log)} { putlog "\[T.Net\] No news." } 
    }

    set tnet_lastitem $tnetdata(ts,0)
  }

  if {$tnet(updates) < 5} { 
    timer 5 tnet:update
  } else {
    timer $tnet(updates) tnet:update
  }
  if {[info exists tnetdata]} { unset tnetdata }

  return 0
}

proc tnet:autoff {nick uhost hand chan text} {
  global lastbind tnet tnet_lastitem
  if {[lsearch -exact $tnet(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$tnet(log)} { putlog "\[T.Net\] Trigger: $lastbind in $chan by $nick" }

  if {$tnet(autonews) == 1} {
    set tnet(autonews) 0;  unset tnet_lastitem
    set whichtimer [timerexists "tnet:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    unset whichtimer
    putlog "\[T.Net\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn tweakers.net nieuws aankondiger uitgezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn tweakers.net nieuws aankondiger staat al uit."
  }
}

proc tnet:auton {nick uhost hand chan text} {
  global lastbind tnet
  if {[lsearch -exact $tnet(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$tnet(log)} { putlog "\[T.Net\] Trigger: $lastbind in $chan by $nick" }

  if {$tnet(autonews) == 0} {
    set tnet(autonews) 1;  tnet:update
    putlog "\[T.Net\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn tweakers.net nieuws aankondiger aangezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn tweakers.net nieuws aankondiger staat al aan."
  }
}

set whichtimer [timerexists "tnet:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
unset whichtimer

if {$tnet(autonews) == 1} { tnet:update }

putlog "\[T.Net\] Nieuws script versie $tnet(version): Loaded!"
