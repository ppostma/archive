# $Id: portscanner.tcl,v 1.3 2003-03-30 15:40:31 peter Exp $

# portscanner.tcl / portscanner script for an eggdrop 
# version 0.4beta / 30/03/2003 / by Peter Postma <peter@webdeveloping.nl>
#
# Use like this:
#  !portscan localhost        (scans localhost)
#  !portscan localhost 80     (scans port 80 on localhost)
#  !portscan -b localhost 22  (shows the banner on port 22 from localhost)
#  !portscan6 localhost       (scans localhost using IPv6)
#
# original idea & script: SilverSliver <silversliver2000@hotmail.com>
# improvements, bugfixes & banner grabbing: Peter <peter@webdeveloping.nl>

### Configuration

# Full path to the scan progra
set scanprog "/usr/home/peter/eggdrop/scripts/my/pscan/scan"

# Flags needed to use the command
set scan_flags "f|f"

# Scan which ports?
set scan_ports "21 22 23 25 37 53 79 80 110 111 113 135 137 139 143 443 445 587 1080 1214 2049 3306 8080"

# Channels where the command is disabled (seperate with spaces)
set scan_nopub ""

# The commands/triggers
set scan_trigger "!portscan"
set scan6_trigger "!portscan6"

### End Configuration


bind pub $scan_flags $scan_trigger pub:portscan
bind pub $scan_flags $scan6_trigger pub:portscan

set portscan_version "0.4beta"

proc pub:portscan {nick uhost hand chan text} {
  global lastbind scanprog scan_ports scan_nopub scan6_trigger

  if {[lsearch -exact $scan_nopub [string tolower $chan]] >= 0} { return 0 }

  if {[string length [lindex $text 0]] == 0} {
    putquick "NOTICE $nick :* Syntax: $lastbind \[-b\] <host/ip> \[port\]"
    return 0
  }

  if {[regexp \[^\[:alnum:\]\[:space:\]\.\:\-\] $text]} {
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

    for {set i 0} {$i < [llength $scan_ports]} {incr i} {

      if {$lastbind == $scan6_trigger} {
        catch {exec $scanprog -6 $host [lindex $scan_ports $i]} status
      } else {
        catch {exec $scanprog $host [lindex $scan_ports $i]} status
      }

      if {[regexp "^FAILED.+\[\(\](.*?)\[\)\]$" $status foo error]} {
        putquick "PRIVMSG $chan :* error: $error"
        return 0
      }

      if {[regexp "^CLOSED.*$" $status]} {
        incr closed
      } elseif {[regexp "^TIMEOUT.*$" $status]} {
        incr stealth
      } elseif {[regexp "^OPEN.+\[\(\](.*?)\[\)\].*$" $status foo strport]} {
        incr open
        putquick "PRIVMSG $chan :* [lindex $scan_ports $i] ($strport)"
      } else {
        putquick "PRIVMSG $chan :* hmmm... this shouldn't happen...quitting."
        return 0
      }
    }

    putquick "PRIVMSG $chan :* done! [llength $scan_ports] ports scanned. (open: $open, closed: $closed, stealth: $stealth)"
  } else {

    if {$lastbind == $scan6_trigger} {
      putquick "PRIVMSG $chan :* start scanning \[$host\]:$port"
      catch {exec $scanprog -6 $host $port} status
    } else {
      putquick "PRIVMSG $chan :* start scanning $host:$port"   
      catch {exec $scanprog $host $port} status
    }

    if {[regexp "^FAILED.+\[\(\](.*?)\[\)\]$" $status foo error]} {
      putquick "PRIVMSG $chan :* error: $error"
      return 0
    }

    if {[regexp "^CLOSED.+\[\(\](.*?)\[\)\].*$" $status foo strport]} {
      putquick "PRIVMSG $chan :* port $port ($strport) is CLOSED"
    } elseif {[regexp "^TIMEOUT.+\[\(\](.*?)\[\)\].*$" $status foo strport]} {
      putquick "PRIVMSG $chan :* no response from port $port ($strport)"
    } elseif {[regexp "^OPEN.+\[\(\](.*?)\[\)\].*$" $status foo strport]} {
      if {[lindex $text 0] == "-v" || [lindex $text 0] == "-b"} {
        if {$lastbind == $scan6_trigger} {
          set pf [open "| $scanprog -6b $host $port" r]
        } else {
          set pf [open "| $scanprog -b $host $port" r]
        }
        while {[gets $pf line] >= 0} {
          if {[regexp "^NOBANNER.+\[\(\](.*?)\[\)\].*$" $line foo strport]} {
            putquick "PRIVMSG $chan :* no banner on port $port ($strport) ?"
            return 0
          }
          putquick "PRIVMSG $chan :* $line"
        }
        close $pf
      } else {
        putquick "PRIVMSG $chan :* port $port ($strport) is OPEN"
      }
    } else {
      putquick "PRIVMSG $chan :* hmmm... something went wrong... :/"
    }
  }
}

putlog "Portscanner v$portscan_version: Loaded!"
