# $Id: osnews.tcl,v 1.7 2003-05-26 17:02:08 peter Exp $

# osnews.tcl / OSnews.org News Announce Script for an eggdrop
# version 1.2 / 26/05/2003 / by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
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
# The osnews.tcl script works best with TCL versions higher than 8.2.
#
# For Questions/suggestions/bug/etc: peter@webdeveloping.nl
# If you found spelling/grammatical errors, please also mail me!
#
# Please change the configuration setting below:

### Configuration settings ###

# flags needed to use the trigger [default=everyone]
set osnews(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set osnews(nopub) "" 

# the triggers: [seperate with spaces]
set osnews(triggers) "!osnews"

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set osnews(method) 1

# display n headlines when a trigger is used [> 1]
set osnews(headlines) 2

# below you can change the layout:
# %title = titel of article
# %link  = link to article
# %b   = bold text
# %u   = underlined text
set osnews(layout) "\[%bOSnews%b\] %title - %link"

# announce the news automaticly in the channels? [0=no / 1=yes]
set osnews(autonews) 0

# autonews: send to which channels? [seperate channels with spaces]
set osnews(autonewschan) "#channel1"

# check for news after n minutes? [min. 30]
# don't set this to low!
set osnews(updates) 60

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



### Begin TCL code ###

set osnews(version) "1.2"

package require http

for {set i 0} {$i < [llength $osnews(triggers)]} {incr i} {
  bind pub $osnews(flags) [lindex $osnews(triggers) $i] osnews:pub
  if {$osnews(log)} { putlog "\[OSnews\] Trigger [lindex $osnews(triggers) $i] added." }
}
catch { unset i }

bind pub $osnews(autotriggerflag) $osnews(autofftrigger) osnews:autoff
bind pub $osnews(autotriggerflag) $osnews(autontrigger) osnews:auton

proc osnews:getdata {} {
  global osnews osnewsdata

  if {$osnews(log)} { putlog "\[OSnews\] Updating data..." }

  set url "http://www.osnews.com/files/recent.rdf"
  set page [::http::config -useragent "Mozilla"]

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[OSnews\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[OSnews\] Problem: [::http::status $page]"
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[OSnews\] Problem: [::http::code $page]"
    return -1
  }

  if {[info exists osnewsdata]} { unset osnewsdata }

  set lines [split [::http::data $page] \n]
  set count 0
  set item 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regsub -all "\\&" $line "\\\\&" line
    if {[regexp "<item>" $line trash]} { set item 1 }
    if {[regexp "</item>" $line trash]} { set item 0 }
    if {$item == 1} {
      regexp "<title>(.*?)</title>" $line trash osnewsdata(title,$count)
      if {[regexp "<link>(.*?)</link>" $line trash osnewsdata(link,$count)]} { incr count }
    }
  }

  catch { ::http::cleanup $page }
  catch { unset url page msg lines count item line trash }

  return 0
}

proc osnews:pub {nick uhost hand chan text} {
  global lastbind osnews osnewsdata
  if {[lsearch -exact $osnews(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$osnews(log)} { putlog "\[OSnews\] Trigger: $lastbind in $chan by $nick" }

  if {[osnews:getdata] != -1} {
    for {set i 0} {$i < $osnews(headlines)} {incr i} {
      if {![info exists osnewsdata(title,$i)]} { break }
      osnews:put $chan $nick $i $osnews(method)
    }
    catch { unset i }
  } else {
    putserv "NOTICE $nick :\[OSnews\] Something went wrong while updating."
  }
  if {[info exists osnewsdata]} { unset osnewsdata }
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
  switch -- $method {
    0 { putserv "PRIVMSG $nick :$outchan" }
    1 { putserv "PRIVMSG $chan :$outchan" }
    2 { putserv "NOTICE $nick :$outchan" }
    3 { putserv "NOTICE $chan :$outchan" }
    default { putserv "PRIVMSG $chan :$outchan" }
  }
  catch { unset item outchan }
}

proc osnews:update {} {
  global osnews osnewsdata

  if {[osnews:getdata] != -1} {

    if {![info exists osnewsdata(title,0)]} {
      putlog "\[OSnews\] Something went wrong while updating..."
      return -1
    }

    if {![info exists osnews(lastitem)]} {
      set osnews(lastitem) $osnewsdata(title,0)
      if {$osnews(log)} { putlog "\[OSnews\] Last news item set to '$osnewsdata(title,0)'." }
    } else {
      if {$osnews(log)} { putlog "\[OSnews\] Last news item is '$osnewsdata(title,0)'." }
    }

    if {$osnewsdata(title,0) != $osnews(lastitem)} {
      if {$osnews(log)} { putlog "\[OSnews\] There's news!" }
      for {set i 0} {$i < $osnews(automax)} {incr i} {
        if {![info exists osnewsdata(title,$i)]} { break }
        if {$osnewsdata(title,$i) == $osnews(lastitem)} { break }
        foreach chan [split $osnews(autonewschan)] { osnews:put $chan $chan $i 1 }
        catch { unset chan }
      }
      catch { unset i }
    } else {
      if {$osnews(log)} { putlog "\[OSnews\] No news." } 
    }

    set osnews(lastitem) $osnewsdata(title,0)
  }

  if {$osnews(updates) < 30} { 
    putlog "\[OSnews\] Warning: the \$osnews(updates) setting is too low! Defaulting to 30 minutes..."
    timer 30 osnews:update
  } else {
    timer $osnews(updates) osnews:update
  }
  if {[info exists osnewsdata]} { unset osnewsdata }

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

set whichtimer [timerexists "osnews:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

if {$osnews(autonews) == 1} { osnews:update }

putlog "\[OSnews\] News Announcer version $osnews(version): Loaded!"
