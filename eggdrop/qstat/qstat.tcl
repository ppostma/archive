# $Id: qstat.tcl,v 1.7 2003-07-08 21:31:33 peter Exp $

# Qstat script for the eggdrop, version 2.3, 04/07/2003 
# 
# This script will query gameservers using the qstat program to
# display server status and players using public commands. 
#
# History:
#  1.0 (original) by Mikael Blomqvist <micke@peachpuff.com>
#  1.5 by ST8 <st8@q3f.net> and in part by Ad <ad@contempt.org.uk>
#  1.7 by Peter Postma <peter@webdeveloping.nl>
#    - security hole fixed. (passing bad arguments to TCL's exec)
#    - display players fixed. 
#  1.8 Peter Postma <peter@webdeveloping.nl>
#    - doesn't need a temp file anymore to display player info
#    - use regsub for input checking 
#    - better error checking / error messages
#    - lot of clean up
#  2.0 by Peter Postma <peter@webdeveloping.nl>
#    - very nasty bugs fixed: endless long flood and bad errors
#    - wiped out alot code, rewrote the main function
#  2.1 by Peter Postma <peter@webdeveloping.nl>
#    - support for RTCW, Quake 1
#    - installation steps added :^)
#    - windrop fix (but still doesn't work perfect)
#  2.2 by Peter Postma <peter@webdeveloping.nl>
#    - added BF, Gamespy, QW and UT2003.
#    - added '-timeout 5' option in qstat exec
#  2.3 by Peter Postma <peter@webdeveloping.nl>
#    - trivial changes, make the script more logical & consistent.
#    - added flood protection
#    - added configurable method to send messages
#
# Installation steps:
# 1) Easiest way of installing: put all Qstat related files (players.qstat, 
#    server.qstat, qstat.tcl, qstat (executable)) into _ONE_ directory.
#    A good choice would be something like: /home/name/eggdrop/qstat 
#    or c:/windrop/qstat
# 2) Download the Qstat program from www.qstat.org and install it
#    to some directory on your system. 
# 3) Change the option "set pathqstat "/home/peter/AI/scripts/my/qstat" 
#    and set it to the path where the Qstat related files are installed.
# 4) Make sure the path you've just set also contains the files:
#    players.qstat & server.qstat. If not, copy them to that directory.
# 5) Optionally change some other configuration settings below.
# 6) Edit your eggdrop's configuration file and add the qstat.tcl script. 
#    If you don't how to do this, please RTFM :)
# 7) Rehash
# 8) Typ !qstat in the channel for a command list.
# 9) Have fun :)
#

### Configuration settings ###

# Path to qstat folder containing qstat stuff/scripts and the qstat program
set qstat(path) "/home/ai/scripts/my/qstat"

# Flags needed to use the commands
set qstat(flags) "-|-"

# Channels where the bot doesn't respond to triggers [seperate with spaces]
set qstat(nopub) ""

# flood protection: seconds between use of the triggers
# to disable: set it to 0
set qstat(antiflood) 10

# method to send the messages:
# 0 = Private message
# 1 = Public message
# 2 = Private notice
# 3 = Public notice
set qstat(method) 1

### End configuration settings ###


### Begin TCL code ###

set qstat(version) "2.3"

bind pub $qstat(flags) "!ut"  qstat:pub
bind pub $qstat(flags) "!hl"  qstat:pub 
bind pub $qstat(flags) "!cs"  qstat:pub 
bind pub $qstat(flags) "!qw"  qstat:pub
bind pub $qstat(flags) "!q1"  qstat:pub
bind pub $qstat(flags) "!q2"  qstat:pub
bind pub $qstat(flags) "!q3"  qstat:pub
bind pub $qstat(flags) "!rcw" qstat:pub
bind pub $qstat(flags) "!bf"  qstat:pub
bind pub $qstat(flags) "!gs"  qstat:pub
bind pub $qstat(flags) "!ut2k3"  qstat:pub
bind pub $qstat(flags) "!ut2003" qstat:pub

bind pub $qstat(flags) "!utp"  qstat:pub 
bind pub $qstat(flags) "!hlp"  qstat:pub
bind pub $qstat(flags) "!qwp"  qstat:pub
bind pub $qstat(flags) "!q1p"  qstat:pub
bind pub $qstat(flags) "!q3p"  qstat:pub
bind pub $qstat(flags) "!q2p"  qstat:pub
bind pub $qstat(flags) "!rcwp" qstat:pub
bind pub $qstat(flags) "!bfp"  qstat:pub
bind pub $qstat(flags) "!ut2k3p"  qstat:pub
bind pub $qstat(flags) "!ut2003p" qstat:pub

bind pub $qstat(flags) "!qstat" qstat:help

