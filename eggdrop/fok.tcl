# $Id: fok.tcl,v 1.35 2004-02-27 20:36:43 peter Exp $

# fok.nl Nieuws script voor de eggdrop
# version 2.1, 27/02/2004, door Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 2.1: (27/02/2004)
#  - script werkt weer met de nieuwe FOK! site.
# 2.0: (24/08/2003)
#  - de manier van het updaten is wat veranderd.
#    de fok(updates) setting wordt nu ook door de triggers gebruikt
#    om te checken hoe lang de data gecached moet worden.
#  - proxy configuratie toegevoegd.
#  - flood protectie wordt nu per kanaal bijgehouden (lijkt me nuttiger zo).
#  - autonews /amsg optie toegevoegd.
#  - script werkt nu ook met Tcl 8.0.
# 1.9: (04/07/2003) [changes]
#  - check voor goede Tcl versie & alltools.tcl
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
#  - veel kleine Tcl changes/fixes
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
#  - checkt of fok.nl down is. als dit niet wordt gecheckt dan
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
# Het fok.tcl script werkt het best met Tcl versies vanaf 8.0.
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
set fok(proxy) ""

# de triggers: [scheiden met een spatie]
set fok(triggers) "!fok fok!"

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set fok(flags) "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set fok(nopub) "" 

# stuur berichten public of private wanneer er een trigger wordt gebruikt?
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set fok(method) 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1]
set fok(headlines) 2

# flood protectie: aantal seconden tussen gebruik van de triggers
# zet 't op 0 om de flood protectie uit te zetten.
set fok(antiflood) 10

# hieronder kun je de layout aanpassen voor de output:
# %datum = datum (vb: Fri, 27 Feb 2004 17:36:47 +0100)
# %titel = titel bericht
# %link  = link naar het bericht
# %b     = bold (dikgedrukte) tekst
# %u     = underlined (onderstreepte) tekst
set fok(layout) "\[%bFok!%b\] %titel - %link"

# om de hoeveel minuten checken of er nieuws is? [minimaal 5]
# deze waarde wordt gebruikt door zowel de triggers als het autonews.
set fok(updates) 5

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set fok(autonews) 0

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
# gebruik "*" om het nieuws naar alle kanalen te sturen (/amsg).
set fok(autonewschan) "#kanaal1 #kanaal2"

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



### Begin Tcl code ###

set fok(version) "2.1"

if {[catch { package require http } err]} {
  putlog "\[Fok!\] Kan [file tail [info script]] niet laden: Probleem met het laden van de http package: $err"
  return 1
}

if {[info tclversion] < 8.0} {
  putlog "\[Fok!\] Kan [file tail [info script]] niet laden: U heeft minimaal Tcl versie 8.0 nodig en u heeft Tcl versie [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[Fok!\] Kan [file tail [info script]] niet laden: Zorg ervoor dat alltools.tcl in uw eggdrop configuratie wordt geladen!"
  return 1
}

