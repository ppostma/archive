# $Id: bsdforums.tcl,v 1.2 2003-06-22 12:07:45 peter Exp $

# BSDForums.org News Announce Script for an eggdrop
# version 1.1, 22/06/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 1.1: (??/??/????)
#  - added url's for latest version & license.
# 1.0: (20/06/2003) [first version]
#  - wrote the script, based on osnews.tcl version 1.3 (beta)
#
# This script makes use of a function from alltools.tcl.
# Please put alltools.tcl in your eggdrop configuration!
#
# This script also uses http.tcl. You *don't* need to put http.tcl
# your eggdrop configuration!
#
# You need at least TCL version 8.2 to get this script running!
#
# For questions/suggestions/bug/etc: peter@webdeveloping.nl
# If you found spelling/grammatical errors, please also mail me!
#
# Script License:
#   http://www.pointless.nl/copyright
#
# You can always find the latest version of this script on this page:
#   http://www.pointless.nl/?page=eggdrop
#
# Please change the configuration setting below:
#

### Configuration settings ###

# flags needed to use the trigger [default=everyone]
set bsdforums(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set bsdforums(nopub) "" 

# the triggers: [seperate with spaces]
set bsdforums(triggers) "!bsdforums"

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set bsdforums(method) 1

# display n headlines when a trigger is used [> 1]
set bsdforums(headlines) 2

# below you can change the layout:
# %title = titel of article
# %link  = link to article
# %b   = bold text
# %u   = underlined text
set bsdforums(layout) "\[%bBSDForums%b\] %title - %link"

# announce the news automaticly in the channels? [0=no / 1=yes]
set bsdforums(autonews) 0

# autonews: send to which channels? [seperate channels with spaces]
set bsdforums(autonewschan) "#channel1"

# check for news after n minutes? [minimal 120]
set bsdforums(updates) 120

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



### Begin TCL code ###

set bsdforums(version) "1.1"

package require Tcl 8.2
package require http

for {set i 0} {$i < [llength $bsdforums(triggers)]} {incr i} {
  bind pub $bsdforums(flags) [lindex $bsdforums(triggers) $i] bsdforums:pub
  if {$bsdforums(log)} { putlog "\[BSDForums\] Trigger [lindex $bsdforums(triggers) $i] added." }
}
catch { unset i }

bind pub $bsdforums(autotriggerflag) $bsdforums(autofftrigger) bsdforums:autoff
bind pub $bsdforums(autotriggerflag) $bsdforums(autontrigger) bsdforums:auton

proc bsdforums:getdata {} {
  global bsdforums bsdforumsdata

  if {$bsdforums(log)} { putlog "\[BSDForums\] Updating data..." }

  set url "http://bsdforums.org/rss/news/rss.xml"
  set page [::http::config -useragent "Mozilla"]

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[BSDForums\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[BSDForums\] Problem: [::http::status $page]"
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[BSDForums\] Problem: [::http::code $page]"
    return -1
  }

  if {[info exists bsdforumsdata]} { unset bsdforumsdata }

  set lines [split [::http::data $page] \n]
  set count 0
  set item 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regsub -all "\\&" $line "\\\\&" line
    if {[regexp "<item (.+)>" $line]} { set item 1 }
    if {[regexp "</item>" $line]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*?)</title>" $line trash bsdforumsdata(title,$count)
      if {[regexp "<link>(.*?)</link>" $line trash bsdforumsdata(link,$count)]} { incr count }
    }
  }

  catch { ::http::cleanup $page }
  catch { unset url page msg lines count item line trash }

  return 0
}

proc bsdforums:pub {nick uhost hand chan text} {
  global lastbind bsdforums bsdforumsdata
  if {[lsearch -exact $bsdforums(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$bsdforums(log)} { putlog "\[BSDForums\] Trigger: $lastbind in $chan by $nick" }

  if {[bsdforums:getdata] != -1} {
    for {set i 0} {$i < $bsdforums(headlines)} {incr i} {
      if {![info exists bsdforumsdata(title,$i)]} { break }
      bsdforums:put $chan $nick $i $bsdforums(method)
    }
    catch { unset i }
  } else {
    putserv "NOTICE $nick :\[BSDForums\] Something went wrong while updating."
  }
  if {[info exists bsdforumsdata]} { unset bsdforumsdata }
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
  switch -- $method {
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

    if {![info exists bsdforumsdata(title,0)]} {
      putlog "\[BSDForums\] Something went wrong while updating..."
      return -1
    }

    if {![info exists bsdforums(lastitem)]} {
      set bsdforums(lastitem) $bsdforumsdata(title,0)
      if {$bsdforums(log)} { putlog "\[BSDForums\] Last news item set to '$bsdforumsdata(title,0)'." }
    } else {
      if {$bsdforums(log)} { putlog "\[BSDForums\] Last news item is '$bsdforumsdata(title,0)'." }
    }

    if {$bsdforumsdata(title,0) != $bsdforums(lastitem)} {
      if {$bsdforums(log)} { putlog "\[BSDForums\] There's news!" }
      for {set i 0} {$i < $bsdforums(automax)} {incr i} {
        if {![info exists bsdforumsdata(title,$i)]} { break }
        if {$bsdforumsdata(title,$i) == $bsdforums(lastitem)} { break }
        foreach chan [split $bsdforums(autonewschan)] { bsdforums:put $chan $chan $i 1 }
        catch { unset chan }
      }
      catch { unset i }
    } else {
      if {$bsdforums(log)} { putlog "\[BSDForums\] No news." } 
    }

    set bsdforums(lastitem) $bsdforumsdata(title,0)
  }

  if {$bsdforums(updates) < 120} { 
    putlog "\[BSDForums\] Warning: the \$bsdforums(updates) setting is too low! Defaulting to 120 minutes..."
    timer 120 bsdforums:update
  } else {
    timer $bsdforums(updates) bsdforums:update
  }
  if {[info exists bsdforumsdata]} { unset bsdforumsdata }

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

set whichtimer [timerexists "bsdforums:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

if {$bsdforums(autonews) == 1} { bsdforums:update }

putlog "\[BSDForums\] News Announcer version $bsdforums(version): Loaded!"
