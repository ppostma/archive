# $Id: google.tcl,v 1.1 2003-07-01 15:01:40 peter Exp $

# Google script for the eggdrop
# version 0.1, 01/07/2003, by Peter Postma <peter@webdeveloping.nl>

package require Tcl 8.2
package require http

bind pub -|- "!google" pub:google

proc pub:google {nick uhost hand chan text} {
  global lastbind

  if {[string length [string trim [lindex $text 0]]] == 0} {
    putquick "NOTICE $nick :Usage: $lastbind <keywords>"
    return 0
  }

  regsub -all { } [join $text] {+} search
  set google(url) "http://www.google.nl/search?q=$search"
  set google(page) [http::config -useragent "Mozilla"]
  if {[catch {set google(page) [http::geturl $google(url) -timeout 15000]} msg]} {
    putquick "NOTICE $nick :Can't connect ($msg)"
    return 0
  }
  set google(data) [http::data $google(page)]

  regexp -nocase {related:(.*?)>} $google(data) t link1
  regexp -nocase {related:.*?>.*?related:(.*?)>} $google(data) t link2
  regexp -nocase {related:.*?>.*?related:.*?>.*?related:(.*?)>} $google(data) t link3

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
  putquick "PRIVMSG $chan :$output"

  catch { http::cleanup $google(page) }
  return 0
}

putlog "Google script 0.1 loaded!"
