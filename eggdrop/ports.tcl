# $Id: ports.tcl,v 1.4 2003-07-04 13:49:09 peter Exp $

# FreeBSD Ports search script for the eggdrop
# version 0.2, 04/07/2003, by Peter Postma <peter@webdeveloping.nl>
#
# Changelog:
# 0.2: (04/07/2003)
#  - added several configuration options:
#  - configurable flags
#  - nopub, where the bot doesn't speak
#  - configurable trigger(s)
#  - flood protection
#  - port info can now be retrieved from
#    /usr/ports/INDEX or the FreeBSD webpage
#  - method to send message
#  - added changelog & version bumped to 0.2
# 0.1: (01/07/2003)
#  - first version 
#

### Configuration settings ###

# flags needed to use the trigger [default=everyone]
set ports(flags) "-|-"

# channels where the bot doesn't respond to triggers [seperate with spaces]
set ports(nopub) "" 

# the triggers: [seperate with spaces]
set ports(triggers) "!port"

# flood protection: seconds between use of the triggers
# to disable: set it to 0
set ports(antiflood) 10

# get port info from the web or local INDEX?
# web   = get from FreeBSD webpage
# index = get from /usr/ports/INDEX
set ports(info) index

# method to send the messages:
# 0 = Private message
# 1 = Public message 
# 2 = Private notice 
# 3 = Public notice
set ports(method) 1

### End Configuration settings ###


### Begin TCL code ###

if {$ports(info) == "web"} { package require http }

set ports(version) 0.2

if {[info tclversion] < 8.1} {
  putlog "Cannot load [file tail [info script]]: You need at least TCL version 8.1 and you have TCL version [info tclversion]."
  return 1
}

for {set i 0} {$i < [llength $ports(triggers)]} {incr i} {
  bind pub $ports(flags) [lindex $ports(triggers) $i] pub:ports
}
catch { unset i }

proc pub:ports {nick uhost hand chan text} {
  global ports lastbind

  if {[string length [string trim [lindex $text 0]]] == 0} {
    putquick "NOTICE $nick :Usage: $lastbind <portname>"
    return 0
  }

  if {[info exists ports(floodprot)]} {
    set diff [expr [clock seconds] - $ports(floodprot)]
    if {$diff < $ports(antiflood)} {
      putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $ports(antiflood) - $diff] seconds..."
      return 0
    }
    catch { unset diff }
  }
  set ports(floodprot) [clock seconds]

  switch -- $ports(method) {
    0 { set method "PRIVMSG $nick" }
    1 { set method "PRIVMSG $chan" }
    2 { set method "NOTICE $nick"  }
    3 { set method "NOTICE $chan"  }
    default { set method "PRIVMSG $chan" }
  }

  if {$ports(info) == "web"} {
    set ports(url) "http://www.freebsd.org/cgi/ports.cgi?query=[join $text]&stype=name"
    if {[catch {set ports(page) [http::geturl $ports(url) -timeout 20000]} msg]} {
      putquick "NOTICE $nick :Can't connect ($msg)"
      return 0
    }
    set ports(data) [http::data $ports(page)]

    if {[regexp -nocase "<h3><a href.*?>Category (.+)</a></h3>.*?<b><a NAME.*?>(.+)</a>.*?<dd>(.+)<br>" $ports(data) t category name descr]} {
      putquick "$method :Port $name in category $category: $descr"
    } else {
      putquick "$method :Port not found."
    }
    catch { http::cleanup $ports(page) }

  } else {
    set ports(file) "/usr/ports/INDEX"

    if {[catch { set fd [open $ports(file) r]} err]} {
      putquick "NOTICE $nick :$err"
      return 0 
    }
    while {[gets $fd ports(line)] >= 0} {
      if {[regexp -nocase "^([join $text].*?)\\|.*?\\|.*?\\|(.*?)\\|.*?\\|.*?\\|(.*?)\\|" $ports(line) t name descr category]} {
        regsub -all " " $category ", " category
        putquick "$method :Port $name in category $category: $descr"
        close $fd
        return 0
      }
    }
    putquick "$method :Port not found."
    close $fd
  }

  return 0
}

putlog "FreeBSD Ports search script $ports(version) loaded!"
