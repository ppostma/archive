# $Id: gamer.tcl,v 1.18 2003-07-07 17:36:16 peter Exp $

# Gamer.nl Nieuws script voor de eggdrop
# version 2.0, 07/07/2003, door Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 2.0: (??/??/????)
#  - de manier van het updaten is wat veranderd.
#    de gamer(updates) setting wordt nu ook door de triggers gebruikt
#    om te checken hoe lang de data gecached moet worden.
#  - proxy configuratie toegevoegd.
# 1.9: (04/07/2003) [changes]
#  - check voor goede TCL versie & alltools.tcl
#  - flood protectie toegevoegd.
#  - url voor laatste versie toegevoegd.
#  - code style changes.
# 1.8: (26/05/2003) [bugfix]
#  - 3de poging om de bug met & teken te fixen.
# 1.7: (20/05/2003) [changes]
#  - de vreemde bug met het & teken is nu op een nettere manier gefixed.
#  - kleine aanpassingen om het script nog wat robuuster te maken.
# 1.6: (17/05/2003) [bugfixes]
#  - fix memory leak!!! (ongebruikte variabelen unsetten en 
#    de belangrijkste: de ::http::cleanup functie!
#  - veel kleine TCL changes/fixes
# 1.5: (12/10/2002) [features/changes]
#  - extra check in de getdata procedure (voor HTTP errors zoals 404, etc..)
#  - triggers/functie toegevoegd om de aankondiger aan en uit te zetten.
#  - stukjes code netter en beter geschreven.
#  - meer gebruiksvriendelijke error en log berichten.
# 1.4a: (09/10/2002) [bugfixes]
#  - bugje gefixed met het & teken. regsub zet &amp; om naar %titamp; 
#    ik heb geen idee waarom, maar de output is iig gefixed :-)  
#  - wat fouten gefixed in de getdata procedure. checks werken nu helemaal ok.
#  - timer opnieuw instellen wanneer er een http error is, is gefixed.
# 1.4: (08/10/2002) [features/changes]
#  - methode om berichten te sturen is nu in te stellen.
#  - check op timestamp ipv. id ! (dit had ik eerder moeten bedenken >:) 
#  - extra layout opties toegevoegd.
#  - updates minimaal 300 seconden!
#  - checkt of gamer.nl down is. als dit niet wordt gecheckt dan
#    blijft het script hangen en moet de bot opnieuw gestart worden.
# 1.3: (06/10/2002) [features/changes]
#  - triggers in te stellen.
#  - kanalen in te stellen waar de triggers gebruikt mogen worden.
#  - layout in te stellen.
#  - automatisch nieuws naar het kanaal sturen (kanalen in te stellen).
#  - automatische updates in te stellen (om de hoeveel seconden).
# 1.2: (03/10/2002) [rewrote 1.1]
#  - helemaal opnieuw geschreven.
#  - betere code (regexp, regsub, http)
# 1.1: (21/08/2002) [first version]
#  - eerste (goed werkende) versie. 
#  - script had alleen triggers, verder nog helemaal niets :-)
#
# Dit script maakt gebruik van een functie uit alltools.tcl.
# Zorg ervoor dat alltools.tcl geladen wordt in je eggdrop configuratie!
#
# Dit script gebruikt ook http.tcl. Deze moet op je systeem aanwezig zijn.
# Zet http.tcl *niet* in je eggdrop configuratie!
#
# Het gamer.tcl script werkt het best met TCL versies vanaf 8.1.
#
# Voor vragen/suggesties/bugs/etc: peter@webdeveloping.nl
#
# De laatste versie van dit script kan je hier vinden:
#   http://www.pointless.nl/?page=eggdrop
#
# Pas aub. de configuratie hieronder aan:
#

### Configuratie instellingen ###

# maak gebruik van een http proxy om de gegevens op te halen?
# stel op deze manier in: "host.isp.com:port" of laat 't leeg voor geen proxy
set gamer(proxy) ""

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set gamer(flags) "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set gamer(nopub) "" 

# de triggers: [scheiden met een spatie]
set gamer(triggers) "!gamer"

# flood protectie: aantal seconden tussen gebruik van de triggers
# voor geen flood protectie: zet 't op 0
set gamer(antiflood) 10

# stuur berichten public of private wanneer er een trigger wordt gebruikt? 
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set gamer(method) 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1] 
set gamer(headlines) 2

# om de hoeveel minuten checken of er nieuws is? [minimaal 5]
# deze waarde wordt gebruikt door zowel de triggers als het autonews.
set gamer(updates) 5

