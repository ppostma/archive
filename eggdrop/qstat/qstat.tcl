# $Id: qstat.tcl,v 1.1.1.1 2003-03-19 14:50:33 peter Exp $

# qstat.tcl / qstat script for an eggdrop / version 2.1 / 26/10/2002 
# 
# This script will query gameservers using the qstat program to
# display server status and players using public commands. 
#
# History:
#  1.0 (original) by Mikael Blomqvist <micke@peachpuff.com>
#  1.5 by ST8 <st8@q3f.net> and in part by Ad <ad@contempt.org.uk>
#  1.7 by Peter Postma <peterpostma@yahoo.com>
#    - security hole fixed. (passing bad arguments to TCL's exec)
#    - display players fixed. 
#  1.8 Peter Postma <peterpostma@yahoo.com>
#    - doesn't need a temp file anymore to display player info
#    - use regsub for input checking 
#    - better error checking / error messages
#    - lot of clean up
#  2.0 by Peter Postma <peterpostma@yahoo.com>
#    - very nasty bugs fixed: endless long flood and bad errors
#    - wiped out alot code, rewrote the main function
#  2.1 by Peter Postma <peterpostma@yahoo.com>
#    - support for RTCW, Quake 1
#    - installation steps added :^)
#    - windrop fix
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
# Configuration settings:

# Flags needed to use the commands
set qstat_flag "-|-"

# Path to qstat folder containing qstat stuff/scripts and the qstat program
set pathqstat "/home/peter/AI/scripts/my/qstat"

# Channels you _dont_ want the bot to reply to public triggers on 
# (seperate with spaces):
set nopub ""

# End configuration settings



################################################################
# This is where the evil TCL code starts, read at your peril!  #
################################################################

set qversion "2.1"

bind pub $qstat_flag "!ut"  pub:qstat
bind pub $qstat_flag "!hl"  pub:qstat 
bind pub $qstat_flag "!cs"  pub:qstat
bind pub $qstat_flag "!q1"  pub:qstat 
bind pub $qstat_flag "!q2"  pub:qstat 
bind pub $qstat_flag "!q3"  pub:qstat 
bind pub $qstat_flag "!rcw" pub:qstat

bind pub $qstat_flag "!utp"  pub:qstat
bind pub $qstat_flag "!hlp"  pub:qstat
bind pub $qstat_flag "!q1p"  pub:qstat
bind pub $qstat_flag "!q3p"  pub:qstat
bind pub $qstat_flag "!q2p"  pub:qstat
bind pub $qstat_flag "!rcwp" pub:qstat

bind pub $qstat_flag "!qstat" pub:qstat_help

proc pub:qstat_help {nick host hand chan arg} {
  global pathqstat nopub

  # check if channel is allowed.
  if {[lsearch -exact $nopub [string tolower $chan]] >= 0} {return 0}

  # output qstat commands / help.
  putserv "NOTICE $nick :Qstat commands:"
  putserv "NOTICE $nick :\002!q1 / !q2 / !q3 / !ut / !hl / !rcw <ip/host>\002 - Displays status of queried Quake 1/2/3, UT, Half-life and RTCW servers"
  putserv "NOTICE $nick :\002!q1 / !q2p / !q3p / !utp / !hlp / !rcwp <ip/host>\002 - Displays all players on queried Quake 1/2/3, UT, Half-life and RTCW servers"
  return 0
}

proc pub:qstat {nick host hand chan arg} {
  global lastbind pathqstat nopub

  # check if channel is allowed.
  if {[lsearch -exact $nopub [string tolower $chan]] >= 0} {return 0}

  # only use one argument.
  set arg [lindex $arg 0]

  # check for input.
  if {[string length [string trim $arg]] == 0 || [qstat:input_check $arg] || [qstat:zero_check $arg]} {
    putquick "NOTICE $nick :Syntax: $lastbind <ip/host>"
    return 0
  }

  # figure out which command was used.
  switch [string tolower $lastbind] {
    "!hl"   { set gametype "-hls";  set players 0 }
    "!cs"   { set gametype "-hls";  set players 0 }
    "!ut"   { set gametype "-uns";  set players 0 }
    "!q1"   { set gametype "-qs";   set players 0 }
    "!q2"   { set gametype "-q2s";  set players 0 }
    "!q3"   { set gametype "-q3s";  set players 0 }
    "!rcw"  { set gametype "-rwm";  set players 0 }
    "!hlp"  { set gametype "-hls";  set players 1 }
    "!utp"  { set gametype "-uns";  set players 1 }
    "!q1p"  { set gametype "-qs";   set players 1 }
    "!q2p"  { set gametype "-q2s";  set players 1 }
    "!q3p"  { set gametype "-q3s";  set players 1 }
    "!rcwp" { set gametype "-rwm";  set players 1 }
    default {
      putquick "NOTICE $nick :Unknown command."
      return 0
    }
  }

  # run the qstat program.
  if {$players} { 
    set stat [open "|$pathqstat/qstat $gametype $arg -Ts $pathqstat/server.qstat -Tp $pathqstat/players.qstat -P" r]
  } else {
    set stat [open "|$pathqstat/qstat $gametype $arg -Ts $pathqstat/server.qstat" r]
  }

  # output the result.
  qstat:results $chan $nick $stat

  # close fork, end program.
  close $stat
  return 0
}

# show results.
proc qstat:results {chan nick pf} {
  while {[gets $pf line] >= 0} { 
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
    putquick "PRIVMSG $chan :$line"  
  }
}

# check for bad characters, if no check this can be exploited.
proc qstat:input_check {text} {
  foreach char {">" "<" "|" "&"} {
    if [string match "*$char*" $text] { return 1 }
  }
  return 0
}

# a trailing zero leads to an endless flood.
proc qstat:zero_check {text} {
  if {[string match "0*" $text]} { return 1 }
  return 0
}

putlog "Qstat4Eggdrop version $qversion: Loaded!"
