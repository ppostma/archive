# $Id: kick.tcl,v 1.1 2003-08-01 17:22:43 peter Exp $

# TCLBot kick module.

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
