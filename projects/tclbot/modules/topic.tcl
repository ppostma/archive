# $Id: topic.tcl,v 1.1 2004-02-01 16:38:11 peter Exp $

# TclBot topic module.

# Add proc
addproc "irc:topic"

set _topic ""

# Topic info from the server
proc irc:topic {data} {
	global _topic

	# No topic
	if {[regexp "^:(.+) 331 (.+?) (#.+?) :(.+?)$" $data t server botname channel notopic]} {
		set _topic $notopic
	}

	# The topic itself
	if {[regexp "^:(.+) 332 (.+?) (#.+?) :(.+?)$" $data t server botname channel thetopic]} {
		set _topic $thetopic
	}

	# Topic info
	if {[regexp "^:(.+) 333 (.+?) (#.+?) (.+?) (.+?)" $data t server botname channel who timestamp]} {
		# nothing yet
	}
}

# Use a trigger to call this function
proc topic {channel {newtopic ""}} {
	global _topic

	if {$newtopic != ""} {
		ircsend "TOPIC $channel :$newtopic"
		return
	} else {
		ircsend "TOPIC $channel"
		vwait _topic
		return $_topic
	}
}
