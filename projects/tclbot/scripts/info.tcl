# $Id: info.tcl,v 1.2 2003-08-01 23:42:33 peter Exp $

# Info commands for the TclBot.
# Requires the message.tcl module to be loaded.

addtrigger pub - "!flags"     pub:flags
addtrigger pub - "!owner"     pub:owner
addtrigger pub - "!master"    pub:master
addtrigger pub - "!operator"  pub:operator
addtrigger pub - "!version"   pub:version
addtrigger pub - "!date"      pub:date
addtrigger pub - "!uptime"    pub:uptime 
addtrigger pub - "!os"        pub:os
addtrigger pub - "!botuptime" pub:botuptime

proc pub:flags {nick mask hand chan text} {
	set nickflags [getflagsbynick $nick]
	if {$nickflags == ""} {
		ircsend "PRIVMSG $chan :$nick: sorry, you don't have any flags."
	} else {
		ircsend "PRIVMSG $chan :$nick: your flags are '$nickflags'"
	}
}

proc pub:owner {nick mask hand chan text} {
	if {[nickisowner $nick]} {
		ircsend "PRIVMSG $chan :$nick: you are my owner!"
	}
}

proc pub:master {nick mask hand chan text} {
	if {[nickismaster $nick]} {
		ircsend "PRIVMSG $chan :$nick: you are a master!"
	}
}

proc pub:operator {nick mask hand chan text} {
	if {[nickisoperator $nick]} {
		ircsend "PRIVMSG $chan :$nick: you are an operator!"
	}
}

proc pub:version {nick mask hand chan text} {
	global botversion
	ircsend "PRIVMSG $chan :I'm TclBot $botversion running on Tcl [info patchlevel]"
}

proc pub:date {nick mask hand chan text} {
	ircsend "PRIVMSG $chan :[clock format [clock seconds]]"
}

proc pub:uptime {nick mask hand chan text} {
	ircsend "PRIVMSG $chan :[exec uptime]"
}

proc pub:os {nick mask hand chan text} {
	ircsend "PRIVMSG $chan :I am running on: [exec uname -srm]"
}

proc pub:botuptime {nick mask hand chan text} {
        global botversion botuptime

	set up [expr ([clock seconds] - $botuptime)]
	set days [expr ($up / 86400)]
	set up [expr ($up % 86400)]
	set hours [expr ($up / 3600)]
	set up [expr ($up % 3600)]
	set mins [expr ($up / 60)]

	set show ""
	if {$days  > 0} { append show "$days days " }
	if {$hours > 0} { append show "$hours hours " }
	if {$mins  > 0} { append show "$mins minutes" }

	ircsend "PRIVMSG $chan :TclBot $botversion is up for $show."
}

log "TclBot Info commands: Loaded."
