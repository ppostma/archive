# $Id: invite.tcl,v 1.2 2003-08-01 23:42:33 peter Exp $

# TclBot 'Join on invite' module.

# Add proc
addproc "irc:invite"

# Invitations
proc irc:invite {data} {
	if {[regexp "^:(.+)\!(.+)\@(.+) INVITE (.+) :?(#.+?)$" $data t nick user host who channel]} {
		log "$nick invites me to $channel, trying to join..."
		ircsend "JOIN :$channel"
	}
}
