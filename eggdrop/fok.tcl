# $Id: fok.tcl,v 1.13 2003-06-24 15:29:01 peter Exp $

# fok.nl Nieuws script voor een eggdrop
# version 1.9, 24/06/2003, door Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.9: (??/??/????)
#  - url voor laatste versie toegevoegd.
#  - style changes.
# 1.8: (26/05/03) [bugfix]
#  - 3de poging om de bug met & teken te fixen.
# 1.7: (20/05/03) [changes]
#  - de vreemde bug met het & teken is nu op een nettere manier gefixed.
#  - kleine aanpassingen om het script nog wat robuuster te maken.
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
#  - checkt of fok.nl down is. als dit niet wordt gecheckt dan
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
# Het fok.tcl script werkt het best met TCL versies vanaf 8.2.
#
# Voor vragen/suggesties/bugs/etc: peter@webdeveloping.nl
#
# De laatste versie van dit script kan je hier vinden:
#   http://www.pointless.nl/?page=eggdrop
#
# Pas aub. de configuratie hieronder aan:
#

### Configuratie instellingen ###

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set fok(flags) "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set fok(nopub) "" 

# de triggers: [scheiden met een spatie]
set fok(triggers) "!fok fok!"

# stuur berichten public of private wanneer er een trigger wordt gebruikt? 
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set fok(method) 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1] 
set fok(headlines) 2

# hieronder kun je de layout aanpassen:
# %tyd = tijd
# %tit = titel
# %id  = id (wordt gebruikt in de nieuws url)
# %rea = aantal reacties
# %b   = bold (dikgedrukte) tekst
# %u   = underlined (onderstreepte) tekst
set fok(layout) "\[%bFok!%b\] %tit - http://fok.nl/?id=%id"

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set fok(autonews) 0

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
set fok(autonewschan) "#kanaal1 #kanaal2"

# om de hoeveel minuten checken of er nieuws is? [minimaal 5]
# zet dit niet te laag, het zal load/verkeer op de servers vergroten.
set fok(updates) 5

# maximaal aantal berichten die worden getoond tijdens de automatische updates.
# hiermee kan je voorkomen dat de channel wordt ondergeflood als je de 
# updates hoog hebt staan (bv. langer dan een uur).
set fok(automax) 3

# trigger om het autonews aan te zetten. [string]
set fok(autontrigger) "!fokon"

# trigger om het autonews uit te zetten. [string]
set fok(autofftrigger) "!fokoff"

# benodigde flags om de autonews[on/off] triggers te gebruiken [default=master]
set fok(autotriggerflag) "m|m"

# log extra informatie (debug) naar de partyline? [0=nee / 1=ja]
set fok(log) 1

### Eind configuratie instellingen ###



### Begin TCL code ###

set fok(version) "1.9"

package require Tcl 8.2
package require http

for {set i 0} {$i < [llength $fok(triggers)]} {incr i} {
  bind pub $fok(flags) [lindex $fok(triggers) $i] fok:pub
  if {$fok(log)} { putlog "\[Fok!\] Trigger [lindex $fok(triggers) $i] added." }
}
catch { unset i }

bind pub $fok(autotriggerflag) $fok(autofftrigger) fok:autoff
bind pub $fok(autotriggerflag) $fok(autontrigger) fok:auton

proc fok:getdata {} {
  global fok fokdata

  if {$fok(log)} { putlog "\[Fok!\] Updating data..." }

  set url "http://www.athena.fokzine.net/~danny/remote.xml"
  set page [::http::config -useragent "Mozilla"]

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[Fok!\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[Fok!\] Problem: [::http::status $page]"
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Fok!\] Problem: [::http::code $page]"
    return -1
  }

  if {[info exists fokdata]} { unset fokdata }

  set lines [split [::http::data $page] \n]
  set count 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regsub -all "\\&" $line "\\\\&" line
    regexp "<id>(.*?)</id>" $line trash fokdata(id,$count)
    regexp "<titel>(.*?)</titel>" $line trash fokdata(titel,$count)
    regexp "<time>(.*?)</time>" $line trash fokdata(tijd,$count)
    regexp "<timestamp>(.*?)</timestamp>" $line trash fokdata(ts,$count)
    if {[regexp "<reacties>(.*?)</reacties>" $line trash fokdata(reac,$count)]} { incr count }
  }

  catch { ::http::cleanup $page }
  catch { unset url page msg lines count line trash }

  return 0
}

