# $Id: calc.tcl,v 1.4 2003-07-04 13:49:09 peter Exp $

# Simple calculator for the eggdrop
# version 1.1, 04/07/2003, by Peter Postma <peter@webdeveloping.nl>

bind pub -|- "!calc" pub:calc

set calc(version) "1.1"

proc pub:calc {nick uhost hand chan text} {
  global lastbind
 
  if {[string length [string trim [lindex $text 0]]] == 0} {
    putserv "NOTICE $nick :Syntax: $lastbind <math>"
    return 0
  }

  # translate 'x' to '*'
  regsub -all "x" $text "*" math

  # translate 'pi' to the number
  regsub -all "pi" $math "3.1415926535897932" math

  # do not allow brackets!!
  if {![regexp -all {\[} $text] || ![regexp -all {\]} $text]} {
    putserv "PRIVMSG $chan :$text = [expr $math]"
  } else {
    putserv "NOTICE $nick :sorry, those chars are not allowed."
  } 
  
  putlog "($lastbind) $nick $uhost in $chan: $text"
  
}

putlog "Calculator v$calc(version) is loaded."
