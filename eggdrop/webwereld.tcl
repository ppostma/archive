# $Id: webwereld.tcl,v 1.6 2003-07-09 15:14:07 peter Exp $

# WebWereld.nl Nieuws script voor de eggdrop
# version 1.1, 09/07/2003, door Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.1: (??/??/????)
#  - de manier van het updaten is wat veranderd.
#    de webw(updates) setting wordt nu ook door de triggers gebruikt
#    om te checken hoe lang de data gecached moet worden.
#  - proxy configuratie toegevoegd.
#  - flood protectie wordt nu per kanaal bij gehouden (lijkt me nuttiger zo).
# 1.0: (04/07/2003)
#  - eerste versie, gebaseerd op tweakers.tcl v1.9
#
# Dit script maakt gebruik van een functie uit alltools.tcl.
# Zorg ervoor dat alltools.tcl geladen wordt in je eggdrop configuratie!
#
# Dit script gebruikt ook http.tcl. Deze moet op je systeem aanwezig zijn.
# Zet http.tcl *niet* in je eggdrop configuratie!
#
# Het webwereld.tcl script werkt het best met TCL versies vanaf 8.1.
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
set webw(proxy) ""

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set webw(flags) "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set webw(nopub) "" 

# de triggers: [scheiden met een spatie]
set webw(triggers) "!webwereld !ww"

# flood protectie: aantal seconden tussen gebruik van de triggers
# voor geen flood protectie: zet 't op 0
set webw(antiflood) 10

# stuur berichten public of private wanneer er een trigger wordt gebruikt? 
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set webw(method) 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1] 
set webw(headlines) 2

# om de hoeveel minuten checken of er nieuws is? [minimaal 30]
# deze waarde wordt gebruikt door zowel de triggers als het autonews.
set webw(updates) 30

# hieronder kun je de layout aanpassen voor de output:
# %title = titel van artikel
# %link  = url/link naar artikel
# %descr = description, beschrijving
# %b     = bold (dikgedrukte) tekst
# %u     = underlined (onderstreepte) tekst
set webw(layout) "\[%bWebWereld%b\] %title - %link"

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set webw(autonews) 0

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
set webw(autonewschan) "#kanaal1 #kanaal2"

# maximaal aantal berichten die worden getoond tijdens de automatische updates.
# hiermee kan je voorkomen dat de channel wordt ondergeflood als je de 
# updates hoog hebt staan (bv. langer dan een uur).
set webw(automax) 3

# trigger om het autonews aan te zetten. [string]
set webw(autontrigger) "!wwon"

# trigger om het autonews uit te zetten. [string]
set webw(autofftrigger) "!wwoff"

# benodigde flags om de autonews[on/off] triggers te gebruiken [default=master]
set webw(autotriggerflag) "m|m"

# log extra informatie (debug) naar de partyline? [0=nee / 1=ja]
set webw(log) 1

### Eind configuratie instellingen ###



### Begin TCL code ###

package require http

set webw(version) "1.1"

if {[info tclversion] < 8.1} {
  putlog "\[WebWereld\] Kan [file tail [info script]] niet laden: U heeft minimaal TCL versie 8.1 nodig en u heeft TCL versie [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[WebWereld\] Kan [file tail [info script]] niet laden: Zorg ervoor dat alltools.tcl in uw eggdrop configuratie wordt geladen!"
  return 1
}

set whichtimer [timerexists "webw:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

for {set i 0} {$i < [llength $webw(triggers)]} {incr i} {
  bind pub $webw(flags) [lindex $webw(triggers) $i] webw:pub
  if {$webw(log)} { putlog "\[WebWereld\] Trigger [lindex $webw(triggers) $i] added." }
}
catch { unset i }

bind pub $webw(autotriggerflag) $webw(autofftrigger) webw:autoff
bind pub $webw(autotriggerflag) $webw(autontrigger) webw:auton

proc webw:getdata {} {
  global webw webwdata

  if {$webw(log)} { putlog "\[WebWereld\] Updating data..." }

  set url "http://www.webwereld.nl/rss/trillian.rss"
  set page [::http::config -useragent "Mozilla"]

  if {$webw(proxy) != ""} {
    if {![regexp {(.+):([0-9].*?)} $webw(proxy) t proxyhost proxyport]} {
      putlog "\[WebWereld\] Wrong proxy configuration ($webw(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[WebWereld\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[WebWereld\] Problem: [::http::status $page]"
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[WebWereld\] Problem: [::http::code $page]"
    return -1
  }

  if {[info exists webwdata]} { unset webwdata }

  set lines [split [::http::data $page] \n]
  set count 0
  set item 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regsub -all "\\&" $line "\\\\&" line
    if {[regexp "<item>" $line]} { set item 1 }
    if {[regexp "</item>" $line]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*?)</title>" $line trash webwdata(title,$count)
      regexp "<link>(.*?)</link>" $line trash webwdata(link,$count)
      if {[regexp "<description>(.*?)</description>" $line trash webwdata(descr,$count)]} { incr count }
    }
  }

  set webw(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg lines count item line trash }

  return 0
}

