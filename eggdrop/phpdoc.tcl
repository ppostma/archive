# $Id: phpdoc.tcl,v 1.5 2003-08-02 23:34:16 peter Exp $

# PHP Doc for the eggdrop
# version 0.4, 03/08/2003, by Peter Postma <peter@webdeveloping.nl>

### Configuration settings ###

# the triggers: [seperate with spaces]
set phpdoc(triggers) "!php !phpdoc"

# flags needed to use the triggers
set phpdoc(flags) "-|-"

# channels where the bot doesn't responds to triggers (seperate with spaces)
set phpdoc(nopub) ""

# flood protection: seconds between use of the triggers
# to disable: set it to 0
set phpdoc(antiflood) 5

# method to send the messages:
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set phpdoc(method) 1

# below you can change the layout of the output:
# %syntax = function syntax
# %descr  = description for the function
# %func   = function name
# %b      = bold text
# %u      = underlined text
set phpdoc(layout) "%syntax - %descr - http://nl.php.net/%func"

# message layout when the function doesn't match:
# %func   = function name
# %b      = bold text
# %u      = underlined text
set phpdoc(nomatch) "No match for '%func' - try http://nl.php.net/%func ?"

# phpdoc funcsummary.txt file
set phpdoc(file) "/usr/home/peter/eggdrop/scripts/my/funcsummary.txt"

### End Configuration settings ###



### Begin Tcl code ###

set phpdoc(version) 0.4

if {[info tclversion] < 8.1} {
  putlog "Cannot load [file tail [info script]]: You need at least Tcl version 8.1 and you have Tcl version [info tclversion]."
  return 1
}

foreach trigger [split $phpdoc(triggers)] {
  bind pub $phpdoc(flags) $trigger phpdoc:pub
}
catch { unset trigger }

proc phpdoc:pub {nick uhost hand chan text} {
  global lastbind phpdoc

  if {[lsearch -exact $phpdoc(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {[string length [string trim [lindex $text 0]]] == 0} {
    putquick "NOTICE $nick :Syntax: $lastbind <function>"
    return 0
  }

  if {[info exists phpdoc(floodprot)]} {
    set diff [expr [clock seconds] - $phpdoc(floodprot)]
    if {$diff < $phpdoc(antiflood)} {
      putquick "NOTICE $nick :Trigger has just been used! Please wait [expr $phpdoc(antiflood) - $diff] seconds..."
      return 0
    }
    catch { unset diff }
  }
  set phpdoc(floodprot) [clock seconds]

  set input [string trim [lindex $text 0]]

  switch $phpdoc(method) {
    0 { set method "PRIVMSG $nick" }
    1 { set method "PRIVMSG $chan" }
    2 { set method "NOTICE $nick" }
    3 { set method "NOTICE $chan" }
    default { set method "PRIVMSG $chan" }
  }

  if {[catch { set fd [open $phpdoc(file) r] } err]} {
    putquick "$method :$err"
    return 0
  }

  while {[gets $fd line] >= 0} {
    if {[regexp "^(.+) $input\\((.*?)\\)" $line]} {
      gets $fd next

      set outchan $phpdoc(layout)
      regsub -all "%syntax" $outchan [string trim $line] outchan
      regsub -all "%descr" $outchan [string trim $next] outchan
      regsub -all "%func" $outchan $input outchan
      regsub -all "%b" $outchan "\002" outchan
      regsub -all "%u" $outchan "\037" outchan
      putquick "$method :$outchan"

      close $fd
      return 0
    }
  }

  set outchan $phpdoc(nomatch)
  regsub -all "%func" $outchan $input outchan
  regsub -all "%b" $outchan "\002" outchan
  regsub -all "%u" $outchan "\037" outchan
  putquick "$method :$outchan"

  close $fd
  return 0
}

putlog "PHPdoc version $phpdoc(version): loaded!"