# hieronder kun je de layout aanpassen voor de output:
# %tyd = tijd
# %tit = titel
# %aut = auteur / schrijver
# %id  = id (wordt gebruikt in de nieuws url)
# %rea = aantal reacties
# %b   = bold (dikgedrukte) tekst
# %u   = underlined (onderstreepte) tekst
set gamer(layout) "\[%bGamer.nl%b\] %tit - http://gamer.nl/nieuws/%id"

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set gamer(autonews) 0

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
set gamer(autonewschan) "#kanaal1 #kanaal2"

# maximaal aantal berichten die worden getoond tijdens de automatische updates.
# hiermee kan je voorkomen dat de channel wordt ondergeflood als je de 
# updates hoog hebt staan (bv. langer dan een uur).
set gamer(automax) 3

# trigger om het autonews aan te zetten. [string]
set gamer(autontrigger) "!gameron"

# trigger om het autonews uit te zetten. [string]
set gamer(autofftrigger) "!gameroff"

# benodigde flags om de autonews[on/off] triggers te gebruiken [default=master]
set gamer(autotriggerflag) "m|m"

# log extra informatie (debug) naar de partyline? [0=nee / 1=ja]
set gamer(log) 1

### Eind configuratie instellingen ###



### Begin TCL code ###

package require http

set gamer(version) "2.0"

if {[info tclversion] < 8.1} {
  putlog "\[Gamer.nl\] Kan [file tail [info script]] niet laden: U heeft minimaal TCL versie 8.1 nodig en u heeft TCL versie [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[Gamer.nl\] Kan [file tail [info script]] niet laden: Zorg ervoor dat alltools.tcl in uw eggdrop configuratie wordt geladen!"
  return 1
}

set whichtimer [timerexists "gamer:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

for {set i 0} {$i < [llength $gamer(triggers)]} {incr i} {
  bind pub $gamer(flags) [lindex $gamer(triggers) $i] gamer:pub
  if {$gamer(log)} { putlog "\[Gamer.nl\] Trigger [lindex $gamer(triggers) $i] added." }
}
catch { unset i }

bind pub $gamer(autotriggerflag) $gamer(autofftrigger) gamer:autoff
bind pub $gamer(autotriggerflag) $gamer(autontrigger) gamer:auton

proc gamer:getdata {} {
  global gamer gamerdata

  if {$gamer(log)} { putlog "\[Gamer.nl\] Updating data..." }

  set url "http://www.gamer.nl/newstracker.xml"
  set page [::http::config -useragent "Mozilla"]

  if {$gamer(proxy) != ""} {
    if {![regexp {(.+):([0-9].*?)} $gamer(proxy) t proxyhost proxyport]} {
      putlog "\[Gamer.nl\] Wrong proxy configuration ($gamer(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[Gamer.nl\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[Gamer.nl\] Problem: [::http::status $page]"
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Gamer.nl\] Problem: [::http::code $page]"
    return -1
  }

  if {[info exists gamerdata]} { unset gamerdata }

  set lines [split [::http::data $page] \n]
  set count 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regsub -all "\\&" $line "\\\\&" line
    regexp "<id>(.*?)</id>" $line trash gamerdata(id,$count)
    regexp "<title>(.*?)</title>" $line trash gamerdata(titel,$count)
    regexp "<author>(.*?)</author>" $line trash gamerdata(auteur,$count)
    regexp "<time>(.*?)</time>" $line trash gamerdata(tijd,$count)
    regexp "<unix_timestamp>(.*?)</unix_timestamp>" $line trash gamerdata(ts,$count)
    if {[regexp "<comments>(.*?)</comments>" $line trash gamerdata(reac,$count)]} { incr count }
  }

  set gamer(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg lines count line trash }

  return 0
}