proc webw:pub {nick uhost hand chan text} {
  global lastbind webw webwdata
  if {[lsearch -exact $webw(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {[info exists webw(floodprot,$chan)]} {
    set verschil [expr [clock seconds] - $webw(floodprot,$chan)]
    if {$verschil < $webw(antiflood)} {
      putquick "NOTICE $nick :Trigger is net al gebruikt! Wacht aub. [expr $webw(antiflood) - $verschil] seconden..."
      return 0
    }
  }
  set webw(floodprot,$chan) [clock seconds]

  if {$webw(log)} { putlog "\[WebWereld\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists webw(lastupdate)]} {
    if {[expr [clock seconds] - $webw(lastupdate)] > [expr $webw(updates) * 60]} {
      set ret [webw:getdata]
    }
  } elseif {![info exists webwdata(link,0)]} {
    set ret [webw:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $webw(headlines)} {incr i} { 
      if {![info exists webwdata(link,$i)]} { break }
      webw:put $chan $nick $i $webw(method)
    }
  } else {
    putserv "NOTICE $nick :\[WebWereld\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
  catch { unset ret verschil i }
}

proc webw:put {chan nick which method} {
  global webw webwdata

  set outchan $webw(layout)
  regsub -all "%title" $outchan $webwdata(title,$which) outchan
  regsub -all "%link"  $outchan $webwdata(link,$which) outchan
  regsub -all "%descr" $outchan $webwdata(descr,$which) outchan
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

proc webw:update {} {
  global webw webwdata

  if {[webw:getdata] != -1} {

    if {![info exists webwdata(link,0)]} {
      putlog "\[WebWereld\] Something went wrong while updating..."
      return -1
    }

    if {![info exists webw(lastitem)]} {
      set webw(lastitem) $webwdata(link,0)
      if {$webw(log)} { putlog "\[WebWereld\] Last news item set to '$webwdata(link,0)'." }
    } else {
      if {$webw(log)} { putlog "\[WebWereld\] Last news item is '$webwdata(link,0)'." }
    }

    if {$webwdata(link,0) != $webw(lastitem)} {
      if {$webw(log)} { putlog "\[WebWereld\] There's news!" }
      for {set i 0} {$i < $webw(automax)} {incr i} {
        if {![info exists webwdata(link,$i)]} { break }
        if {$webwdata(link,$i) == $webw(lastitem)} { break }
        foreach chan [split $webw(autonewschan)] { webw:put $chan $chan $i 1 }
        catch { unset chan }
      }
      catch { unset i }
    } else {
      if {$webw(log)} { putlog "\[WebWereld\] No news." } 
    }

    set webw(lastitem) $webwdata(link,0)
  }

  if {$webw(updates) < 30} {
    putlog "\[WebWereld\] Warning: the \$webw(updates) setting is too low! Defaulting to 30 minutes..."
    timer 30 webw:update
  } else {
    timer $webw(updates) webw:update
  }

  return 0
}

proc webw:autoff {nick uhost hand chan text} {
  global lastbind webw
  if {[lsearch -exact $webw(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$webw(log)} { putlog "\[WebWereld\] Trigger: $lastbind in $chan by $nick" }

  if {$webw(autonews) == 1} {
    set webw(autonews) 0;  catch { unset webw(lastitem) }
    set whichtimer [timerexists "webw:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[WebWereld\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn webwereld.nl nieuws aankondiger uitgezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn webwereld.nl nieuws aankondiger staat al uit."
  }
}

proc webw:auton {nick uhost hand chan text} {
  global lastbind webw
  if {[lsearch -exact $webw(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$webw(log)} { putlog "\[WebWereld\] Trigger: $lastbind in $chan by $nick" }

  if {$webw(autonews) == 0} {
    set webw(autonews) 1;  webw:update
    putlog "\[WebWereld\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION heeft zijn webwereld.nl nieuws aankondiger aangezet.\001"
  } else {
    putserv "NOTICE $nick :Mijn webwereld.nl nieuws aankondiger staat al aan."
  }
}

if {$webw(autonews) == 1} { webw:update }

putlog "\[WebWereld\] Nieuws script versie $webw(version): Loaded!"
