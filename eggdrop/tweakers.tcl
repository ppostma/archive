# $Id: tweakers.tcl,v 1.21 2003-07-09 15:14:07 peter Exp $

# Tweakers.net Nieuws script voor de eggdrop
# version 2.0, 09/07/2003, door Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 2.0: (??/??/????)
#  - de manier van het updaten is wat veranderd.
#    de tnet(updates) setting wordt nu ook door de triggers gebruikt
#    om te checken hoe lang de data gecached moet worden.
#  - proxy configuratie toegevoegd.
#  - flood protectie wordt nu per kanaal bij gehouden (lijkt me nuttiger zo).
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
#  - checkt of tweakers.net down is. als dit niet wordt gecheckt dan
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
# Het tweakers.tcl script werkt het best met TCL versies vanaf 8.1.
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
set tnet(proxy) ""

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set tnet(flags) "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set tnet(nopub) "" 

# de triggers: [scheiden met een spatie]
set tnet(triggers) "!tnet !tweakers"

# flood protectie: aantal seconden tussen gebruik van de triggers
# voor geen flood protectie: zet 't op 0
set tnet(antiflood) 10

# stuur berichten public of private wanneer er een trigger wordt gebruikt? 
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set tnet(method) 1

# aantal headlines weergeven wanneer een trigger wordt gebruikt. [>1] 
set tnet(headlines) 2

# om de hoeveel minuten checken of er nieuws is? [minimaal 5]
# deze waarde wordt gebruikt door zowel de triggers als het autonews.
set tnet(updates) 5

# hieronder kun je de layout aanpassen voor de output:
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
set tnet(autonews) 0

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
set tnet(autonewschan) "#kanaal1 #kanaal2"

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

package require http

set tnet(version) "2.0"

if {[info tclversion] < 8.1} {
  putlog "\[T.Net\] Kan [file tail [info script]] niet laden: U heeft minimaal TCL versie 8.1 nodig en u heeft TCL versie [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[T.Net\] Kan [file tail [info script]] niet laden: Zorg ervoor dat alltools.tcl in uw eggdrop configuratie wordt geladen!"
  return 1
}

set whichtimer [timerexists "tnet:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

for {set i 0} {$i < [llength $tnet(triggers)]} {incr i} {
  bind pub $tnet(flags) [lindex $tnet(triggers) $i] tnet:pub
  if {$tnet(log)} { putlog "\[T.Net\] Trigger [lindex $tnet(triggers) $i] added." }
}
catch { unset i }

bind pub $tnet(autotriggerflag) $tnet(autofftrigger) tnet:autoff
bind pub $tnet(autotriggerflag) $tnet(autontrigger) tnet:auton

proc tnet:getdata {} {
  global tnet tnetdata

  if {$tnet(log)} { putlog "\[T.Net\] Updating data..." }

  set url "http://www.tweakers.net/turbotracker.dsp"
  set page [::http::config -useragent "Mozilla"]

  if {$tnet(proxy) != ""} {
    if {![regexp {(.+):([0-9].*?)} $tnet(proxy) t proxyhost proxyport]} {
      putlog "\[T.Net\] Wrong proxy configuration ($tnet(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[T.Net\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[T.Net\] Problem: [::http::status $page]"
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[T.Net\] Problem: [::http::code $page]"
    return -1
  }

  if {[info exists tnetdata]} { unset tnetdata }

  set lines [split [::http::data $page] \n]
  set count 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regsub -all "\\&" $line "\\\\&" line
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

  set tnet(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg lines count line trash }

  return 0
}

proc tnet:pub {nick uhost hand chan text} {
  global lastbind tnet tnetdata
  if {[lsearch -exact $tnet(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {[info exists tnet(floodprot,$chan)]} {
    set verschil [expr [clock seconds] - $tnet(floodprot,$chan)]
    if {$verschil < $tnet(antiflood)} {
      putquick "NOTICE $nick :Trigger is net al gebruikt! Wacht aub. [expr $tnet(antiflood) - $verschil] seconden..."
      return 0
    }
  }
  set tnet(floodprot,$chan) [clock seconds]

  if {$tnet(log)} { putlog "\[T.Net\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists tnet(lastupdate)]} {
    if {[expr [clock seconds] - $tnet(lastupdate)] > [expr $tnet(updates) * 60]} {
      set ret [tnet:getdata]
    }
  } elseif {![info exists tnetdata(ts,0)]} {
    set ret [tnet:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $tnet(headlines)} {incr i} { 
      if {![info exists tnetdata(ts,$i)]} { break }
      tnet:put $chan $nick $i $tnet(method)
    }
  } else {
    putserv "NOTICE $nick :\[T.Net\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
  catch { unset ret verschil i }
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
  regsub -all "&euml;" $outchan "ë" outchan
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

proc tnet:update {} {
  global tnet tnetdata

  if {[tnet:getdata] != -1} {

    if {![info exists tnetdata(ts,0)]} {
      putlog "\[T.Net\] Something went wrong while updating..."
      return -1
    }

    if {![info exists tnet(lastitem)]} {
      set tnet(lastitem) $tnetdata(ts,0)
      if {$tnet(log)} { putlog "\[T.Net\] Last news item timestamp set to '$tnetdata(ts,0)'." }
    } else {
      if {$tnet(log)} { putlog "\[T.Net\] Last news item timestamp is '$tnetdata(ts,0)'." }
    }

    if {$tnetdata(ts,0) > $tnet(lastitem)} {
      if {$tnet(log)} { putlog "\[T.Net\] There's news!" }
      for {set i 0} {$i < $tnet(automax)} {incr i} {
        if {![info exists tnetdata(ts,$i)]} { break }
        if {$tnetdata(ts,$i) == $tnet(lastitem)} { break }
        foreach chan [split $tnet(autonewschan)] { tnet:put $chan $chan $i 1 }
        catch { unset chan }
      }
      catch { unset i }
    } else {
      if {$tnet(log)} { putlog "\[T.Net\] No news." } 
    }

    set tnet(lastitem) $tnetdata(ts,0)
  }

  if {$tnet(updates) < 5} {
    putlog "\[T.Net\] Warning: the \$tnet(updates) setting is too low! Defaulting to 5 minutes..."
    timer 5 tnet:update
  } else {
    timer $tnet(updates) tnet:update
  }

  return 0
}

proc tnet:autoff {nick uhost hand chan text} {
  global lastbind tnet
  if {[lsearch -exact $tnet(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$tnet(log)} { putlog "\[T.Net\] Trigger: $lastbind in $chan by $nick" }

  if {$tnet(autonews) == 1} {
    set tnet(autonews) 0;  catch { unset tnet(lastitem) }
    set whichtimer [timerexists "tnet:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
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

if {$tnet(autonews) == 1} { tnet:update }

putlog "\[T.Net\] Nieuws script versie $tnet(version): Loaded!"
