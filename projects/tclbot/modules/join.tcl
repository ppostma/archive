# $Id: join.tcl,v 1.2 2003-08-01 23:42:33 peter Exp $

# TclBot Join log module.

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
