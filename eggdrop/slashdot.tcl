# $Id: slashdot.tcl,v 1.1.1.1 2003-03-19 14:50:33 peter Exp $

# slashdot.tcl / Slashdot News Headlines script for an eggdrop
# version 1.2 / 01/10/2002 / by Peter Postma <peter@webdeveloping.nl>
#
# Changelog: versions > 1.2 were crappy. bad code etc..
#            so changelog isn't interesting :P         
#
# Typ: !slashdot in the channel and the bot will show 
# the current headlines from http://slashdot.org
#
# You can adjust some variables below to configure this script.
#

### Configuration settings ###

# flags needed to use the !slashdot command
set slashdot_flag "-|-"

# amount of headlines to be showed
set slashdot_headlines 2

### End Configuration settings ###



### Begin TCL code ###

set slashversion "1.2"

package require http

bind pub $slashdot_flag "!slashdot" pub:slashdot
bind pub $slashdot_flag "/." pub:slashdot

proc pub:slashdot {nick uhost hand chan text} {
  global slashdot_headlines

  set url "http://slashdot.org/slashdot.xml"
  set page [http::config -useragent "Mozilla"]
  set page [http::geturl $url -timeout 15000]
  set lines [split [http::data $page] \n]
  set numlines [llength $lines]
  set count 0

  for {set i 0} {$i < $numlines} {incr i} {
    set line [lindex $lines $i]

    regexp "<title>(.*?)</title>" $line trash slash_title
    regexp "<url>(.*?)</url>" $line trash slash_url
    if {[regexp "<section>(.*?)</section>" $line trash slash_section]} {
      putquick "PRIVMSG $chan :\[$slash_section\] $slash_title - $slash_url"
      incr count
      if {$count >= $slashdot_headlines} { return 0 }
    }
  }
  return 0
}

putlog "Slashdot News script version $slashversion: LOADED!"
