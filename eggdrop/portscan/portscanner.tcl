# $Id: portscanner.tcl,v 1.1.1.1 2003-03-19 14:50:33 peter Exp $

# portscanner.tcl / portscanner script for an eggdrop 
# version 0.3.4 / 24/11/2002 / by Peter Postma <peterpostma@yahoo.com> 
#
# dit script gebruikt scan.c (door Peter & Sliver) om een host te scannen.
# extra optie is banner grabbing. deze gebruikt banner.c (door Pano) om 
# de banner op een bepaalde poort te laten zien. 
# 
# compileer scan.c en banner.c alsvolgt:
#  $ gcc -Wall -o banner banner.c
#  $ gcc -Wall -o scan scan.c
#
# pas de onderstaande variabelen aan voor het juiste path van de proggies. 
# de variabele ports kan je ook aanpassen om het aantal poorten die gescant
# moeten worden aan te passen. maak deze niet te groot, op "stealth" hosts kan
# het scannen dan erg lang duren.
#
# gebruik in IRC: '!portscan <host/ip> <port>'. gebruik de optie '-v' 
# om de banners te grabben, dus: '!portscan -v <host/ip> <port>'.
# een o-flag is nodig om dit te kunnen gebruiken.
#
# VB: !portscan localhost       (scant localhost)
#     !portscan localhost 80    (scant poort 80 op localhost)
#     !portscan -v localhost 22 (laat de banner zien op poort 22, localhost)
#
# origineel idee en script: SilverSliver <silversliver2000@hotmail.com>
# verbeteringen, bugfixes en banner grabbing: Peter <peterpostma@yahoo.com>


# scan progs... lijkt me duidelijk 
set scanprog    "/usr/local/eggdrop/scripts/my/portscan/scan"
set scanprog6   "/usr/local/eggdrop/scripts/my/portscan/scan6"
set bannerprog  "/usr/local/eggdrop/scripts/my/portscan/banner"
set bannerprog6 "/usr/local/eggdrop/scripts/my/portscan/banner6"

# benodigde flags om te scannen
set scan_flags "f|f"

# poorten die gescant worden
set scan_ports "21 22 23 25 37 53 79 80 110 111 113 135 137 139 143 443 445 587 982 1080 1214 2049 3306 8080"

# channels waar de bot niet reageerd op triggers (scheiden met spatie)
set scan_nopub ""

# de triggers
set scan_trigger "!portscan"
set scan6_trigger "!portscan6"


bind pub $scan_flags $scan_trigger pub:portscan
bind pub $scan_flags $scan6_trigger pub:portscan

set portscan_version "0.3.4"

proc pub:portscan {nick uhost hand chan text} {
  global lastbind scanprog scanprog6 bannerprog bannerprog6 scan_ports scan_nopub scan6_trigger

  if {[lsearch -exact $scan_nopub [string tolower $chan]] >= 0} {return 0}

  if {[string length [lindex $text 0]] == 0} {
    putquick "NOTICE $nick :* syntax: $lastbind <host/ip> \[port\]"
    return 0
  }
 
  foreach char {">" "<" "|" "&"} {
    if [string match "*$char*" $text] {
      putquick "NOTICE $nick :* error: invalid input characters"
      return 0
    } 
  }

  if {[lindex $text 0] == "-v"} {
    set host [lindex $text 1]
    set port [lindex $text 2]
  } else {
    set host [lindex $text 0]
    set port [lindex $text 1]
  }

  if {$lastbind == $scan6_trigger} {
    set scanipv $scanprog6
    set banneripv $bannerprog6
  } else {
    set scanipv $scanprog
    set banneripv $bannerprog
  }

  if {[string length $port] == 0} {
 
    set open 0
    set closed 0
    set stealth 0    

    putquick "PRIVMSG $chan :* start scanning $host"

    for {set i 0} {$i < [llength $scan_ports]} {incr i} {

      catch {exec $scanipv $host [lindex $scan_ports $i]} status

      if {$status == "Error resolving hostname"} {
        putquick "PRIVMSG $chan :* error resolving hostname..."
        return 0
      }

      if {$status == "closed"} {
        incr closed
      } elseif {$status == "stealth"} {
        incr stealth
      } else {
        incr open
        putquick "PRIVMSG $chan :* $status"
      } 
    }  
  
    putquick "PRIVMSG $chan :* done! [llength $scan_ports] ports scanned. (open: $open, closed: $closed, stealth: $stealth)"
  } else {
    if {$lastbind == $scan6_trigger} {
      putquick "PRIVMSG $chan :* start scanning \[$host\]:$port"
    } else {
      putquick "PRIVMSG $chan :* start scanning $host:$port"   
    }
 
    catch {exec $scanipv $host $port} status

    if {$status == "Error resolving hostname"} {
      putquick "PRIVMSG $chan :* error resolving hostname..."
      return 0
    }

    if {$status == "closed"} {
      putquick "PRIVMSG $chan :* port $port is CLOSED"    
    } elseif {$status == "stealth"} {
      putquick "PRIVMSG $chan :* no response from port $port (STEALTH)"
    } else {
      if {[lindex $text 0] == "-v"} {
        set pf [open "| $banneripv $host $port" r]
        while {[gets $pf line] >= 0} { putquick "PRIVMSG $chan :* $line" }
        close $pf
      } else {
        putquick "PRIVMSG $chan :* port $port is OPEN ($status)"
      }
    }
  }
}

putlog "Portscanner v$portscan_version: Loaded!"
