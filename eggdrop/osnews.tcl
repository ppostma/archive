# $Id: osnews.tcl,v 1.28 2003-08-09 15:16:57 peter Exp $

# OSnews.com News Announce Script for the eggdrop
# version 1.4, 09/08/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.4: (??/??/????)
#  - changed the update method somewhat.
#    the osnews(updates) it now also being used by the triggers
#    to check how long to cache the data.
#  - proxy configuration added.
#  - flood protection is now for each channel (this is more usefull IMHO).
#  - autonews /amsg optie toegevoegd.
#  - script works with Tcl 8.0 now.
# 1.3: (04/07/2003) [changes]
#  - check for correct Tcl version & alltools.tcl
#  - added flood protection.
#  - added url for latest version.
#  - trivial style changes.
# 1.2: (26/05/2003) [bugfix]
#  - third attempt to get rid of the bug with the & character.
# 1.1: (20/05/2003) [changes]
#  - strange bug with & character fixed.
#  - small changes to make the script more robust.
# 1.0: (18/05/2003) [first version]
#  - wrote this script, based on tweakers.tcl version 1.6
#
# This script makes use of a function from alltools.tcl.
# Please put alltools.tcl in your eggdrop configuration!
#
# This script also uses http.tcl. You *don't* need to put http.tcl
# your eggdrop configuration!
#
# You need at least Tcl version 8.0 to get this script running!
#
# For questions/suggestions/bug/etc: peter@webdeveloping.nl
# If you found spelling/grammatical errors, please also mail me!
#
# You can always find the latest version of this script on this page:
#   http://www.pointless.nl/?page=eggdrop
#
# Please change the configuration setting below:
#

### Configuration settings ###

# make use of a http proxy the get the data?
# enter the info like: "host.isp.com:port" or let it empty for no proxy
set osnews(proxy) ""

# the triggers: [seperate with spaces]
set osnews(triggers) "!osnews"

