# $Id: kerneltrap.tcl,v 1.25 2003-08-02 14:27:55 peter Exp $

# KernelTrap.org News Announce Script for the eggdrop
# version 1.4, 02/08/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.4: (??/??/????)
#  - changed the update method somewhat.
#    the kerneltrap(updates) it now also being used by the triggers
#    to check how long to cache the data.
#  - proxy configuration added.
#  - flood protection is now for each channel (this is more usefull IMHO).
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
set kerneltrap(proxy) ""

# the triggers: [seperate with spaces]
set kerneltrap(triggers) "!kerneltrap"

# flags needed to use the trigger [default=everyone]
set kerneltrap(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set kerneltrap(nopub) "" 

# method to send the messages:
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set kerneltrap(method) 1

# display n headlines when a trigger is used [> 1]
set kerneltrap(headlines) 2

# flood protection: seconds between use of the triggers
# set this to 0 to disable the flood protection.
set kerneltrap(antiflood) 10

# below you can change the layout of the output:
# %title = titel of article
# %link  = link to article
# %b   = bold text
# %u   = underlined text
set kerneltrap(layout) "\[%bKernelTrap%b\] %title - %link"

# check for news after n minutes? [min. 30]
# this value is being used by the trigger and the autonews.
set kerneltrap(updates) 60

# announce the news automaticly in the channels? [0=no / 1=yes]
set kerneltrap(autonews) 0

# autonews: send to which channels? [seperate channels with spaces]
set kerneltrap(autonewschan) "#channel1"

# max. amount of messages which will be displayed meanwhile automatic updates.
# with this setting you can prevent channel flooding if the updates are high
# (e.g. longer than a hour).
set kerneltrap(automax) 3

# trigger to turn off the autonews. [string]
set kerneltrap(autontrigger) "!kerneltrapon"

# trigger to turn on the autonews. [string]
set kerneltrap(autofftrigger) "!kerneltrapoff"

# flags needed to use the autonews[on/off] trigger [default=master]
set kerneltrap(autotriggerflag) "m|m"

# log additional information (debug) to the partyline? [0=no / 1=yes]
set kerneltrap(log) 1

### End Configuration settings ###



### Begin Tcl code ###

set kerneltrap(version) "1.4"

if {[catch { package require http } err]} {
  putlog "\[KernelTrap\] Cannot load [file tail [info script]]: Problem loading the http package: $err"
  return 1
}

if {[info tclversion] < 8.0} {
  putlog "\[KernelTrap\] Cannot load [file tail [info script]]: You need at least Tcl version 8.0 and you have Tcl version [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[KernelTrap\] Cannot load [file tail [info script]]: Please load alltools.tcl in your eggdrop configuration!"
  return 1
}

set whichtimer [timerexists "kerneltrap:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

foreach trigger [split $kerneltrap(triggers)] {
  bind pub $kerneltrap(flags) $trigger kerneltrap:pub
  if {$kerneltrap(log)} { putlog "\[KernelTrap\] Trigger $trigger added." }
}
catch { unset trigger }

if {$kerneltrap(autofftrigger) != ""} { bind pub $kerneltrap(autotriggerflag) $kerneltrap(autofftrigger) kerneltrap:autoff }
if {$kerneltrap(autontrigger)  != ""} { bind pub $kerneltrap(autotriggerflag) $kerneltrap(autontrigger) kerneltrap:auton }

proc kerneltrap:getdata {} {
  global kerneltrap kerneltrapdata

  if {$kerneltrap(log)} { putlog "\[KernelTrap\] Updating data..." }

  set url "http://www.kerneltrap.org/module.php?mod=node&op=feed"
  set page [::http::config -useragent "Mozilla"]

  if {$kerneltrap(proxy) != ""} {
    if {![regexp {(.+):([0-9]+)} $kerneltrap(proxy) trash proxyhost proxyport]} {
      putlog "\[KernelTrap\] Wrong proxy configuration ($kerneltrap(proxy))"
      return -1
    }
    set page [::http::config -proxyhost $proxyhost -proxyport $proxyport]
    catch { unset proxyhost proxyport }
  }

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[KernelTrap\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[KernelTrap\] Problem: [::http::status $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[KernelTrap\] Problem: [::http::code $page]"
    catch { ::http::cleanup $page }
    return -1
  }

  if {[info exists kerneltrapdata]} { unset kerneltrapdata }

  set count 0
  set item 0
  foreach line [split [::http::data $page] \n] {
    regsub -all "\\&" $line "\\\\&" line
    if {[regexp "<item>" $line]} { set item 1 }
    if {[regexp "</item>" $line]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*)</title>" $line trash kerneltrapdata(title,$count)
      if {[regexp "<link>(.*)</link>" $line trash kerneltrapdata(link,$count)]} { incr count }
    }
  }

  set kerneltrap(lastupdate) [clock seconds]

  catch { ::http::cleanup $page }
  catch { unset url page msg count item line trash }

  return 0
}

proc kerneltrap:pub {nick uhost hand chan text} {
  global lastbind kerneltrap kerneltrapdata
  if {[lsearch -exact $kerneltrap(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$kerneltrap(antiflood) > 0} {
    if {[info exists kerneltrap(floodprot,$chan)]} {
      set diff [expr [clock seconds] - $kerneltrap(floodprot,$chan)]
      if {$diff < $kerneltrap(antiflood)} {
        putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $kerneltrap(antiflood) - $diff] seconds..."
        return 0
      }
    }
    set kerneltrap(floodprot,$chan) [clock seconds]
  }

  if {$kerneltrap(log)} { putlog "\[KernelTrap\] Trigger: $lastbind in $chan by $nick" }

  set ret 0
  if {[info exists kerneltrap(lastupdate)]} {
    if {[expr [clock seconds] - $kerneltrap(lastupdate)] >= [expr $kerneltrap(updates) * 60]} {
      set ret [kerneltrap:getdata]
    }
  } elseif {![info exists kerneltrapdata(title,0)]} {
    set ret [kerneltrap:getdata]
  }

  if {$ret != -1} {
    for {set i 0} {$i < $kerneltrap(headlines)} {incr i} {
      if {![info exists kerneltrapdata(title,$i)]} { break }
      kerneltrap:put $chan $nick $i $kerneltrap(method)
    }
  } else {
    putserv "NOTICE $nick :\[KernelTrap\] Something went wrong while updating."
  }
  catch { unset ret diff i }
}

proc kerneltrap:put {chan nick which method} {
  global kerneltrap kerneltrapdata

  set outchan $kerneltrap(layout)
  regsub -all "%title" $outchan $kerneltrapdata(title,$which) outchan
  regsub -all "%link"  $outchan $kerneltrapdata(link,$which) outchan
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

proc kerneltrap:update {} {
  global kerneltrap kerneltrapdata

  if {[kerneltrap:getdata] != -1} {

    if {![info exists kerneltrapdata(link,0)]} {
      putlog "\[KernelTrap\] Something went wrong while updating..."
      return -1
    }

    if {![info exists kerneltrap(lastitem)]} {
      set kerneltrap(lastitem) $kerneltrapdata(link,0)
      if {$kerneltrap(log)} { putlog "\[KernelTrap\] Last news item set to '$kerneltrapdata(link,0)'." }
    } else {
      if {$kerneltrap(log)} { putlog "\[KernelTrap\] Last news item is '$kerneltrapdata(link,0)'." }
    }

    if {$kerneltrapdata(link,0) != $kerneltrap(lastitem)} {
      if {$kerneltrap(log)} { putlog "\[KernelTrap\] There's news!" }
      for {set i 0} {$i < $kerneltrap(automax)} {incr i} {
        if {![info exists kerneltrapdata(link,$i)]} { break }
        if {$kerneltrapdata(link,$i) == $kerneltrap(lastitem)} { break }
        foreach chan [split $kerneltrap(autonewschan)] { kerneltrap:put $chan $chan $i 1 }
      }
    } else {
      if {$kerneltrap(log)} { putlog "\[KernelTrap\] No news." } 
    }

    set kerneltrap(lastitem) $kerneltrapdata(link,0)
  }

  if {$kerneltrap(updates) < 30} {
    putlog "\[KernelTrap\] Warning: the \$kerneltrap(updates) setting is too low! Defaulting to 30 minutes..."
    timer 30 kerneltrap:update
  } else {
    timer $kerneltrap(updates) kerneltrap:update
  }

  catch { unset i chan }
  return 0
}

proc kerneltrap:autoff {nick uhost hand chan text} {
  global lastbind kerneltrap
  if {[lsearch -exact $kerneltrap(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$kerneltrap(log)} { putlog "\[KernelTrap\] Trigger: $lastbind in $chan by $nick" }

  if {$kerneltrap(autonews) == 1} {
    set kerneltrap(autonews) 0;  catch { unset kerneltrap(lastitem) }
    set whichtimer [timerexists "kerneltrap:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[KernelTrap\] Autonews turned off."
    putserv "PRIVMSG $chan :\001ACTION has turned the kerneltrap.org news announcer off.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already off!"
  }
}

proc kerneltrap:auton {nick uhost hand chan text} {
  global lastbind kerneltrap
  if {[lsearch -exact $kerneltrap(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$kerneltrap(log)} { putlog "\[KernelTrap\] Trigger: $lastbind in $chan by $nick" }

  if {$kerneltrap(autonews) == 0} {
    set kerneltrap(autonews) 1;  kerneltrap:update
    putlog "\[KernelTrap\] Autonews turned on."
    putserv "PRIVMSG $chan :\001ACTION has turned the kerneltrap.org news announcer on.\001"
  } else {
    putserv "NOTICE $nick :My news announcer is already on!"
  }
}

if {$kerneltrap(autonews) == 1} { kerneltrap:update }

putlog "\[KernelTrap\] News Announcer version $kerneltrap(version): Loaded!"