proc gamer:pub {nick uhost hand chan text} {
  global lastbind gamer gamerdata
  if {[lsearch -exact $gamer(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {[info exists gamer(floodprot)]} {
    set verschil [expr [clock seconds] - $gamer(floodprot)]
    if {$verschil < $gamer(antiflood)} {
      putquick "NOTICE $nick :Trigger is net al gebruikt! Wacht aub. [expr $gamer(antiflood) - $verschil] seconden..."
      return 0
    }
  }
  set gamer(floodprot) [clock seconds]

  if {$gamer(log)} { putlog "\[Gamer.nl\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists gamer(lastupdate)]} {
    if {[expr [clock seconds] - $gamer(lastupdate)] > [expr $gamer(updates) * 60]} {
      set ret [gamer:getdata]
    }
  } elseif {![info exists gamerdata(id,0)]} {
    set ret [gamer:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $gamer(headlines)} {incr i} {
      if {![info exists gamerdata(id,$i)]} { break }
      gamer:put $chan $nick $i $gamer(method)
    }
  } else {
    putserv "NOTICE $nick :\[Gamer.nl\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
  catch { unset ret verschil i }
}

proc gamer:put {chan nick which method} {
  global gamer gamerdata

  set outchan $gamer(layout)
  regsub -all "%tyd" $outchan $gamerdata(tijd,$which) outchan
  regsub -all "%id"  $outchan $gamerdata(id,$which) outchan
  regsub -all "%rea" $outchan $gamerdata(reac,$which) outchan
  regsub -all "%aut" $outchan $gamerdata(auteur,$which) outchan
  regsub -all "%tit" $outchan $gamerdata(titel,$which) outchan
  regsub -all "&amp;"  $outchan "\\&" outchan
  regsub -all "&quot;" $outchan "\"" outchan
  regsub -all "%b" $outchan "\002" outchan
  regsub -all "%u" $outchan "\037" outchan
  switch -- $method {
    0 { putserv "PRIVMSG $nick :$outchan" }
    1 { putserv "PRIVMSG $chan :$outchan" }
    2 { putserv "NOTICE $nick :$outchan" }
    3 { putserv "NOTICE $chan :$outchan" }
    default { putserv "PRIVMSG $chan :$outchan" }
  }
  catch { unset item outchan }
}

proc gamer:update {} {
  global gamer gamerdata

  if {[gamer:getdata] != -1} {

    if {![info exists gamerdata(ts,0)]} {
      putlog "\[Gamer.nl\] Something went wrong while updating..."
      return -1
    }

    if {![info exists gamer(lastitem)]} {
      set gamer(lastitem) $gamerdata(ts,0)
      if {$gamer(log)} { putlog "\[Gamer.nl\] Last news item timestamp set to '$gamerdata(ts,0)'." }
    } else {
      if {$gamer(log)} { putlog "\[Gamer.nl\] Last news item timestamp is '$gamerdata(ts,0)'." }
    }

    if {$gamerdata(ts,0) > $gamer(lastitem)} {
      if {$gamer(log)} { putlog "\[Gamer.nl\] There's news!" }
      for {set i 0} {$i < $gamer(automax)} {incr i} {
        if {![info exists gamerdata(ts,$i)]} { break }
        if {$gamerdata(ts,$i) == $gamer(lastitem)} { break }
        foreach chan [split $gamer(autonewschan)] { gamer:put $chan $chan $i 1 }
        catch { unset chan }
      }
      catch { unset i }
    } else {
      if {$gamer(log)} { putlog "\[Gamer.nl\] No news." } 
    }

    set gamer(lastitem) $gamerdata(ts,0)
  }

  if {$gamer(updates) < 5} {
    putlog "\[Gamer.nl\] Warning: the \$gamer(updates) setting is too low! Defaulting to 5 minutes..."
    timer 5 gamer:update
  } else {
    timer $gamer(updates) gamer:update
  }

  return 0
}

proc gamer:autoff {nick uhost hand chan text} {
  global lastbind gamer
  if {[lsearch -exact $gamer(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$gamer(log)} { putlog "\[Gamer.nl\] Trigger: $lastbind in $chan by $nick" }

  if {$gamer(autonews) == 1} {
    set gamer(autonews) 0;  catch { unset gamer(lastitem) }
    set whichtimer [timerexists "gamer:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[Gamer.nl\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn gamer.nl nieuws aankondiger uitgezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn gamer.nl nieuws aankondiger staat al uit."
  }
}

proc gamer:auton {nick uhost hand chan text} {
  global lastbind gamer
  if {[lsearch -exact $gamer(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$gamer(log)} { putlog "\[Gamer.nl\] Trigger: $lastbind in $chan by $nick" }

  if {$gamer(autonews) == 0} {
    set gamer(autonews) 1;  gamer:update
    putlog "\[Gamer.nl\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn gamer.nl nieuws aankondiger aangezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn gamer.nl nieuws aankondiger staat al aan."
  }
}

if {$gamer(autonews) == 1} { gamer:update }

putlog "\[Gamer.nl\] Nieuws script versie $gamer(version): Loaded!"