# flags needed to use the trigger [default=everyone]
set osnews(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set osnews(nopub) "" 

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set osnews(method) 1

# display n headlines when a trigger is used [> 1]
set osnews(headlines) 2

# flood protection: seconds between use of the triggers
# set this to 0 to disable the flood protection.
set osnews(antiflood) 10

# below you can change the layout of the output:
# %title = titel of article
# %link  = link to article
# %b   = bold text
# %u   = underlined text
set osnews(layout) "\[%bOSnews%b\] %title - %link"

# check for news after n minutes? [min. 30]
# this value is being used by the trigger and the autonews.
set osnews(updates) 60

# announce the news automaticly in the channels? [0=no / 1=yes]
set osnews(autonews) 0

# autonews: send to which channels? [seperate channels with spaces]
# use "*" to send news to all channels (/amsg).
set osnews(autonewschan) "#channel1"

# max. amount of messages which will be displayed meanwhile automatic updates.
# with this setting you can prevent channel flooding if the updates are high
# (e.g. longer than a hour).
set osnews(automax) 3

# trigger to turn off the autonews. [string]
set osnews(autontrigger) "!osnewson"

# trigger to turn on the autonews. [string]
set osnews(autofftrigger) "!osnewsoff"

# flags needed to use the autonews[on/off] trigger [default=master]
set osnews(autotriggerflag) "m|m"

# log additional information (debug) to the partyline? [0=no / 1=yes]
set osnews(log) 1

### End Configuration settings ###



### Begin Tcl code ###

set osnews(version) "1.4"

if {[catch { package require http } err]} {
  putlog "\[OSNews\] Cannot load [file tail [info script]]: Problem loading the http package: $err"
  return 1
}

if {[info tclversion] < 8.0} {
  putlog "\[OSNews\] Cannot load [file tail [info script]]: You need at least Tcl version 8.0 and you have Tcl version [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[OSNews\] Cannot load [file tail [info script]]: Please load alltools.tcl in your eggdrop configuration!"
  return 1
}

set whichtimer [timerexists "osnews:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

foreach trigger [split $osnews(triggers)] {
  bind pub $osnews(flags) $trigger osnews:pub
  if {$osnews(log)} { putlog "\[OSnews\] Trigger $trigger added." }
}
catch { unset trigger }

if {$osnews(autofftrigger) != ""} { bind pub $osnews(autotriggerflag) $osnews(autofftrigger) osnews:autoff }
if {$osnews(autontrigger)  != ""} { bind pub $osnews(autotriggerflag) $osnews(autontrigger) osnews:auton }

proc osnews:getdata {} {
  global osnews osnewsdata

  if {$osnews(log)} { putlog "\[OSnews\] Updating data..." }

  set url "http://www.osnews.com/files/recent.rdf"
  set page [::http::config -useragent "Mozilla"]

  if {$osnews(proxy) != ""} {
    if {![regexp {(.+):([0-9]+)} $osnews(proxy) trash proxyhost proxyport]} {
      putlog "\[Clanbase\] Wrong proxy configuration ($osnews(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch { set page [::http::geturl $url -timeout 15000] } msg]} {
    putlog "\[OSnews\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[OSnews\] Problem: [::http::status $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[OSnews\] Problem: [::http::code $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[catch { set data [::http::data $page] } msg]} {
    putlog "\[OSnews\] Problem: $msg"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[info exists osnewsdata]} { unset osnewsdata }

  set count 0
  set item 0
  foreach line [split $data \n] {
    regsub -all "\\&" $line "\\\\&" line
    if {[regexp "<item>" $line]} { set item 1 }
    if {[regexp "</item>" $line]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*)</title>" $line trash osnewsdata(title,$count)
      if {[regexp "<link>(.*)</link>" $line trash osnewsdata(link,$count)]} { incr count }
    }
  }

  set osnews(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg data count item line trash }

  return 0
}

proc osnews:pub {nick uhost hand chan text} {
  global lastbind osnews osnewsdata
  if {[lsearch -exact $osnews(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$osnews(antiflood) > 0} {
    if {[info exists osnews(floodprot,$chan)]} {
      set diff [expr [clock seconds] - $osnews(floodprot,$chan)]
      if {$diff < $osnews(antiflood)} {
        putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $osnews(antiflood) - $diff] seconds..."
        return 0
      }
    }
    set osnews(floodprot,$chan) [clock seconds]
  }

  if {$osnews(log)} { putlog "\[OSnews\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists osnews(lastupdate)]} {
    if {[expr [clock seconds] - $osnews(lastupdate)] >= [expr $osnews(updates) * 60]} {
      set ret [osnews:getdata]
    }
  } elseif {![info exists osnewsbdata(title,0)]} {
    set ret [osnews:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $osnews(headlines)} {incr i} {
      if {![info exists osnewsdata(title,$i)]} { break }
      osnews:put $chan $nick $i $osnews(method)
    }
  } else {
    putserv "NOTICE $nick :\[OSnews\] Something went wrong while updating."
  }
  catch { unset ret diff i }
}

proc osnews:put {chan nick which method} {
  global osnews osnewsdata

  set outchan $osnews(layout)
  regsub -all "%title" $outchan $osnewsdata(title,$which) outchan
  regsub -all "%link"  $outchan $osnewsdata(link,$which) outchan
  regsub -all "&amp;"  $outchan "\\&" outchan
  regsub -all "&quot;" $outchan "\"" outchan
  regsub -all "%b"   $outchan "\002" outchan
  regsub -all "%u"   $outchan "\037" outchan
  switch $method {
    0 { putserv "PRIVMSG $nick :$outchan" }
    1 { putserv "PRIVMSG $chan :$outchan" }
    2 { putserv "NOTICE $nick :$outchan" }
    3 { putserv "NOTICE $chan :$outchan" }
    default { putserv "PRIVMSG $chan :$outchan" }
  }
  catch { unset outchan }
}

proc osnews:update {} {
  global osnews osnewsdata

  if {[osnews:getdata] != -1} {

    if {![info exists osnewsdata(link,0)]} {
      putlog "\[OSnews\] Something went wrong while updating..."
      return -1
    }

    if {![info exists osnews(lastitem)]} {
      set osnews(lastitem) $osnewsdata(link,0)
      if {$osnews(log)} { putlog "\[OSnews\] Last news item set to '$osnewsdata(link,0)'." }
    } else {
      if {$osnews(log)} { putlog "\[OSnews\] Last news item is '$osnewsdata(link,0)'." }
    }

    if {$osnewsdata(link,0) != $osnews(lastitem)} {
      if {$osnews(log)} { putlog "\[OSnews\] There's news!" }
      if {[regexp {^\*$} $osnews(autonewschan)]} {
        set dest [channels]
      } else {
        set dest $osnews(autonewschan)
      }
      for {set i 0} {$i < $osnews(automax)} {incr i} {
        if {![info exists osnewsdata(link,$i)]} { break }
        if {$osnewsdata(link,$i) == $osnews(lastitem)} { break }
        foreach chan [split $dest] { osnews:put $chan $chan $i 1 }
      }
      catch { unset dest i chan }
    } else {
      if {$osnews(log)} { putlog "\[OSnews\] No news." }
    }

    set osnews(lastitem) $osnewsdata(link,0)
  }

  if {$osnews(updates) < 30} {
    putlog "\[OSnews\] Warning: the \$osnews(updates) setting is too low! Defaulting to 30 minutes..."
    timer 30 osnews:update
  } else {
    timer $osnews(updates) osnews:update
  }

  return 0
}

proc osnews:autoff {nick uhost hand chan text} {
  global lastbind osnews
  if {[lsearch -exact $osnews(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$osnews(log)} { putlog "\[OSnews\] Trigger: $lastbind in $chan by $nick" }

  if {$osnews(autonews) == 1} {
    set osnews(autonews) 0;  catch { unset osnews(lastitem) }
    set whichtimer [timerexists "osnews:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[OSnews\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION has turned the osnews.org news announcer off.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already off!"
  }
}

proc osnews:auton {nick uhost hand chan text} {
  global lastbind osnews
  if {[lsearch -exact $osnews(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$osnews(log)} { putlog "\[OSnews\] Trigger: $lastbind in $chan by $nick" }

  if {$osnews(autonews) == 0} {
    set osnews(autonews) 1;  osnews:update
    putlog "\[OSnews\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION has turned the osnews.org news announcer on.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already on!"
  }
}

if {$osnews(autonews) == 1} { osnews:update }

putlog "\[OSnews\] News Announcer version $osnews(version): Loaded!"
