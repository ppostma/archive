# $Id: invite.tcl,v 1.1 2003-08-01 17:22:43 peter Exp $

# TCLBot 'Join on invite' module.

# Add proc
addproc "irc:invite"

# Invitations
proc irc:invite {data} {
	if {[regexp "^:(.+)\!(.+)\@(.+) INVITE (.+) :?(#.+?)$" $data t nick user host who channel]} {
		log "$nick invites me to $channel, trying to join..."
		ircsend "JOIN :$channel"
	}
}
