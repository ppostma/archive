# $Id: calc.tcl,v 1.2 2003-06-22 12:22:44 peter Exp $

# Calculator for an eggdrop
# version 1.0, 22/06/2003, by Peter Postma <peter@webdeveloping.nl>

bind pub -|- "!calc" pub:calc

set calc_version "1.0"

proc pub:calc {nick uhost hand chan text} {
  global lastbind
 
  if {[string length [string trim [lindex $text 0]]] == 0} {
    putserv "notice $nick :Syntax: $lastbind <math>"
    return 0
  }

  # translate 'x' to '*'
  regsub -all "x" $text "*" math

  # translate 'pi' to the number
  regsub -all "pi" $math "3.1415926535897932" math

  # do not allow brackets!!
  if {![regexp -all {\[} $text] || ![regexp -all {\]} $text]} {
    putserv "privmsg $chan :$text = [expr $math]"
  } else {
    putserv "notice $nick :sorry, those chars are not allowed."
  } 
  
  putlog "($lastbind) $nick $uhost at $chan: $text"
  
}

putlog "Calculator v$calc_version is loaded."
