# $Id: bsdforums.tcl,v 1.16 2003-08-02 14:21:05 peter Exp $

# BSDForums.org News Announce Script for the eggdrop
# version 1.2, 02/08/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.1: (??/??/????)
#  - changed the update method somewhat.
#    the bsdforums(updates) it now also being used by the triggers
#    to check how long to cache the data.
#  - proxy configuration added.
#  - flood protection is now for each channel (this is more usefull IMHO).
#  - script works with Tcl 8.0 now.
# 1.1: (04/07/2003)
#  - check for correct Tcl version & alltools.tcl
#  - added flood protection.
#  - added url for latest version. 
# 1.0: (20/06/2003) [first version]
#  - wrote the script, based on osnews.tcl version 1.3 (beta)
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
set bsdforums(proxy) ""

# the triggers: [seperate with spaces]
set bsdforums(triggers) "!bsdforums"

# flags needed to use the trigger [default=everyone]
set bsdforums(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set bsdforums(nopub) "" 

# method to send the messages:
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set bsdforums(method) 1

# display n headlines when a trigger is used [> 1]
set bsdforums(headlines) 2

# flood protection: seconds between use of the triggers
# set this to 0 to disable the flood protection.
set bsdforums(antiflood) 10

# below you can change the layout of the output:
# %title = titel of article
# %link  = link to article
# %b   = bold text
# %u   = underlined text
set bsdforums(layout) "\[%bBSDForums%b\] %title - %link"

# check for news after n minutes? [minimal 120]
# this value is being used by the trigger and the autonews.
set bsdforums(updates) 120

# announce the news automaticly in the channels? [0=no / 1=yes]
set bsdforums(autonews) 0

# autonews: send to which channels? [seperate channels with spaces]
set bsdforums(autonewschan) "#channel1"

# max. amount of messages which will be displayed meanwhile automatic updates.
# with this setting you can prevent channel flooding if the updates are high
# (e.g. longer than a hour).
set bsdforums(automax) 3

# trigger to turn off the autonews. [string]
set bsdforums(autontrigger) "!bsdforumson"

# trigger to turn on the autonews. [string]
set bsdforums(autofftrigger) "!bsdforumsoff"

# flags needed to use the autonews[on/off] trigger [default=master]
set bsdforums(autotriggerflag) "m|m"

# log additional information (debug) to the partyline? [0=no / 1=yes]
set bsdforums(log) 1

### End Configuration settings ###



### Begin Tcl code ###

package require http

set bsdforums(version) "1.2"

if {[info tclversion] < 8.0} {
  putlog "\[BSDForums\] Cannot load [file tail [info script]]: You need at least Tcl version 8.0 and you have Tcl version [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[BSDForums\] Cannot load [file tail [info script]]: Please load alltools.tcl in your eggdrop configuration!"
  return 1
}

set whichtimer [timerexists "bsdforums:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

foreach trigger [split $bsdforums(triggers)] {
  bind pub $bsdforums(flags) $trigger bsdforums:pub
  if {$bsdforums(log)} { putlog "\[BSDForums\] Trigger $trigger added." }
}
catch { unset trigger }

if {$bsdforums(autofftrigger) != ""} { bind pub $bsdforums(autotriggerflag) $bsdforums(autofftrigger) bsdforums:autoff }
if {$bsdforums(autontrigger)  != ""} { bind pub $bsdforums(autotriggerflag) $bsdforums(autontrigger) bsdforums:auton }

proc bsdforums:getdata {} {
  global bsdforums bsdforumsdata

  if {$bsdforums(log)} { putlog "\[BSDForums\] Updating data..." }

  set url "http://bsdforums.org/rss/news/rss.xml"
  set page [::http::config -useragent "Mozilla"]

  if {$bsdforums(proxy) != ""} {
    if {![regexp {(.+):([0-9]+)} $bsdforums(proxy) trash proxyhost proxyport]} {
      putlog "\[BSDForums\] Wrong proxy configuration ($bsdforums(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[BSDForums\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[BSDForums\] Problem: [::http::status $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[BSDForums\] Problem: [::http::code $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[info exists bsdforumsdata]} { unset bsdforumsdata }

  set count 0
  set item 0
  foreach line [split [::http::data $page] \n] {
    regsub -all "\\&" $line "\\\\&" line
    if {[regexp "<item .+>" $line]} { set item 1 }
    if {[regexp "</item>" $line]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*)</title>" $line trash bsdforumsdata(title,$count)
      if {[regexp "<link>(.*)</link>" $line trash bsdforumsdata(link,$count)]} { incr count }
    }
  }

  set bsdforums(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg count item line trash }

  return 0
}

proc bsdforums:pub {nick uhost hand chan text} {
  global lastbind bsdforums bsdforumsdata
  if {[lsearch -exact $bsdforums(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$bsdforums(antiflood) > 0} {
    if {[info exists bsdforums(floodprot,$chan)]} {
      set diff [expr [clock seconds] - $bsdforums(floodprot,$chan)]
      if {$diff < $bsdforums(antiflood)} {
        putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $bsdforums(antiflood) - $diff] seconds..."
        return 0
      }
    }
    set bsdforums(floodprot,$chan) [clock seconds]
  }

  if {$bsdforums(log)} { putlog "\[BSDForums\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists bsdforums(lastupdate)]} {
    if {[expr [clock seconds] - $bsdforums(lastupdate)] >= [expr $bsdforums(updates) * 60]} {
      set ret [bsdforums:getdata]
    }
  } elseif {![info exists bsdforumsdata(title,0)]} {
    set ret [bsdforums:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $bsdforums(headlines)} {incr i} {
      if {![info exists bsdforumsdata(title,$i)]} { break }
      bsdforums:put $chan $nick $i $bsdforums(method)
    }
  } else {
    putserv "NOTICE $nick :\[BSDForums\] Something went wrong while updating."
  }
  catch { unset ret diff i }
}

proc bsdforums:put {chan nick which method} {
  global bsdforums bsdforumsdata

  set outchan $bsdforums(layout)
  regsub -all "%title" $outchan $bsdforumsdata(title,$which) outchan
  regsub -all "%link"  $outchan $bsdforumsdata(link,$which) outchan
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
  catch { unset item outchan }
}

proc bsdforums:update {} {
  global bsdforums bsdforumsdata

  if {[bsdforums:getdata] != -1} {

    if {![info exists bsdforumsdata(link,0)]} {
      putlog "\[BSDForums\] Something went wrong while updating..."
      return -1
    }

    if {![info exists bsdforums(lastitem)]} {
      set bsdforums(lastitem) $bsdforumsdata(link,0)
      if {$bsdforums(log)} { putlog "\[BSDForums\] Last news item set to '$bsdforumsdata(link,0)'." }
    } else {
      if {$bsdforums(log)} { putlog "\[BSDForums\] Last news item is '$bsdforumsdata(link,0)'." }
    }

    if {$bsdforumsdata(link,0) != $bsdforums(lastitem)} {
      if {$bsdforums(log)} { putlog "\[BSDForums\] There's news!" }
      for {set i 0} {$i < $bsdforums(automax)} {incr i} {
        if {![info exists bsdforumsdata(link,$i)]} { break }
        if {$bsdforumsdata(link,$i) == $bsdforums(lastitem)} { break }
        foreach chan [split $bsdforums(autonewschan)] { bsdforums:put $chan $chan $i 1 }
      }
    } else {
      if {$bsdforums(log)} { putlog "\[BSDForums\] No news." } 
    }

    set bsdforums(lastitem) $bsdforumsdata(link,0)
  }

  if {$bsdforums(updates) < 120} { 
    putlog "\[BSDForums\] Warning: the \$bsdforums(updates) setting is too low! Defaulting to 120 minutes..."
    timer 120 bsdforums:update
  } else {
    timer $bsdforums(updates) bsdforums:update
  }

  catch { unset i chan }
  return 0
}

proc bsdforums:autoff {nick uhost hand chan text} {
  global lastbind bsdforums
  if {[lsearch -exact $bsdforums(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$bsdforums(log)} { putlog "\[BSDForums\] Trigger: $lastbind in $chan by $nick" }

  if {$bsdforums(autonews) == 1} {
    set bsdforums(autonews) 0;  catch { unset bsdforums(lastitem) }
    set whichtimer [timerexists "bsdforums:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[BSDForums\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION has turned the bsdforums.org news announcer off.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already off!"
  }
}

proc bsdforums:auton {nick uhost hand chan text} {
  global lastbind bsdforums
  if {[lsearch -exact $bsdforums(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$bsdforums(log)} { putlog "\[BSDForums\] Trigger: $lastbind in $chan by $nick" }

  if {$bsdforums(autonews) == 0} {
    set bsdforums(autonews) 1;  bsdforums:update
    putlog "\[BSDForums\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION has turned the bsdforums.org news announcer on.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already on!"
  }
}

if {$bsdforums(autonews) == 1} { bsdforums:update }

putlog "\[BSDForums\] News Announcer version $bsdforums(version): Loaded!"