proc qstat:help {nick host hand chan arg} {
  global qstat

  # check if channel is allowed.
  if {[lsearch -exact $qstat(nopub) [string tolower $chan]] >= 0} { return 0 }

  # output qstat commands / help.
  putserv "NOTICE $nick :Qstat commands:"
  putserv "NOTICE $nick :\002!qw / !q1 / !q2 / !q3 / !rcw <ip/host>\002 - Displays status of queried Quake World, 1, 2, 3 or RTCW servers"
  putserv "NOTICE $nick :\002!ut / !ut2003 / !hl / !bf / !gs <ip/host>\002 - Displays status of queried UT(2003), Half-life, BF1942 and GameSpy servers"

  return 0
}

proc qstat:pub {nick host hand chan arg} {
  global lastbind qstat

  # check if channel is allowed.
  if {[lsearch -exact $qstat(nopub) [string tolower $chan]] >= 0} { return 0 }

  # only use one argument in the list.
  set arg [lindex $arg 0]

  # check for input.
  if {[string length [string trim $arg]] == 0 || [qstat:input_check $arg]} {
    putserv "NOTICE $nick :Syntax: $lastbind <ip/host>"
    return 0
  }

  # figure out which command was used.
  switch [string tolower $lastbind] {
    "!hl"     { set gametype "-hls";  set players 0 }
    "!cs"     { set gametype "-hls";  set players 0 }
    "!ut"     { set gametype "-uns";  set players 0 }
    "!qw"     { set gametype "-qws";  set players 0 }
    "!q1"     { set gametype "-qs";   set players 0 }
    "!q2"     { set gametype "-q2s";  set players 0 }
    "!q3"     { set gametype "-q3s";  set players 0 }
    "!rcw"    { set gametype "-rwm";  set players 0 }
    "!bf"     { set gametype "-gps";  set players 0 }
    "!gs"     { set gametype "-gps";  set players 0 }
    "!ut2k3"  { set gametype "-ut2s"; set players 0 }
    "!ut2003" { set gametype "-ut2s"; set players 0 }
    "!hlp"     { set gametype "-hls";  set players 1 }
    "!utp"     { set gametype "-uns";  set players 1 }
    "!qwp"     { set gametype "-qws";  set players 1 }
    "!q1p"     { set gametype "-qs";   set players 1 }
    "!q2p"     { set gametype "-q2s";  set players 1 }
    "!q3p"     { set gametype "-q3s";  set players 1 }
    "!rcwp"    { set gametype "-rwm";  set players 1 }
    "!bfp"     { set gametype "-gps";  set players 1 }
    "!ut2k3p"  { set gametype "-ut2s"; set players 1 }
    "!ut2003p" { set gametype "-ut2s"; set players 1 }
    default {
      putserv "NOTICE $nick :Unknown command."
      return 0
    }
  }

  # flood protection
  if {[info exists qstat(floodprot)]} {
    set diff [expr [clock seconds] - $qstat(floodprot)]
    if {$diff < $qstat(antiflood)} {
      putserv "NOTICE $nick :Trigger has just been used! Please wait [expr $qstat(antiflood) - $diff] seconds..."
      return 0
    }
  }
  set qstat(floodprot) [clock seconds]

  # run the qstat program.
  if {$players == 1} { 
    set fd [open "|$qstat(path)/qstat -timeout 5 $gametype $arg -Ts $qstat(path)/server.qstat -Tp $qstat(path)/players.qstat -P" r]
  } else {
    set fd [open "|$qstat(path)/qstat -timeout 5 $gametype $arg -Ts $qstat(path)/server.qstat" r]
  }

  # output the result.
  qstat:results $fd $chan $nick $qstat(method)

  # close fork, end program.
  close $fd
  return 0
}

# show results.
proc qstat:results {fd chan nick method} {
  while {[gets $fd line] >= 0} { 
    if {[string match "DOWN*" $line]} {
      putquick "NOTICE $nick :Connection refused while querying server."
      break
    } elseif {[string match "HOSTNOTFOUND*" $line]} {
      putquick "NOTICE $nick :Host not found."
      break
    } elseif {[string match "TIMEOUT*" $line]} {
      putquick "NOTICE $nick :Timeout while querying server."
      break
    }
    switch -- $method {
      0 { putserv "PRIVMSG $nick :$line" }
      1 { putserv "PRIVMSG $chan :$line" }
      2 { putserv "NOTICE $nick :$line" }
      3 { putserv "NOTICE $chan :$line" }
      default { putserv "PRIVMSG $chan :$line" }
    }
  }
}

# check for valid characters
proc qstat:input_check {text} {
  if {[regexp \[^\[:alnum:\]_\.\:\-\] $text]} { return 1 }
  if {[string match "0*" $text]} { return 1 }
  return 0
}

putlog "Qstat4Eggdrop version $qstat(version): loaded."
