# $Id: clanbase.tcl,v 1.1 2003-05-18 12:08:56 peter Exp $

# cb.tcl / Clanbase.com News Announce Script for an eggdrop
# version 1.0 / 18/05/2003 / by Peter Postma <peter@webdeveloping.nl>
#
# This script makes use of a function from alltools.tcl.
# Please put alltools.tcl in your eggdrop configuration!
#
# This script also uses http.tcl. You *don't* need to put http.tcl
# your eggdrop configuration!
#
# The cb.tcl script works best with TCL versions higher than 8.2.
#
# For Questions/suggestions/bug/etc: peter@webdeveloping.nl
# If you found spelling/grammatical errors, please also mail me!
#
# Please change the configuration setting below:

### Configuration settings ###

# flags needed to use the trigger [default=everyone]
set cb(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set cb(nopub) "" 

# the triggers: [seperate with spaces]
set cb(triggers) "!cb !clanbase"

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set cb(method) 1

# display n headlines when a trigger is used [> 1]
set cb(headlines) 2

# below you can change the layout:
# %title = title from article
# %link  = link to cb's news item
# %b   = bold text
# %u   = underlined text
set cb(layout) "\[%bClanbase%b\] %title - %link"

# announce the news automaticly in the channels? [0=no / 1=yes]
set cb(autonews) 1

# autonews: send to which channels? [seperate channels with spaces]
set cb(autonewschan) "#channel"

# check for news after n minutes? [min. 30]
# don't set this to low!
set cb(updates) 120

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

set cb(version) "1.0"

package require http

for {set i 0} {$i < [llength $cb(triggers)]} {incr i} {
  bind pub $cb(flags) [lindex $cb(triggers) $i] cb:pub
  if {$cb(log)} { putlog "\[Clanbase\] Trigger [lindex $cb(triggers) $i] added." }
}
unset i

bind pub $cb(autotriggerflag) $cb(autofftrigger) cb:autoff
bind pub $cb(autotriggerflag) $cb(autontrigger) cb:auton

proc cb:getdata {} {
  global cb cbdata

  if {$cb(log)} { putlog "\[Clanbase\] Updating data..." }
  if {[info exists cbdata]} { unset cbdata }

  set url "http://www.clanbase.com/rss.php"
  set page [::http::config -useragent "Mozilla"]

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[Clanbase\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[Clanbase\] Problem: [::http::status $page]"
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Clanbase\] Problem: [::http::code $page]"
    return -1
  }

  set lines [split [::http::data $page] \n]
  set count 0
  set item 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    if {[regexp "<item>" $line trash]} { set item 1 }
    if {[regexp "</item>" $line trash]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*?)</title>" $line trash cbdata(title,$count)
      if {[regexp "<link>(.*?)</link>" $line trash cbdata(link,$count)]} { incr count }
    }
  }
  ::http::cleanup $page

  unset url page msg count item lines
  if {[info exists line]} { unset line }
  if {[info exists trash]} { unset trash }

  return 0
}

proc cb:pub {nick uhost hand chan text} {
  global lastbind cb cbdata
  if {[lsearch -exact $cb(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$cb(log)} { putlog "\[Clanbase\] Trigger: $lastbind in $chan by $nick" }

  if {[cb:getdata] != -1} {
    for {set i 0} {$i < $cb(headlines)} {incr i} { cb:put $chan $nick $i $cb(method) }
    unset i
  } else {
    putserv "NOTICE $nick :\[Clanbase\] Something went wrong while updating."
  }
  if {[info exists cbdata]} { unset cbdata }
}

proc cb:put {chan nick which method} {
  global cb cbdata

  set outchan $cb(layout)
  regsub -all "%title" $outchan $cbdata(title,$which) outchan
  regsub -all "%link" $outchan $cbdata(link,$which) outchan
  regsub -all "&amp;" $outchan "\\\&" outchan
  regsub -all "%b"   $outchan "\002" outchan
  regsub -all "%u"   $outchan "\037" outchan
  switch -- $method {
    0 { putserv "PRIVMSG $nick :$outchan" } 
    1 { putserv "PRIVMSG $chan :$outchan" }
    2 { putserv "NOTICE $nick :$outchan" }
    3 { putserv "NOTICE $chan :$outchan" }
    default { putserv "PRIVMSG $chan :$outchan" }
  }
  unset outchan
}

proc cb:update {} {
  global cb cbdata cb_lastitem

  if {[cb:getdata] != -1} {

    if {![info exists cbdata(title,0)]} {
      putlog "\[Clanbase\] Something went wrong while updating."
      return -1
    }

    if {![info exists cb_lastitem]} {
      set cb_lastitem $cbdata(title,0)
      if {$cb(log)} { putlog "\[Clanbase\] Last news item set to $cbdata(title,0)" }
    } else {
      if {$cb(log)} { putlog "\[Clanbase\] Last news item is $cbdata(title,0)" }
    }

    if {$cbdata(title,0) != $cb_lastitem} {
      if {$cb(log)} { putlog "\[Clanbase\] There's news!" }
      for {set i 0} {$i < $cb(automax)} {incr i} {
        if {$cbdata(title,$i) == $cb_lastitem} { break }
        foreach chan [split $cb(autonewschan)] { cb:put $chan $chan $i 1 }
        unset chan
      }
      unset i
    } else {
      if {$cb(log)} { putlog "\[Clanbase\] No news." } 
    }

    set cb_lastitem $cbdata(title,0)
  }

  if {$cb(updates) < 30} { 
    timer 30 cb:update
  } else {
    timer $cb(updates) cb:update
  }
  if {[info exists cbdata]} { unset cbdata }

  return 0
}

proc cb:autoff {nick uhost hand chan text} {
  global lastbind cb cb_lastitem
  if {[lsearch -exact $cb(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$cb(log)} { putlog "\[Clanbase\] Trigger: $lastbind in $chan by $nick" }

  if {$cb(autonews) == 1} {
    set cb(autonews) 0;  unset cb_lastitem
    set whichtimer [timerexists "cb:update"]
    if {$whichtimer != ""} { killtimer $whichtimer }
    unset whichtimer
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

set whichtimer [timerexists "cb:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
unset whichtimer

if {$cb(autonews) == 1} { cb:update }

putlog "\[Clanbase\] News Announcer version $cb(version): Loaded!"
