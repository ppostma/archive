# $Id: phpdoc.tcl,v 1.1 2003-03-20 16:45:44 peter Exp $

# phpdoc.tcl / php docu for an eggdrop
# version 0.2 / 20/03/2003 / by Peter Postma <peter@webdeveloping.nl>    

### Configuration settings:

# flags needed to use the command !php
set phpdoc_flag "-|-"

# phpdoc funcsummary.txt file
set phpdoc_file "/home/ai/scripts/my/funcsummary.txt"

# channels where the bot doesn't responds to triggers (seperate with spaces) 
set phpdoc_nopub ""

### End Configuration settings


set phpdoc_version "0.2"

bind pub $phpdoc_flag "!php"    pub:phpdoc
bind pub $phpdoc_flag "!phpdoc" pub:phpdoc

proc pub:phpdoc {nick uhost hand chan text} {
  global lastbind phpdoc_file phpdoc_nopub

  if {[lsearch -exact $phpdoc_nopub [string tolower $chan]] >= 0} { return 0 }
  if {[string length [string trim [lindex $text 0]]] == 0} {
    putquick "notice $nick :Syntax: $lastbind <function>"
    return 0
  }

  set input [lindex $text 0]
  regsub -all {\(} $input "" input
  regsub -all {\)} $input "" input

  set read_doc [open $phpdoc_file r]
  while {[gets $read_doc line] >= 0} {
    if {[regexp "^(.+) $input\[\(](.+)\[\)\]$" $line]} {
      gets $read_doc next
      putquick "PRIVMSG $chan :$line - [string trim $next] - http://nl.php.net/$input"
      close $read_doc
      return 0
    } 
  }

  putquick "PRIVMSG $chan :No match for \'$input\' - try http://nl.php.net/$input ?"
  close $read_doc
  return 0
}
  
putlog "PHPdoc version $phpdoc_version: Loaded!"
