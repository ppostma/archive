# $Id: kick.tcl,v 1.2 2003-08-01 23:42:33 peter Exp $

# TclBot kick module.

# Add proc
addproc "irc:kick"

# Kick, rejoin if it's me
proc irc:kick {data} {
	global botnick

	if {[regexp "^:(.+)\!(.+)\@(.+) KICK :?(#.+?) (.+?) :(.+)$" $data t nick user host channel who reason]} {
		if {[string compare $who $botnick] == 0} {
			if {[string match "stay*" $reason]} {
				log "$nick kicked me from $channel :("
			} else {
				log "$nick kicked me from $channel, trying to rejoin..."
				ircsend "JOIN :$channel"
			}
		}
	}
}
