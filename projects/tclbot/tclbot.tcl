# $Id: tclbot.tcl,v 1.2 2003-08-01 23:42:33 peter Exp $

#
# Copyright (c) 2003 Peter Postma <peter@webdeveloping.nl>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

# TclBot main.

package require Tcl 8.1

set botversion "0.1.2"
set sockirc 0
set lastbind ""

set procs ""
set triggers(lastid) 0

# Add a proc
proc addproc {procname} {
	global procs

	foreach p $procs {
		if {[string compare $p $procname] == 0} { return }
	}
	lappend procs $procname
}

# Delete a proc
proc delproc {procname} {
	global procs

	set i 0
	foreach p $procs {
		if {[string compare $p $procname] == 0} {
			set modules [lreplace $procs $i $i]
			break
		}
		incr i
	}
}

# Add a trigger
proc addtrigger {type flags trigger procname} {
	global triggers

	set id [incr triggers(lastid)]
	array set triggers [list $id,trigger $trigger $id,type $type $id,flags $flags $id,proc $procname]
}

# Get proc while searching for trigger
proc getprocbytrigger {name type} {
	global triggers

	foreach i [array names triggers *,trigger] {
		if {[string compare $triggers($i) $name] == 0} {
			set id [lindex [split $i ,] 0]
			if {[string compare [gettypebyid $id] $type] == 0} {
				return [getprocbyid $id]
			}
		}
	}
	return
}

# Get proc while searching for id
proc getprocbyid {id} {
	global triggers

	if {[array names triggers $id,proc] == "$id,proc"} {
		return $triggers($id,proc)
	}
	return
}

# Get type while searching for id
proc gettypebyid {id} {
	global triggers

	if {[array names triggers $id,type] == "$id,type"} {
		return $triggers($id,type)
	}
	return
}

# Get flags while searching for trigger
proc getflagsbytrigger {name} {
	global triggers

	foreach i [array names triggers *,trigger] {
		if {[string compare $triggers($i) $name] == 0} {
			return [getflagsbyid [lindex [split $i ,] 0]]
		}
	}
	return
}

# Get flags while searching for id 
proc getflagsbyid {id} {
	global triggers

	if {[array names triggers $id,flags] == "$id,flags"} {
		return $triggers($id,flags)
	}
	return
}

# Get flags while search for nickname
proc getflagsbynick {nick} {
	set flags ""

	if {[nickisoperator $nick]} { append flags "o" }
	if {[nickismaster $nick]} { append flags "m" }
	if {[nickisowner $nick]} { append flags "n" }

	return $flags
}

# Check is nick is an operator
proc nickisoperator {nick} {
	global operators

	foreach i [split $operators] {
		if {[string compare [string tolower $nick] $i] == 0} { return 1 }
	}
	return 0
}

# Check is nick is a master
proc nickismaster {nick} {
	global masters

	foreach i [split $masters] {
		if {[string compare [string tolower $nick] $i] == 0} { return 1 }
	}
	return 0
}

# Check if nick is an owner
proc nickisowner {nick} {
	global owners

	foreach i [split $owners] {
		if {[string compare [string tolower $nick] $i] == 0} { return 1 }
	}
	return 0
}

# Send text with timestamp to stdout
proc log {msg} {
	puts stdout "\[[clock format [clock seconds] -format "%H:%M"]\] $msg"
}

# Send text with timestamp to stderr
proc logerr {msg} {
	puts stderr "\[[clock format [clock seconds] -format "%H:%M"]\] $msg"
}

# Make connection to the IRC server
proc connect {} {
	global sockirc irchost ircport

	log "Connecting to $irchost:$ircport"
	if {[catch { set sockirc [socket $irchost $ircport] } err]} {
		logerr "Can't connect ($err)" 
		exit 1
	}
	fconfigure $sockirc -translation crlf
	fconfigure $sockirc -buffering line

	fileevent $sockirc readable readirc
}

# Send login commands to the IRC server
proc login {} {
	global botnick ident realname

	log "Sending pass/nick/user commands"
	ircsend "PASS secret"
	ircsend "NICK $botnick"
	ircsend "USER $ident * * :$realname"
}

# Send commands to the IRC server
proc ircsend {cmd} {
	global sockirc

	if {[catch { puts $sockirc $cmd } err]} {
		logerr "Error in ircsend: $err"
	}
}

# Read data from the IRC server
proc readirc {} {
	global sockirc

	if {[eof $sockirc] || [catch { gets $sockirc line }]} {
		logerr "Error while reading from IRC socket."
		close $sockirc
		exit 1
	}
	if {[catch { handleirc $line } err]} {
		logerr "Error while handling IRC data: $err"
	}
}

# Handle data from IRC server
proc handleirc {line} {
	global debug channels procs 

	# PING, send PONG
	if {[regexp "^PING :(.+)$" $line t pong]} {
		ircsend "PONG :$pong"

	# End of MOTD, start to join channels here
	} elseif {[regexp "^:(.+) 376 (.*?) :(.+)$" $line t server server what msg]} {

		foreach chan [split $channels] { ircsend "JOIN :$chan" }

	# Server error (4??) messages
	} elseif {[regexp "^:(.+) (\[4\]{1}\[0-9\]{2}) (.*?) :(.+)$" $line t server code what msg]} {

		log "($code) $what - $msg"
	}

	# Execute procs in the modules
	foreach p [split $procs] {
		if {[catch { $p $line } err]} {
			log "Error while executing '$p': $err"
		}
	}

	# Log data if debug == 'yes'
	if {[string compare [string tolower $debug] "yes"] == 0} { log $line }

	return 0
}

# Reload configuration/modules/scripts
proc reload {} {
	global argv
	uplevel #0 {
		if {[catch { source [lindex $argv 0] } err]} {
			log "Error while reloading: $err"
		}
	}
}

# Yes, I'm really TclBot :)
log "TclBot v$botversion by Peter Postma <peter@webdeveloping.nl>"

# Check which configfile to use
if {[file exists "default.conf"] && $argc < 1} {
	set conf "default.conf"
} elseif {$argc < 1} {
	logerr "Usage: $argv0 configuration.conf"
	return 1
} else {
	set conf [join $argv]
}

# Load configuration
if {[file exists $conf] == 0} {
	logerr "Can't open '$conf'."
	exit 1
}
if {[catch {source $conf} err]} {
	logerr "Error while loading the configuration: $err"
	exit 1
}

# Save botuptime
set botuptime [clock seconds]

# Connect to IRC
connect

# Send login commands to the IRC server
login

# Wait forever
vwait forever

return 0
