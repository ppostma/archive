# $Id: portscanner.tcl,v 1.11 2006-11-08 22:20:50 peter Exp $

# Portscan script for the eggdrop 
# version 0.4.2, 04/07/2003, by Peter Postma <peter@pointless.nl>
#
# Use like this:
#  !portscan localhost        (scans localhost)
#  !portscan localhost 80     (scans port 80 on localhost)
#  !portscan -b localhost 22  (shows the banner on port 22 from localhost)
#  !portscan6 localhost       (scans localhost using IPv6)
#
# Original idea & script: SilverSliver <silversliver2000@hotmail.com>
# Improvements, bugfixes & banner grabbing: Peter <peter@pointless.nl>

### Configuration ###

# Full path to the scan program
set scan(program) "/usr/home/peter/eggdrop/scripts/my/portscan/scan"

# Flags needed to use the command
set scan(flags) "f|f"

# Scan which ports?
set scan(ports) "21 22 23 25 37 53 79 80 110 111 113 135 137 139 143 443 445 587 1080 1214 2049 3306 8080"

# Channels where the command is disabled (seperate with spaces)
set scan(nopub) ""

# The commands/triggers
set scan(trigger) "!portscan"
set scan(trigger6) "!portscan6"

### End Configuration ###


bind pub $scan(flags) $scan(trigger) pub:portscan
bind pub $scan(flags) $scan(trigger6) pub:portscan

set scan(version) "0.4.2"

proc pub:portscan {nick uhost hand chan text} {
  global lastbind scan

  if {[lsearch -exact $scan(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {[string length [lindex $text 0]] == 0} {
    putquick "NOTICE $nick :* Syntax: $lastbind \[-b\] <host/ip> \[port/service\]"
    return 0
  }

  if {![regexp \[\[:alnum:\]\[:space:\]\.\:\-\] [join $text]]} {
    putquick "NOTICE $nick :* Error: Invalid characters!"
    return 0
  }

  if {[lindex $text 0] == "-v" || [lindex $text 0] == "-b"} {
    set host [lindex $text 1]
    set port [lindex $text 2]
  } else {
    set host [lindex $text 0]
    set port [lindex $text 1]
  }

  if {[string length $port] == 0} {

    set open 0
    set closed 0
    set stealth 0

    putquick "PRIVMSG $chan :* start scanning $host"

    for {set i 0} {$i < [llength $scan(ports)]} {incr i} {

      if {$lastbind == $scan(trigger6)} {
        catch {exec $scan(program) -6 $host [lindex $scan(ports) $i]} status
      } else {
        catch {exec $scan(program) $host [lindex $scan(ports) $i]} status
      }

      if {[regexp "^FAILED \[\(\](.*?)\[\)\]$" $status foo error]} {
        putquick "PRIVMSG $chan :* error: $error"
        return 0
      }

      if {[regexp "^CLOSED.*$" $status]} {
        incr closed
      } elseif {[regexp "^TIMEOUT.*$" $status]} {
        incr stealth
      } elseif {[regexp "^OPEN \[\(\](.*?)\[\)\]$" $status foo strport]} {
        incr open
        putquick "PRIVMSG $chan :* $strport"
      } else {
        putquick "PRIVMSG $chan :* $status"
        return 0
      }
    }

    putquick "PRIVMSG $chan :* scan finished! [llength $scan(ports)] ports scanned. (open: $open, closed: $closed, stealth: $stealth)"
  } else {

    if {$lastbind == $scan(trigger6)} {
      catch {exec $scan(program) -6 $host $port} status
    } else {
      catch {exec $scan(program) $host $port} status
    }

    if {[regexp "^FAILED \[\(\](.*?)\[\)\]$" $status foo error]} {
      putquick "PRIVMSG $chan :* error: $error"
      return 0
    }

    if {[regexp "^CLOSED \[\(\](.*?)\[\)\]$" $status foo strport]} {
      putquick "PRIVMSG $chan :* port $strport is CLOSED"
    } elseif {[regexp "^TIMEOUT \[\(\](.*?)\[\)\]$" $status foo strport]} {
      putquick "PRIVMSG $chan :* no response from port $strport"
    } elseif {[regexp "^OPEN \[\(\](.*?)\[\)\]$" $status foo strport]} {
      if {[lindex $text 0] == "-v" || [lindex $text 0] == "-b"} {
        if {$lastbind == $scan(trigger6)} {
          set pf [open "| $scan(program) -6 -b $host $port" r]
        } else {
          set pf [open "| $scan(program) -b $host $port" r]
        }
        while {[gets $pf line] >= 0} {
          if {[regexp "^NOBANNER \[\(\](.*?)\[\)\]$" $line foo strport]} {
            putquick "PRIVMSG $chan :* no banner on port $strport ?"
            return 0
          }
          putquick "PRIVMSG $chan :* $line"
        }
        close $pf
      } else {
        putquick "PRIVMSG $chan :* port $strport is OPEN"
      }
    } else {
      putquick "PRIVMSG $chan :* $status"
      return 0
    }
  }
}

putlog "Portscanner v$scan(version) loaded!"
