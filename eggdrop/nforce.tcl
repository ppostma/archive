# $Id: nforce.tcl,v 1.1 2004-08-14 15:39:52 peter Exp $

# Nforce.nl releases script voor de eggdrop
# version 1.0, 14/08/2004, door Peter Postma <peter@webdeveloping.nl>
#
# Dit script maakt gebruik van een functie uit alltools.tcl.
# Zorg ervoor dat alltools.tcl geladen wordt in je eggdrop configuratie!
#
# Dit script gebruikt ook http.tcl. Deze moet op je systeem aanwezig zijn.
# Zet http.tcl *niet* in je eggdrop configuratie!
#
# Het nforce.tcl script werkt het best met Tcl versies vanaf 8.0.
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
set nforce(proxy) ""

# de triggers: [scheiden met een spatie]
set nforce(triggers) "!nforce"

# benodigde flags om de triggers te kunnen gebruiken. [default=iedereen]
set nforce(flags) "-|-"

# kanalen waar de bot niet op de triggers zal reageren [scheiden met spatie]
set nforce(nopub) "" 

# stuur berichten public of private wanneer er een trigger wordt gebruikt?
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set nforce(method) 1

# aantal releases weergeven wanneer een trigger wordt gebruikt. [> 1]
set nforce(releases) 2

# flood protectie: aantal seconden tussen gebruik van de triggers
# zet 't op 0 om de flood protectie uit te zetten.
set nforce(antiflood) 10

# hieronder kun je de layout aanpassen voor de output:
# %date    = datum
# %name    = naam van de release
# %group   = naam van de group
# %section = sectie naam / categorie
# %size    = aantal bestanden + MB
# %orig    = originele NFO naam
# %nfo     = NFO url
# %site    = site url
# %review  = review url
# %b       = bold (dikgedrukte) tekst
# %u       = underlined (onderstreepte) tekst
set nforce(layout) "\[%bNFOrce%b\] %date - %name (%section) - %nfo"

# om de hoeveel minuten checken of er nieuws is? [minimaal 5]
# deze waarde wordt gebruikt door zowel de triggers als het autonews.
set nforce(updates) 20

# het nieuws automatisch weergeven in de kanalen? [0=nee / 1=ja] 
set nforce(autonews) 0

# autonews: stuur naar welke kanalen? [kanalen scheiden met een spatie]
# gebruik "*" om het nieuws naar alle kanalen te sturen (/amsg).
set nforce(autonewschan) "#channel"

# maximaal aantal berichten die worden getoond tijdens de automatische updates.
# hiermee kan je voorkomen dat de channel wordt ondergeflood als je de 
# updates hoog hebt staan (bv. langer dan een uur).
set nforce(automax) 3

# trigger om het autonews aan te zetten. [string]
set nforce(autontrigger) "!nforceon"

# trigger om het autonews uit te zetten. [string]
set nforce(autofftrigger) "!nforceoff"

# benodigde flags om de autonews[on/off] triggers te gebruiken [default=master]
set nforce(autotriggerflag) "m|m"

# log extra informatie (debug) naar de partyline? [0=nee / 1=ja]
set nforce(log) 1

### Eind configuratie instellingen ###



### Begin Tcl code ###

set nforce(version) "1.0"

if {[catch { package require http } err]} {
  putlog "\[NFOrce\] Kan [file tail [info script]] niet laden: Probleem met het laden van de http package: $err"
  return 1
}

if {[info tclversion] < 8.0} {
  putlog "\[NFOrce\] Kan [file tail [info script]] niet laden: U heeft minimaal Tcl versie 8.0 nodig en u heeft Tcl versie [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[NFOrce\] Kan [file tail [info script]] niet laden: Zorg ervoor dat alltools.tcl in uw eggdrop configuratie wordt geladen!"
  return 1
}

