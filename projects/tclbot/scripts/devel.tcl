# $Id: devel.tcl,v 1.2 2003-08-01 23:42:33 peter Exp $

# Developer commands for the TclBot.
# Has some handy functions which makes debugging quite easier.
# Requires the message.tcl module to be loaded.

addtrigger pub n "@irc"    pub:irc
addtrigger pub n "@tcl"	   pub:tcl
addtrigger pub n "@exec"   pub:exec
addtrigger pub n "@reload" pub:reload
addtrigger msg n "reload"  pub:reload
addtrigger msg n "say"     msg:say

proc pub:irc {nick mask hand chan text} {
	ircsend [join $text]
}

proc pub:tcl {nick mask hand chan text} {
	set errnum [catch { eval [join $text] } err]
	if {$err == ""} {
		set err "<empty string>"
	} else {
		set err "result: $err"
	}
	if {$errnum != 0} { set err "$err ($errnum)" }
	foreach line [split $err "\n"] { ircsend "PRIVMSG $chan :$line" }
}

proc pub:exec {nick mask hand chan text} {
	set errnum [catch { exec [join $text] } err]
	if {$err == ""} {
		set err "<empty string>"
	} else {
		set err "result: $err"
	}
	if {$errnum != 0} { set err "$err ($errnum)" }
	foreach line [split $err "\n"] { ircsend "PRIVMSG $chan :$line" }
}

proc pub:reload {nick mask hand chan text} {
	reload
}

proc msg:say {nick mask hand nick text} {
	set dest [lindex $text 0]
	if {$dest == ""} { return }
	set txt [lrange $text 1 end]
	if {$txt == ""} { return }
	ircsend "PRIVMSG $dest :[join $txt]"
}

log "Development (irc/tcl/exec) commands: Loaded."