set whichtimer [timerexists "fok:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

foreach trigger [split $fok(triggers)] {
  bind pub $fok(flags) $trigger fok:pub
  if {$fok(log)} { putlog "\[Fok!\] Trigger $trigger added." }
}
catch { unset trigger }

if {$fok(autofftrigger) != ""} { bind pub $fok(autotriggerflag) $fok(autofftrigger) fok:autoff }
if {$fok(autontrigger)  != ""} { bind pub $fok(autotriggerflag) $fok(autontrigger) fok:auton }

proc fok:getdata {} {
  global fok fokdata

  if {$fok(log)} { putlog "\[Fok!\] Updating data..." }

  set url "http://rss.fok.nl/feeds/nieuws"
  set page [::http::config -useragent "Mozilla"]

  if {$fok(proxy) != ""} {
    if {![regexp {(.+):([0-9]+)} $fok(proxy) trash proxyhost proxyport]} {
      putlog "\[Fok!\] Wrong proxy configuration ($fok(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch { set page [::http::geturl $url -timeout 15000] } msg]} {
    putlog "\[Fok!\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[Fok!\] Problem: [::http::status $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Fok!\] Problem: [::http::code $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[catch { set data [::http::data $page] } msg]} {
    putlog "\[Fok!\] Problem: $msg"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[info exists fokdata]} { unset fokdata }

  set count 0
  set item 0
  foreach line [split $data \n] {
    regsub -all "\\&" $line "\\\\&" line
    if {[regexp "<item>" $line]} { set item 1 }
    if {[regexp "</item>" $line]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*)</title>" $line trash fokdata(titel,$count)
      regexp "<pubDate>(.*)</pubDate>" $line trash fokdata(datum,$count)
      if {[regexp "<link>(.*)</link>" $line trash fokdata(link,$count)]} { incr count }
    }
  }

  set fok(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg data count item line trash }

  return 0
}

proc fok:pub {nick uhost hand chan text} {
  global lastbind fok fokdata
  if {[lsearch -exact $fok(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$fok(antiflood) > 0} {
    if {[info exists fok(floodprot,$chan)]} {
      set diff [expr [clock seconds] - $fok(floodprot,$chan)]
      if {$diff < $fok(antiflood)} {
        putquick "NOTICE $nick :Trigger is net al gebruikt! Wacht aub. [expr $fok(antiflood) - $diff] seconden..."
        return 0
      }
    }
    set fok(floodprot,$chan) [clock seconds]
  }

  if {$fok(log)} { putlog "\[Fok!\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists fok(lastupdate)]} {
    if {[expr [clock seconds] - $fok(lastupdate)] >= [expr $fok(updates) * 60]} {
      set ret [fok:getdata]
    }
  } elseif {![info exists fokdata(link,0)]} {
    set ret [fok:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $fok(headlines)} {incr i} {
      if {![info exists fokdata(link,$i)]} { break }
      if {[catch { fok:put $chan $nick $i $fok(method) } err]} {
        putlog "\[Fok!\] Problem in data array: $err"
      }
    }
  } else {
    putserv "NOTICE $nick :\[Fok!\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
  catch { unset ret diff i err }
}

proc fok:put {chan nick which method} {
  global fok fokdata

  set outchan $fok(layout)
  regsub -all "%datum" $outchan $fokdata(datum,$which) outchan
  regsub -all "%link"  $outchan $fokdata(link,$which) outchan
  regsub -all "%titel" $outchan $fokdata(titel,$which) outchan
  regsub -all "&amp;"  $outchan "\\&" outchan
  regsub -all "&quot;" $outchan "\"" outchan
  regsub -all "%b" $outchan "\002" outchan
  regsub -all "%u" $outchan "\037" outchan
  switch $method {
    0 { putserv "PRIVMSG $nick :$outchan" }
    1 { putserv "PRIVMSG $chan :$outchan" }
    2 { putserv "NOTICE $nick :$outchan" }
    3 { putserv "NOTICE $chan :$outchan" }
    default { putserv "PRIVMSG $chan :$outchan" }
  }
  catch { unset outchan }
}

proc fok:update {} {
  global fok fokdata

  if {[fok:getdata] != -1} {

    if {![info exists fokdata(link,0)]} {
      putlog "\[Fok!\] Something went wrong while updating..."
      return -1
    }

    if {![info exists fok(lastitem)]} {
      set fok(lastitem) $fokdata(link,0)
      if {$fok(log)} { putlog "\[Fok!\] Last news item set to '$fokdata(link,0)'." }
    } else {
      if {$fok(log)} { putlog "\[Fok!\] Last news item is '$fokdata(link,0)'." }
    }

    if {$fokdata(link,0) != $fok(lastitem)} {
      if {$fok(log)} { putlog "\[Fok!\] There's news!" }
      if {[regexp {^\*$} $fok(autonewschan)]} {
        set dest [channels]
      } else {
        set dest $fok(autonewschan)
      }
      for {set i 0} {$i < $fok(automax)} {incr i} {
        if {![info exists fokdata(link,$i)]} { break }
        if {$fokdata(link,$i) == $fok(lastitem)} { break }
        foreach chan [split $dest] { 
          if {[catch { fok:put $chan $chan $i 1 } err]} {
            putlog "\[Fok!\] Problem in data array: $err"
          }
        }
      }
      catch { unset dest i chan err }
    } else {
      if {$fok(log)} { putlog "\[Fok!\] No news." }
    }

    set fok(lastitem) $fokdata(link,0)
  }

  if {$fok(updates) < 5} {
    putlog "\[Fok!\] Warning: the \$fok(updates) setting is too low! Defaulting to 5 minutes..."
    timer 5 fok:update
  } else {
    timer $fok(updates) fok:update
  }

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

if {$fok(autonews) == 1} { fok:update }

putlog "\[Fok!\] Nieuws script versie $fok(version): Loaded!"
