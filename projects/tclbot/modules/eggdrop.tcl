# $Id: eggdrop.tcl,v 1.1 2003-08-01 17:22:43 peter Exp $

# Eggdrop compatibility module for the TCLBot.
# This script adds the eggdrop specific functions to TCLBot.
# It's quite limited and buggy and very incomplete.

# Cheat :)
set alltools_loaded 1

# Global lists for the timers
set timers ""
set utimers ""

# Set a timer
proc timer {minutes procname} {
	global timers

	after [expr $minutes * 60 * 1000] $procname
	after [expr $minutes * 60 * 1000] "killtimer $procname"
	lappend timers [list $minutes $procname]
}

# Kill a timer
proc killtimer {procname} {
	global timers

	after cancel $procname
	after cancel "killtimer $procname"

	set i 0
	foreach t $timers {
		if {[string compare [lindex $t 1] $procname] == 0} {
			set timers [lreplace $timers $i $i]
			break
		}
		incr i
	}
}

# Check if a timer is set (eggdrop's alltools.tcl)
proc timerexists {procname} {
	global timers

	foreach t $timers {
		if {[string compare [lindex $t 1] $procname] == 0} {
			return [lindex $t 1]
		}
	}
	return
}

# Set a utimer
proc utimer {seconds procname} {
	global utimers

	after [expr $seconds * 1000] $procname
	after [expr $seconds * 1000] "killutimer $procname"
	lappend utimers [list $seconds $procname]
}

# Kill a utimer
proc killutimer {procname} {
	global utimers

	after cancel $procname
	after cancel "killutimer $procname"

	set i 0
	foreach t $timers {
		if {[string compare [lindex $t 1] $procname] == 0} {
			set utimers [lreplace $utimers $i $i]
			break
		}
		incr i
	}
}

# Check if a utimer is set (eggdrop's alltools.tcl)
proc utimerexists {procname} {
	global utimers

	foreach t $utimers {
		if {[string compare [lindex $t 1] $procname] == 0} {
			return [lindex $t 1]
		}
	}
	return
}

# Bind/Add a trigger
proc bind {type flags trigger procname} {
	addtrigger $type $flags $trigger $procname
}

# Send msg to IRC (should use a queue)
proc putserv {msg} { ircsend $msg }

# Send msg immediately to IRC
proc putquick {msg} { ircsend $msg }

# Send privmsg to IRC
proc putmsg {dest msg} { ircsend "PRIVMSG $dest :$msg" }

# Log messages
proc putlog {msg} { log $msg }

# Rehash, reloads configuration file & scripts
proc rehash {} { reload }

# Just exit, argument is optional
proc die {{text ""}} {
	ircsend "QUIT :$text"
	putlog "Shutting down..."
	exit 0
}
