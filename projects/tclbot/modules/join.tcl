# $Id: join.tcl,v 1.1 2003-08-01 17:22:43 peter Exp $

# TCLBot Join log module.

# Add proc
addproc "irc:join"

# Joins
proc irc:join {data} {
	global botnick

	if {[regexp "^:(.+)\!(.+)\@(.+) JOIN :?(.+)$" $data t nick user host channel]} {
		if {[string compare $nick $botnick] == 0} {
			log "Joined $channel"
		}
	}
}