set whichtimer [timerexists "nforce:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

foreach trigger [split $nforce(triggers)] {
  bind pub $nforce(flags) $trigger nforce:pub
  if {$nforce(log)} { putlog "\[NFOrce\] Trigger $trigger added." }
}
catch { unset trigger }

if {$nforce(autofftrigger) != ""} { bind pub $nforce(autotriggerflag) $nforce(autofftrigger) nforce:autoff }
if {$nforce(autontrigger)  != ""} { bind pub $nforce(autotriggerflag) $nforce(autontrigger) nforce:auton }

proc nforce:getdata {} {
  global nforce nforcedata

  if {$nforce(log)} { putlog "\[NFOrce\] Updating data..." }

  set url "http://www.nforce.nl/rss/bots_last-10.php"
  set page [::http::config -useragent "Mozilla"]

  if {$nforce(proxy) != ""} {
    if {![regexp {(.+):([0-9]+)} $nforce(proxy) trash proxyhost proxyport]} {
      putlog "\[NFOrce\] Wrong proxy configuration ($nforce(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch { set page [::http::geturl $url -timeout 20000] } msg]} {
    putlog "\[NFOrce\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[NFOrce\] Problem: [::http::status $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[NFOrce\] Problem: [::http::code $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[catch { set data [::http::data $page] } msg]} {
    putlog "\[NFOrce\] Problem: $msg"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[info exists nforcedata]} { unset nforcedata }

  regsub -all "<br>" $data "\n" data

  set count 0
  foreach line [split $data "\n"] {

    regsub -all "\\&" $line "\\\\&" line
    set ldata [split $line ";"]

    set nforcedata(date,$count) [lindex $ldata 0]
    set nforcedata(name,$count) [lindex $ldata 1]
    set nforcedata(group,$count) [lindex $ldata 2]
    set nforcedata(section,$count) [lindex $ldata 3]
    set nforcedata(size,$count) [lindex $ldata 4]
    set nforcedata(orig,$count) [lindex $ldata 5]
    set nforcedata(nfo,$count) [lindex $ldata 6]
    set nforcedata(site,$count) [lindex $ldata 7]
    set nforcedata(review,$count) [lindex $ldata 8]

    incr count
  }

  set nforce(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg data count line trash }

  return 0
}

proc nforce:pub {nick uhost hand chan text} {
  global lastbind nforce nforcedata
  if {[lsearch -exact $nforce(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$nforce(antiflood) > 0} {
    if {[info exists nforce(floodprot,$chan)]} {
      set diff [expr [clock seconds] - $nforce(floodprot,$chan)]
      if {$diff < $nforce(antiflood)} {
        putquick "NOTICE $nick :Trigger is net al gebruikt! Wacht aub. [expr $nforce(antiflood) - $diff] seconden..."
        return 0
      }
    }
    set nforce(floodprot,$chan) [clock seconds]
  }

  if {$nforce(log)} { putlog "\[NFOrce\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists nforce(lastupdate)]} {
    if {[expr [clock seconds] - $nforce(lastupdate)] >= [expr $nforce(updates) * 60]} {
      set ret [nforce:getdata]
    }
  } elseif {![info exists nforcedata(nfo,0)]} {
    set ret [nforce:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $nforce(releases)} {incr i} { 
      if {![info exists nforcedata(nfo,$i)]} { break }
      if {[catch { nforce:put $chan $nick $i $nforce(method) } err]} {
        putlog "\[NFOrce\] Problem in data array: $err"
      }
    }
  } else {
    putserv "NOTICE $nick :\[NFOrce\] Er ging iets fout tijdens het ophalen van de gegevens."
  }
  catch { unset ret diff i err }
}

proc nforce:put {chan nick which method} {
  global nforce nforcedata

  set outchan $nforce(layout)
  regsub -all "%date" $outchan $nforcedata(date,$which) outchan
  regsub -all "%name" $outchan $nforcedata(name,$which) outchan
  regsub -all "%group" $outchan $nforcedata(group,$which) outchan
  regsub -all "%section" $outchan $nforcedata(section,$which) outchan
  regsub -all "%size" $outchan $nforcedata(size,$which) outchan
  regsub -all "%orig" $outchan $nforcedata(orig,$which) outchan
  regsub -all "%nfo" $outchan $nforcedata(nfo,$which) outchan
  regsub -all "%site" $outchan $nforcedata(site,$which) outchan
  regsub -all "%review" $outchan $nforcedata(review,$which) outchan
  regsub -all "&euml;" $outchan "ë" outchan
  regsub -all "&amp;" $outchan "\\&" outchan
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

proc nforce:update {} {
  global nforce nforcedata

  if {[nforce:getdata] != -1} {

    if {![info exists nforcedata(nfo,0)]} {
      putlog "\[NFOrce\] Something went wrong while updating..."
      return -1
    }

    if {![info exists nforce(lastitem)]} {
      set nforce(lastitem) $nforcedata(nfo,0)
      if {$nforce(log)} { putlog "\[NFOrce\] Last release set to '$nforcedata(nfo,0)'." }
    } else {
      if {$nforce(log)} { putlog "\[NFOrce\] Last release is '$nforcedata(nfo,0)'." }
    }

    if {$nforcedata(nfo,0) > $nforce(lastitem)} {
      if {$nforce(log)} { putlog "\[NFOrce\] New release found." }
      if {[regexp {^\*$} $nforce(autonewschan)]} {
        set dest [channels]
      } else {
        set dest $nforce(autonewschan)
      }
      for {set i 0} {$i < $nforce(automax)} {incr i} {
        if {![info exists nforcedata(nfo,$i)]} { break }
        if {$nforcedata(nfo,$i) == $nforce(lastitem)} { break }
        foreach chan [split $dest] {
          if {[catch { nforce:put $chan $chan $i 1 } err]} {
            putlog "\[NFOrce\] Problem in data array: $err"
          }
        }
      }
      catch { unset dest i chan err }
    } else {
      if {$nforce(log)} { putlog "\[NFOrce\] No new releases." }
    }

    set nforce(lastitem) $nforcedata(nfo,0)
  }

  if {$nforce(updates) < 15} {
    putlog "\[NFOrce\] Warning: the \$nforce(updates) setting is too low! Defaulting to 15 minutes..."
    timer 15 nforce:update
  } else {
    timer $nforce(updates) nforce:update
  }

  return 0
}

proc nforce:autoff {nick uhost hand chan text} {
  global lastbind nforce
  if {[lsearch -exact $nforce(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$nforce(log)} { putlog "\[NFOrce\] Trigger: $lastbind in $chan by $nick" }

  if {$nforce(autonews) == 1} {
    set nforce(autonews) 0;  catch { unset nforce(lastitem) }
    set whichtimer [timerexists "nforce:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[NFOrce\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION heeft de nforce.nl release aankondiger uitgezet.\001"
  } else {
    putserv "NOTICE $nick :De nforce.nl release aankondiger staat al uit."
  }
}

proc nforce:auton {nick uhost hand chan text} {
  global lastbind nforce
  if {[lsearch -exact $nforce(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$nforce(log)} { putlog "\[NFOrce\] Trigger: $lastbind in $chan by $nick" }

  if {$nforce(autonews) == 0} {
    set nforce(autonews) 1;  nforce:update
    putlog "\[NFOrce\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION heeft de nforce.nl release aankondiger aangezet.\001"
  } else {
    putserv "NOTICE $nick :De nforce.nl release aankondiger staat al aan."
  }
}

if {$nforce(autonews) == 1} { nforce:update }

putlog "\[NFOrce\] Release announcer version $nforce(version): Loaded!"
