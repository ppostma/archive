# $Id: clanbase.tcl,v 1.22 2003-07-10 09:56:06 peter Exp $

# Clanbase.com News Announce Script for the eggdrop
# version 1.4, 10/07/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.4: (??/??/????)
#  - changed the update method somewhat.
#    the cb(updates) it now also being used by the triggers
#    to check how long to cache the data.
#  - proxy configuration added.
#  - flood protection is now for each channel (this is more usefull IMHO).
#  - script works with TCL 8.0 now.
# 1.3: (04/07/2003) [changes]
#  - check for correct TCL version & alltools.tcl
#  - added flood protection.
#  - added url for latest version.
#  - trivial code style changes.
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
# You need at least TCL version 8.0 to get this script running!
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
set cb(proxy) ""

# flags needed to use the trigger [default=everyone]
set cb(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set cb(nopub) "" 

# the triggers: [seperate with spaces]
set cb(triggers) "!cb !clanbase"

# flood protection: seconds between use of the triggers
# to disable: set it to 0
set cb(antiflood) 10

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set cb(method) 1

# display n headlines when a trigger is used [> 1]
set cb(headlines) 2

# check for news after n minutes? [min. 30]
# this value is being used by the trigger and the autonews.
set cb(updates) 60

# below you can change the layout of the output:
# %title = title from article
# %link  = link to cb's news item
# %b   = bold text
# %u   = underlined text
set cb(layout) "\[%bClanbase%b\] %title - %link"

# announce the news automaticly in the channels? [0=no / 1=yes]
set cb(autonews) 0

# autonews: send to which channels? [seperate channels with spaces]
set cb(autonewschan) "#channel"

# max. amount of messages which will be displayed meanwhile automatic updates.
# with this setting you can prevent channel flooding if the updates are high
# (e.g. longer than a hour).
set cb(automax) 3

# trigger to turn off the autonews. [string]
set cb(autontrigger) "!cbon"

# trigger to turn on the autonews. [string]
set cb(autofftrigger) "!cboff"

# flags needed to use the autonews[on/off] trigger [default=master]
set cb(autotriggerflag) "m|m"

# log additional information (debug) to the partyline? [0=no / 1=yes]
set cb(log) 1

### End Configuration settings ###



### Begin TCL code ###

package require http

set cb(version) "1.4"

if {[info tclversion] < 8.0} {
  putlog "\[Clanbase\] Cannot load [file tail [info script]]: You need at least TCL version 8.0 and you have TCL version [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[Clanbase\] Cannot load [file tail [info script]]: Please load alltools.tcl in your eggdrop configuration!"
  return 1
}

set whichtimer [timerexists "cb:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

foreach trigger [split $cb(triggers)] {
  bind pub $cb(flags) $trigger cb:pub
  if {$cb(log)} { putlog "\[Clanbase\] Trigger $trigger added." }
}
catch { unset trigger }

bind pub $cb(autotriggerflag) $cb(autofftrigger) cb:autoff
bind pub $cb(autotriggerflag) $cb(autontrigger) cb:auton

proc cb:getdata {} {
  global cb cbdata

  if {$cb(log)} { putlog "\[Clanbase\] Updating data..." }

  set url "http://www.clanbase.com/rss.php"
  set page [::http::config -useragent "Mozilla"]

  if {$cb(proxy) != ""} {
    if {![regexp {(.+):([0-9]+)} $cb(proxy) trash proxyhost proxyport]} {
      putlog "\[Clanbase\] Wrong proxy configuration ($cb(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch {set page [::http::geturl $url -timeout 30000]} msg]} {
    putlog "\[Clanbase\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[Clanbase\] Problem: [::http::status $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Clanbase\] Problem: [::http::code $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[info exists cbdata]} { unset cbdata }

  set count 0
  set item 0
  foreach line [split [::http::data $page] \n] {
    regsub -all "\\&" $line "\\\\&" line
    if {[regexp "<item>" $line]} { set item 1 }
    if {[regexp "</item>" $line]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*)</title>" $line trash cbdata(title,$count)
      if {[regexp "<link>(.*)</link>" $line trash cbdata(link,$count)]} { incr count }
    }
  }

  set cb(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg count item line trash}

  return 0
}

proc cb:pub {nick uhost hand chan text} {
  global lastbind cb cbdata
  if {[lsearch -exact $cb(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {[info exists cb(floodprot,$chan)]} {
    set diff [expr [clock seconds] - $cb(floodprot,$chan)]
    if {$diff < $cb(antiflood)} {
      putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $cb(antiflood) - $diff] seconds..."
      return 0
    }
  }
  set cb(floodprot,$chan) [clock seconds]

  if {$cb(log)} { putlog "\[Clanbase\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists cb(lastupdate)]} {
    if {[expr [clock seconds] - $cb(lastupdate)] >= [expr $cb(updates) * 60]} {
      set ret [cb:getdata]
    }
  } elseif {![info exists cbdata(title,0)]} {
    set ret [cb:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $cb(headlines)} {incr i} {
      if {![info exists cbdata(title,$i)]} { break }
      cb:put $chan $nick $i $cb(method)
    }
  } else {
    putserv "NOTICE $nick :\[Clanbase\] Something went wrong while updating."
  }
  catch { unset ret diff i }
}

proc cb:put {chan nick which method} {
  global cb cbdata

  set outchan $cb(layout)
  regsub -all "%title" $outchan $cbdata(title,$which) outchan
  regsub -all "%link"  $outchan $cbdata(link,$which) outchan
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
  catch { unset item outchan }
}

proc cb:update {} {
  global cb cbdata

  if {[cb:getdata] != -1} {

    if {![info exists cbdata(title,0)]} {
      putlog "\[Clanbase\] Something went wrong while updating..."
      return -1
    }

    if {![info exists cb(lastitem)]} {
      set cb(lastitem) $cbdata(title,0)
      if {$cb(log)} { putlog "\[Clanbase\] Last news item set to '$cbdata(title,0)'." }
    } else {
      if {$cb(log)} { putlog "\[Clanbase\] Last news item is '$cbdata(title,0)'." }
    }

    if {$cbdata(title,0) != $cb(lastitem)} {
      if {$cb(log)} { putlog "\[Clanbase\] There's news!" }
      for {set i 0} {$i < $cb(automax)} {incr i} {
        if {![info exists cbdata(title,$i)]} { break }
        if {$cbdata(title,$i) == $cb(lastitem)} { break }
        foreach chan [split $cb(autonewschan)] { cb:put $chan $chan $i 1 }
      }
    } else {
      if {$cb(log)} { putlog "\[Clanbase\] No news." } 
    }

    set cb(lastitem) $cbdata(title,0)
  }

  if {$cb(updates) < 30} {
    putlog "\[Clanbase\] Warning: the \$cb(updates) setting is too low! Defaulting to 30 minutes..."
    timer 30 cb:update
  } else {
    timer $cb(updates) cb:update
  }

  catch { unset i chan }
  return 0
}

proc cb:autoff {nick uhost hand chan text} {
  global lastbind cb
  if {[lsearch -exact $cb(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$cb(log)} { putlog "\[Clanbase\] Trigger: $lastbind in $chan by $nick" }

  if {$cb(autonews) == 1} {
    set cb(autonews) 0;  catch { unset cb(lastitem) }
    set whichtimer [timerexists "cb:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[Clanbase\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION has turned the clanbase.com news announcer off.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already off!"
  }
}

proc cb:auton {nick uhost hand chan text} {
  global lastbind cb
  if {[lsearch -exact $cb(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$cb(log)} { putlog "\[Clanbase\] Trigger: $lastbind in $chan by $nick" }

  if {$cb(autonews) == 0} {
    set cb(autonews) 1;  cb:update
    putlog "\[Clanbase\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION has turned the clanbase.com news announcer on.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already on!"
  }
}

if {$cb(autonews) == 1} { cb:update }

putlog "\[Clanbase\] News Announcer version $cb(version): Loaded!"
