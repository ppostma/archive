# $Id: message.tcl,v 1.3 2004-02-01 15:19:57 peter Exp $

# TclBot Message module.
# Provides interface for PRIVMSG

# Add proc
addproc "irc:privmsg"

# Public/Private Messages
proc irc:privmsg {data} {
	global botnick lastbind

	if {[regexp "^:(.+?)\!(.+?)\@(.+?)??PRIVMSG??(#?.+?)??:(.+?)$" $data t nick user host channel msg]} {

		set trigger [lindex [split $msg] 0]
		set text [lrange [split $msg] 1 end]

		# Private
		if {[string compare $botnick $channel] == 0} {
			set type "msg"
		# Public
		} else {
			set type "pub"
		}

		# Get procname for trigger, if no proc avail, return
		set procname [getprocbytrigger $trigger $type]
		if {$procname == ""} { return }

		# Get the user's flags, append a '-' because everyone has it
		set flagsnick [getflagsbynick $nick]
		append flagsnick "-"

		# Get flags for the trigger
		set flagstrig [getflagsbytrigger $trigger]

		# Compare flags with each other and when they match, run proc
		for {set i 0} {$i < [string length $flagsnick]} {incr i} {
			set f [string index $flagsnick $i]
			if {[string match *$f* $flagstrig]} {
				set lastbind $trigger
				$procname $nick $nick!$user@$host $nick $channel $text
				return
			}
		}
	}
}
