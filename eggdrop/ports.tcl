# $Id: ports.tcl,v 1.1 2003-07-01 15:01:40 peter Exp $

# FreeBSD Port search script for the eggdrop
# version 0.1, 01/07/2003, by Peter Postma <peter@webdeveloping.nl>

package require Tcl 8.2
package require http

bind pub -|- "!port" pub:ports

proc pub:ports {nick uhost hand chan text} {
  global lastbind

  if {[string length [string trim [lindex $text 0]]] == 0} {
    putquick "NOTICE $nick :Usage: $lastbind <portname>"
    return 0
  }

  set ports(url) "http://www.freebsd.org/cgi/ports.cgi?query=[join $text]&stype=name"
  if {[catch {set ports(page) [http::geturl $ports(url) -timeout 15000]} msg]} {
    putquick "NOTICE $nick :Can't connect ($msg)"
    return 0
  }
  set ports(data) [http::data $ports(page)]

  if {[regexp -nocase {<h3><a href.*?>Category (.+)</a></h3>.*?<b><a NAME.*?>(.+)</a>.*?<dd>(.+)<br>} $ports(data) t category name descr]} {
    putquick "PRIVMSG $chan :Port $name in category $category: $descr"
  } else {
    putquick "PRIVMSG $chan :Port not found."
  }
  catch { http::cleanup $ports(page) }
  return 0
}

putlog "FreeBSD Port search script 0.1 loaded!"