proc fok:pub {nick uhost hand chan text} {
  global lastbind fok fokdata
  if {[lsearch -exact $fok(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$fok(log)} { putlog "\[Fok!\] Trigger: $lastbind in $chan by $nick" }

  if {[fok:getdata] != -1} {
    for {set i 0} {$i < $fok(headlines)} {incr i} {
      if {![info exists fokdata(id,$i)]} { break }
      fok:put $chan $nick $i $fok(method)
    }
    catch { unset i }
  } else {
    putserv "NOTICE $nick :\[Fok!\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
  if {[info exists fokdata]} { unset fokdata }
}

proc fok:put {chan nick which method} {
  global fok fokdata

  set outchan $fok(layout)
  regsub -all "%tyd" $outchan $fokdata(tijd,$which) outchan
  regsub -all "%id"  $outchan $fokdata(id,$which) outchan
  regsub -all "%rea" $outchan $fokdata(reac,$which) outchan
  regsub -all "%tit" $outchan $fokdata(titel,$which) outchan
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

proc fok:update {} {
  global fok fokdata

  if {[fok:getdata] != -1} {

    if {![info exists fokdata(ts,0)]} {
      putlog "\[Fok!\] Something went wrong while updating..."
      return -1
    }

    if {![info exists fok(lastitem)]} {
      set fok(lastitem) $fokdata(ts,0)
      if {$fok(log)} { putlog "\[Fok!\] Last news item timestamp set to '$fokdata(ts,0)'." }
    } else {
      if {$fok(log)} { putlog "\[Fok!\] Last news item timestamp is '$fokdata(ts,0)'." }
    }

    if {$fokdata(ts,0) > $fok(lastitem)} {
      if {$fok(log)} { putlog "\[Fok!\] There's news!" }
      for {set i 0} {$i < $fok(automax)} {incr i} {
        if {![info exists fokdata(ts,$i)]} { break }
        if {$fokdata(ts,$i) == $fok(lastitem)} { break }
        foreach chan [split $fok(autonewschan)] { fok:put $chan $chan $i 1 }
        catch { unset chan }
      }
      catch { unset i }
    } else {
      if {$fok(log)} { putlog "\[Fok!\] No news." } 
    }

    set fok(lastitem) $fokdata(ts,0)
  }

  if {$fok(updates) < 5} { 
    putlog "\[Fok!\] Warning: the \$fok(updates) setting is too low! Defaulting to 5 minutes..."
    timer 5 fok:update
  } else {
    timer $fok(updates) fok:update
  }
  if {[info exists fokdata]} { unset fokdata }

  return 0
}

proc fok:autoff {nick uhost hand chan text} {
  global lastbind fok
  if {[lsearch -exact $fok(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$fok(log)} { putlog "\[Fok!\] Trigger: $lastbind in $chan by $nick" }

  if {$fok(autonews) == 1} {
    set fok(autonews) 0;  catch { unset fok(lastitem) }
    set whichtimer [timerexists "fok:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[Fok!\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn fok.nl nieuws aankondiger uitgezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn fok.nl nieuws aankondiger staat al uit."
  }
}

proc fok:auton {nick uhost hand chan text} {
  global lastbind fok
  if {[lsearch -exact $fok(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$fok(log)} { putlog "\[Fok!\] Trigger: $lastbind in $chan by $nick" }

  if {$fok(autonews) == 0} {
    set fok(autonews) 1;  fok:update
    putlog "\[Fok!\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn fok.nl nieuws aankondiger aangezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn fok.nl nieuws aankondiger staat al aan."
  }
}

set whichtimer [timerexists "fok:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

if {$fok(autonews) == 1} { fok:update }

putlog "\[Fok!\] Nieuws script versie $fok(version): Loaded!"
