# $Id: google.tcl,v 1.8 2003-08-02 14:31:22 peter Exp $

# Google script for the eggdrop
# version 0.3, 02/08/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 0.3: (09/07/2003)
#  - added several configuration options
#

### Configuration settings ###

# flags needed to use the trigger [default=everyone]
set google(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set google(nopub) "" 

# the triggers: [seperate with spaces]
set google(triggers) "!google"

# flood protection: seconds between use of the triggers
# to disable: set it to 0
set google(antiflood) 5

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set google(method) 1

# show how many results? 1, 2 or 3??
set google(results) 3

### End Configuration settings ###


### Begin Tcl code ###

set google(version) 0.3

if {[catch { package require http } err]} {
  putlog "Cannot load [file tail [info script]]: Problem loading the http package: $err"
  return 1
}

if {[info tclversion] < 8.1} {
  putlog "Cannot load [file tail [info script]]: You need at least Tcl version 8.1 and you have Tcl version [info tclversion]."
  return 1
}

for {set i 0} {$i < [llength $google(triggers)]} {incr i} {
  bind pub $google(flags) [lindex $google(triggers) $i] pub:google
}
catch { unset i }

proc pub:google {nick uhost hand chan text} {
  global lastbind google

  if {[lsearch -exact $google(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {[string length [string trim [lindex $text 0]]] == 0} {
    putquick "NOTICE $nick :Usage: $lastbind <keywords>"
    return 0
  }

  if {[info exists google(floodprot)]} {
    set diff [expr [clock seconds] - $google(floodprot)]
    if {$diff < $google(antiflood)} {
      putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $google(antiflood) - $diff] seconds..."
      return 0
    }
    catch { unset diff }
  }
  set google(floodprot) [clock seconds]

  regsub -all { } [join $text] {+} search
  set google(url) "http://www.google.nl/search?q=$search"
  set google(page) [http::config -useragent "Mozilla"]
  if {[catch {set google(page) [http::geturl $google(url) -timeout 15000]} msg]} {
    putquick "NOTICE $nick :Can't connect ($msg)"
    return 0
  }
  set google(data) [http::data $google(page)]

  if {$google(results) >= 1} {
    regexp -nocase {related:(.*?)>} $google(data) t link1
  }
  if {$google(results) >= 2} {
    regexp -nocase {related:.*?>.*?related:(.*?)>} $google(data) t link2
  }
  if {$google(results) >= 3} {
    regexp -nocase {related:.*?>.*?related:.*?>.*?related:(.*?)>} $google(data) t link3
  }

  if {[info exists link3]} {
    set output "http://$link1 - http://$link2 - http://$link3"
  } elseif {[info exists link2]} {
    set output "http://$link1 - http://$link2"
  } elseif {[info exists link1]} {
    set output "http://$link1"
  } else {
    set output "Nothing found."
  }

  regsub -all {%26} $output {\&} output
  regsub -all {%3F} $output {?} output
  regsub -all {%3D} $output {=} output 

  switch -- $google(method) {
    0 { putquick "PRIVMSG $nick :$output" }
    1 { putquick "PRIVMSG $chan :$output" }
    2 { putquick "NOTICE $nick :$output" }
    3 { putquick "NOTICE $chan :$output" }
    default { putquick "PRIVMSG $chan :$output" }
  }

  catch { unset output t link1 link2 link3 }
  catch { http::cleanup $google(page) }
  return 0
}

putlog "Google script $google(version) loaded!"
