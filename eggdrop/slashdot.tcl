# $Id: slashdot.tcl,v 1.9 2003-05-21 16:26:02 peter Exp $

# slashdot.tcl / Slashdot.org News Announce Script for an eggdrop
# version 1.7 / 20/05/2003 / by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
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
# The slashdot.tcl script works best with TCL versions higher than 8.2.
#
# For Questions/suggestions/bug/etc: peter@webdeveloping.nl
# If you found spelling/grammatical errors, please also mail me!
#
# Please change the configuration setting below:

### Configuration settings ###

# flags needed to use the trigger [default=everyone]
set slashdot(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set slashdot(nopub) "" 

# the triggers: [seperate with spaces]
set slashdot(triggers) "!slashdot /."

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set slashdot(method) 1

# display n headlines when a trigger is used [> 1]
set slashdot(headlines) 2

# below you can change the layout:
# %tim = time
# %aut = author
# %sec = section
# %tit = title
# %url = url to slashdot's news item
# %com = comments
# %b   = bold text
# %u   = underlined text
set slashdot(layout) "\[%bSlashdot%b\] (%sec) %tit - %url"

# announce the news automaticly in the channels? [0=no / 1=yes]
set slashdot(autonews) 0

# autonews: send to which channels? [seperate channels with spaces]
set slashdot(autonewschan) "#channel1 #channel2"

# check for news after n minutes? [min. 30]
# don't set this to low!
set slashdot(updates) 60

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



### Begin TCL code ###

set slashdot(version) "1.7"

package require http

for {set i 0} {$i < [llength $slashdot(triggers)]} {incr i} {
  bind pub $slashdot(flags) [lindex $slashdot(triggers) $i] slashdot:pub
  if {$slashdot(log)} { putlog "\[Slashdot\] Trigger [lindex $slashdot(triggers) $i] added." }
}
catch { unset i }

bind pub $slashdot(autotriggerflag) $slashdot(autofftrigger) slashdot:autoff
bind pub $slashdot(autotriggerflag) $slashdot(autontrigger) slashdot:auton

proc slashdot:getdata {} {
  global slashdot slashdotdata

  if {$slashdot(log)} { putlog "\[Slashdot\] Updating data..." }
  if {[info exists slashdotdata]} { unset slashdotdata }

  set url "http://slashdot.org/slashdot.xml"
  set page [::http::config -useragent "Mozilla"]

  if {[catch {set page [::http::geturl $url -timeout 15000]} msg]} {
    putlog "\[Slashdot\] Problem: $msg"
    return -1
  }
  
  if {[::http::status $page] != "ok"} {
    putlog "\[Slashdot\] Problem: [::http::status $page]"
    return -1
  }

  if {![regexp -nocase {ok} [::http::code $page]]} {
    putlog "\[Slashdot\] Problem: [::http::code $page]"
    return -1
  }

  set lines [split [::http::data $page] \n]
  set count 0

  for {set i 0} {$i < [llength $lines]} {incr i} {
    set line [lindex $lines $i]
    regexp "<title>(.*?)</title>" $line trash slashdotdata(title,$count)
    regexp "<url>(.*?)</url>" $line trash slashdotdata(url,$count)
    regexp "<time>(.*?)</time>" $line trash slashdotdata(time,$count)
    regexp "<author>(.*?)</author>" $line trash slashdotdata(author,$count)
    regexp "<comments>(.*?)</comments>" $line trash slashdotdata(comments,$count)
    if {[regexp "<section>(.*?)</section>" $line trash slashdotdata(section,$count)]} { incr count }
  }
  ::http::cleanup $page

  catch { unset url page msg lines count line trash }

  return 0
}

proc slashdot:pub {nick uhost hand chan text} {
  global lastbind slashdot slashdotdata
  if {[lsearch -exact $slashdot(nopub) [string tolower $chan]] >= 0} { return 0 }  

  if {$slashdot(log)} { putlog "\[Slashdot\] Trigger: $lastbind in $chan by $nick" }

  if {[slashdot:getdata] != -1} {
    for {set i 0} {$i < $slashdot(headlines)} {incr i} {
      if {![info exists slashdotdata(time,$i)]} { break }
      slashdot:put $chan $nick $i $slashdot(method)
    }
    catch { unset i }
  } else {
    putserv "NOTICE $nick :\[Slashdot\] Something went wrong while updating."
  }
  if {[info exists slashdotdata]} { unset slashdotdata }
}

proc slashdot:put {chan nick which method} {
  global slashdot slashdotdata

  set outchan $slashdot(layout)
  foreach item {time author title section url comments} {
    regsub -all "\\&" $slashdotdata($item,$which) "\\\\&" slashdotdata($item,$which)
  }
  regsub -all "%tim" $outchan $slashdotdata(time,$which) outchan
  regsub -all "%aut" $outchan $slashdotdata(author,$which) outchan
  regsub -all "%tit" $outchan $slashdotdata(title,$which) outchan
  regsub -all "%sec" $outchan $slashdotdata(section,$which) outchan
  regsub -all "%url" $outchan $slashdotdata(url,$which) outchan
  regsub -all "%com" $outchan $slashdotdata(comments,$which) outchan
  regsub -all "&amp;"  $outchan "\\&" outchan
  regsub -all "&quot;" $outchan "\"" outchan
  regsub -all "%b" $outchan "\002" outchan
  regsub -all "%u" $outchan "\037" outchan
  switch -- $method {
    0 { putserv "PRIVMSG $nick :$outchan" }
    1 { putserv "PRIVMSG $chan :$outchan" }
    2 { putserv "NOTICE $nick :$outchan" }
    3 { putserv "NOTICE $chan :$outchan" }
    default { putserv "PRIVMSG $chan :$outchan" }
  }
  catch { unset item outchan }
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
      for {set i 0} {$i < $slashdot(automax)} {incr i} {
        if {![info exists slashdotdata(time,$i)]} { break }
        if {$slashdotdata(time,$i) == $slashdot(lastitem)} { break }
        foreach chan [split $slashdot(autonewschan)] { slashdot:put $chan $chan $i 1 }
        catch { unset chan }
      }
      catch { unset i }
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
  if {[info exists slashdotdata]} { unset slashdotdata }

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

set whichtimer [timerexists "slashdot:update"]
if {$whichtimer != ""} { killtimer $whichtimer }
catch { unset whichtimer }

if {$slashdot(autonews) == 1} { slashdot:update }

putlog "\[Slashdot\] News Announcer version $slashdot(version): Loaded!"
