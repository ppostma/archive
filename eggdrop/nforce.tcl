# $Id: nforce.tcl,v 1.3 2004-09-05 01:16:27 peter Exp $

# Nforce.nl release script for the eggdrop
# version 1.2, 05/09/2004, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.2: (05/09/2004)
#  - Fixed a typo, found by JAFO <jafo at wondernet dot nu>.
# 1.1: (25/08/2004)
#  - translated to english.
# 1.0: (14/08/2004) [first version]
#  - first release.
#
# This script makes use of a function from alltools.tcl.
# Please put alltools.tcl in your eggdrop configuration!
#
# This script also uses http.tcl. You don't need to put http.tcl
# your eggdrop configuration!
#
# You need at least Tcl version 8.0 to get this script running!
#
# For questions/suggestions/bug/etc: peter@webdeveloping.nl
# If you found spelling/grammatical errors, please also mail me!
#
# You can always find the latest version of this script on this page:
#   http://pointless.nl/eggdrop.php
#
# Please change the configuration setting below:
#

### Configuration settings ###

# use a http proxy the get the data? (use this if you don't have direct
# internet connection). enter the info in this form: "host.isp.com:port" or
# leave it empty for no proxy.
set nforce(proxy) ""

# the triggers: [separate with spaces]
set nforce(triggers) "!nforce"

# flags needed to use the triggers [default = everyone]
set nforce(flags) "-|-"

# channels where the bot doesn't respond to triggers [separate with spaces]
set nforce(nopub) "" 

# method to send the messages:
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set nforce(method) 1

# display how many releases when a trigger is used [> 1]
set nforce(releases) 2

# flood protection: seconds between use of the triggers
# set this to 0 to disable the flood protection.
set nforce(antiflood) 10

# below you can change the layout of the output:
# %date    = date
# %name    = name of the release
# %group   = naam of the group
# %section = section name / categorie
# %size    = number of files + MB
# %orig    = original NFO name
# %nfo     = NFO url
# %site    = site url
# %review  = review url
# %b       = bold text
# %u       = underlined text
set nforce(layout) "\[%bNFOrce%b\] %date - %name (%section) - %nfo"

# check for releases after how many minutes? [minimal 15]
# this is used by the triggers and the auto updater.
set nforce(updates) 20

# announce new releases automatically in the channels? [0=no / 1=yes]
set nforce(announcer) 0

# announcer: send to which channels? [separate channels with spaces]
# use "*" to announce to all channels (/amsg).
set nforce(announcechan) "#channel"

# maximum number of messages which will be displayed with the automatic updates.
# with this setting you can prevent channel flooding if the updates are high
# (e.g. longer than a hour).
set nforce(announcemax) 3

# trigger to turn off the announcer. [string]
set nforce(autontrigger) "!nforceon"

# trigger to turn on the announcer. [string]
set nforce(autofftrigger) "!nforceoff"

# flags needed to use the announcer [on/off] triggers [default = master]
set nforce(autotriggerflag) "m|m"

# log additional information (debug) to the partyline? [0=no / 1=yes]
set nforce(log) 1

### End Configuration settings ###



### Begin Tcl code ###

set nforce(version) "1.2"

if {[catch { package require http } err]} {
  putlog "\[NFOrce\] Cannot load [file tail [info script]]: Problem loading the http package: $err"
  return 1
}

if {[info tclversion] < 8.0} {
  putlog "\[NFOrce\] Cannot load [file tail [info script]]: You need at least Tcl version 8.0 and you have Tcl version [info tclversion]."
  return 1
}

if {![info exists alltools_loaded]} {
  putlog "\[NFOrce\] Cannot load [file tail [info script]]: Please put alltools.tcl in your eggdrop configuration!"
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
      putlog "\[NFOrce\] Invalid proxy configuration ($nforce(proxy))"
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
        putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $nforce(antiflood) - $diff] seconds..."
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
    putserv "NOTICE $nick :\[NFOrce\] Something went wrong while updating."
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
      if {[regexp {^\*$} $nforce(announcechan)]} {
        set dest [channels]
      } else {
        set dest $nforce(announcechan)
      }
      for {set i 0} {$i < $nforce(announcemax)} {incr i} {
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

  if {$nforce(announcer) == 1} {
    set nforce(announcer) 0;  catch { unset nforce(lastitem) }
    set whichtimer [timerexists "nforce:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    catch { unset whichtimer }
    putlog "\[NFOrce\] Announcer turned off."
    putserv "PRIVMSG $chan :\001ACTION has turned the nforce.nl release announcer off.\001"
  } else {
    putserv "NOTICE $nick :The nforce.nl release announcer is already off."
  }
}

proc nforce:auton {nick uhost hand chan text} {
  global lastbind nforce
  if {[lsearch -exact $nforce(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$nforce(log)} { putlog "\[NFOrce\] Trigger: $lastbind in $chan by $nick" }

  if {$nforce(announcer) == 0} {
    set nforce(announcer) 1;  nforce:update
    putlog "\[NFOrce\] Announcer turned on."
    putserv "PRIVMSG $chan :\001ACTION has turned the nforce.nl release announcer on.\001"
  } else {
    putserv "NOTICE $nick :The nforce.nl release announcer is already on."
  }
}

if {$nforce(announcer) == 1} { nforce:update }

putlog "\[NFOrce\] Release announcer version $nforce(version): Loaded!"
