# $Id: slashdot.tcl,v 1.31 2003-08-18 20:36:43 peter Exp $

# Slashdot.org News Announce Script for the eggdrop
# version 2.0, 09/08/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 2.0: (??/??/????)
#  - changed the update method somewhat.
#    the slashdot(updates) it now also being used by the triggers
#    to check how long to cache the data.
#  - proxy configuration added.
#  - flood protection is now for each channel (this is more usefull IMHO).
#  - autonews /amsg optie toegevoegd.
#  - script works with Tcl 8.0 now.
# 1.9: (04/07/2003) [changes]
#  - check for correct Tcl version & alltools.tcl
#  - added flood protection.
#  - added url for latest version.
#  - trivial style changes.
# 1.8: (26/05/2003) [bugfix]
#  - third attempt to get rid of the bug with the & character.
# 1.7: (20/05/2003) [changes]
#  - strange bug with & character fixed.
#  - small changes to make the script more robust.
# 1.6: (17/05/2003) [first version]
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
set slashdot(proxy) ""

# the triggers: [seperate with spaces]
set slashdot(triggers) "!slashdot /."

# flags needed to use the trigger [default=everyone]
set slashdot(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set slashdot(nopub) "" 

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set slashdot(method) 1

# display n headlines when a trigger is used [> 1]
set slashdot(headlines) 2

# flood protection: seconds between use of the triggers
# set this to 0 to disable the flood protection.
set slashdot(antiflood) 10

# below you can change the layout of the output:
# %tim = time
# %aut = author
# %sec = section
# %tit = title
# %url = url to slashdot's news item
# %com = comments
# %b   = bold text
# %u   = underlined text
set slashdot(layout) "\[%bSlashdot%b\] (%sec) %tit - %url"

# check for news after n minutes? [min. 30]
# this value is being used by the trigger and the autonews.
set slashdot(updates) 60

# announce the news automaticly in the channels? [0=no / 1=yes]
set slashdot(autonews) 0

# autonews: send to which channels? [seperate channels with spaces]
# use "*" to send news to all channels (/amsg).
set slashdot(autonewschan) "#channel1 #channel2"

# max. amount of messages which will be displayed meanwhile automatic updates.
# with this setting you can prevent channel flooding if the updates are high
# (e.g. longer than a hour).
set slashdot(automax) 3

# trigger to turn off the autonews. [string]
set slashdot(autontrigger) "!slashdoton"

# trigger to turn on the autonews. [string]
set slashdot(autofftrigger) "!slashdotoff"

# flags needed to use the autonews[on/off] trigger [default=master]
set slashdot(autotriggerflag) "m|m"

# log additional information (debug) to the partyline? [0=no / 1=yes]
set slashdot(log) 1

### End Configuration settings ###



### Begin Tcl code ###

set slashdot(version) "2.0"

if {[catch { package require http } err]} {
  putlog "\[Slashdot\] Cannot load [file tail [info script]]: Problem loading the http package: $err"
  return 1
}

if {[info tclversion] < 8.0} {
  putlog "\[Slashdot\] Cannot load [file tail [info script]]: You need at least Tcl version 8.0 and you have Tcl version [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[Slashdot\] Cannot load [file tail [info script]]: Please load alltools.tcl in your eggdrop configuration!"
  return 1
}

set whichtimer [timerexists "slashdot:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

foreach trigger [split $slashdot(triggers)] {
  bind pub $slashdot(flags) $trigger slashdot:pub
  if {$slashdot(log)} { putlog "\[Slashdot\] Trigger $trigger added." }
}
catch { unset trigger }

if {$slashdot(autofftrigger) != ""} { bind pub $slashdot(autotriggerflag) $slashdot(autofftrigger) slashdot:autoff }
if {$slashdot(autontrigger)  != ""} { bind pub $slashdot(autotriggerflag) $slashdot(autontrigger) slashdot:auton }

proc slashdot:getdata {} {
  global slashdot slashdotdata

  if {$slashdot(log)} { putlog "\[Slashdot\] Updating data..." }

  set url "http://slashdot.org/slashdot.xml"
  set page [::http::config -useragent "Mozilla"]

  if {$slashdot(proxy) != ""} {
    if {![regexp {(.+):([0-9]+)} $slashdot(proxy) trash proxyhost proxyport]} {
      putlog "\[Slashdot\] Wrong proxy configuration ($slashdot(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch { set page [::http::geturl $url -timeout 15000] } msg]} {
    putlog "\[Slashdot\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[Slashdot\] Problem: [::http::status $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Slashdot\] Problem: [::http::code $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[catch { set data [::http::data $page] } msg]} {
    putlog "\[Slashdot\] Problem: $msg"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[info exists slashdotdata]} { unset slashdotdata }

  set count 0
  foreach line [split $data \n] {
    regsub -all "\\&" $line "\\\\&" line
    regexp "<title>(.*)</title>" $line trash slashdotdata(title,$count)
    regexp "<url>(.*)</url>" $line trash slashdotdata(url,$count)
    regexp "<time>(.*)</time>" $line trash slashdotdata(time,$count)
    regexp "<author>(.*)</author>" $line trash slashdotdata(author,$count)
    regexp "<comments>(.*)</comments>" $line trash slashdotdata(comments,$count)
    if {[regexp "<section>(.*)</section>" $line trash slashdotdata(section,$count)]} { incr count }
  }

  set slashdot(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg data count line trash }

  return 0
}

proc slashdot:pub {nick uhost hand chan text} {
  global lastbind slashdot slashdotdata
  if {[lsearch -exact $slashdot(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$slashdot(antiflood) > 0} {
    if {[info exists slashdot(floodprot,$chan)]} {
      set diff [expr [clock seconds] - $slashdot(floodprot,$chan)]
      if {$diff < $slashdot(antiflood)} {
        putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $slashdot(antiflood) - $diff] seconds..."
        return 0
      }
    }
    set slashdot(floodprot,$chan) [clock seconds]
  }

  if {$slashdot(log)} { putlog "\[Slashdot\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists slashdot(lastupdate)]} {
    if {[expr [clock seconds] - $slashdot(lastupdate)] >= [expr $slashdot(updates) * 60]} {
      set ret [slashdot:getdata]
    }
  } elseif {![info exists slashdotdata(title,0)]} {
    set ret [slashdot:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $slashdot(headlines)} {incr i} {
      if {![info exists slashdotdata(time,$i)]} { break }
      if {[catch { slashdot:put $chan $nick $i $slashdot(method) } err]} {
        putlog "\[Slashdot\] Problem in data array: $err"
      }
    }
  } else {
    putserv "NOTICE $nick :\[Slashdot\] Something went wrong while updating."
  }
  catch { unset ret diff i err }
}

proc slashdot:put {chan nick which method} {
  global slashdot slashdotdata

  set outchan $slashdot(layout)
  regsub -all "%tim" $outchan $slashdotdata(time,$which) outchan
  regsub -all "%aut" $outchan $slashdotdata(author,$which) outchan
  regsub -all "%tit" $outchan $slashdotdata(title,$which) outchan
  regsub -all "%sec" $outchan $slashdotdata(section,$which) outchan
  regsub -all "%url" $outchan $slashdotdata(url,$which) outchan
  regsub -all "%com" $outchan $slashdotdata(comments,$which) outchan
  regsub -all "&amp;amp;" $outchan "\\&" outchan
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

proc slashdot:update {} {
  global slashdot slashdotdata

  if {[slashdot:getdata] != -1} {

    if {![info exists slashdotdata(time,0)]} {
      putlog "\[Slashdot\] Something went wrong while updating..."
      return -1
    }

    if {![info exists slashdot(lastitem)]} {
      set slashdot(lastitem) $slashdotdata(time,0)
      if {$slashdot(log)} { putlog "\[Slashdot\] Last news item time set to '$slashdotdata(time,0)'." }
    } else {
      if {$slashdot(log)} { putlog "\[Slashdot\] Last news item time is '$slashdotdata(time,0)'." }
    }

    if {$slashdotdata(time,0) > $slashdot(lastitem)} {
      if {$slashdot(log)} { putlog "\[Slashdot\] There's news!" }
      if {[regexp {^\*$} $slashdot(autonewschan)]} {
        set dest [channels]
      } else {
        set dest $slashdot(autonewschan)
      }
      for {set i 0} {$i < $slashdot(automax)} {incr i} {
        if {![info exists slashdotdata(time,$i)]} { break }
        if {$slashdotdata(time,$i) == $slashdot(lastitem)} { break }
        foreach chan [split $dest] {
          if {[catch { slashdot:put $chan $chan $i 1 } err]} {
            putlog "\[Slashdot\] Problem in data array: $err"
          }
        }
      }
      catch { unset dest i chan err }
    } else {
      if {$slashdot(log)} { putlog "\[Slashdot\] No news." }
    }

    set slashdot(lastitem) $slashdotdata(time,0)
  }

  if {$slashdot(updates) < 30} {
    putlog "\[Slashdot\] Warning: the \$slashdot(updates) setting is too low! Defaulting to 30 minutes..."
    timer 30 slashdot:update
  } else {
    timer $slashdot(updates) slashdot:update
  }

  return 0
}

proc slashdot:autoff {nick uhost hand chan text} {
  global lastbind slashdot
  if {[lsearch -exact $slashdot(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$slashdot(log)} { putlog "\[Slashdot\] Trigger: $lastbind in $chan by $nick" }

  if {$slashdot(autonews) == 1} {
    set slashdot(autonews) 0;  catch { unset slashdot(lastitem) }
    set whichtimer [timerexists "slashdot:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[Slashdot\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION has turned the slashdot.org news announcer off.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already off!"
  }
}

proc slashdot:auton {nick uhost hand chan text} {
  global lastbind slashdot
  if {[lsearch -exact $slashdot(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$slashdot(log)} { putlog "\[Slashdot\] Trigger: $lastbind in $chan by $nick" }

  if {$slashdot(autonews) == 0} {
    set slashdot(autonews) 1;  slashdot:update
    putlog "\[Slashdot\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION has turned the slashdot.org news announcer on.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already on!"
  }
}

if {$slashdot(autonews) == 1} { slashdot:update }

putlog "\[Slashdot\] News Announcer version $slashdot(version): Loaded!"
