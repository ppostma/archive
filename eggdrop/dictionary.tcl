# $Id: dictionary.tcl,v 1.6 2003-08-02 14:21:05 peter Exp $

# Dictionary script for the eggdrop
# version 0.8.4, 04/07/2003, by Peter Postma <peter@webdeveloping.nl>    
#
# Changelog:
#  0.8.4: (04/07/2003)
#   - fixed probs with strings/lists by using 'join' correctly.
#   - lot of code/style changes.
#  0.8.3: (24/08/2002)
#   - code optimized.
#  0.8.2: (18/08/2002)
#   - shared database option added.
#  0.8.1: (12/08/2002)
#   - problem with brackets fixed.
#   - underline option added to suffix.
#   - some code clean up.
#  0.8:
#   - first version.
#
# With this script, your users can create their own dictionary. 
# They can add words and can let explain them. 
# Users with a special flag (configurable) can delete words.
# 
# To add words use:      !define or !learn
# To explain a word use: !explain or !what or ??
# To delete a word use:  !delword
# To delete the whole dictionary in a channel use: !deldict
#
# Please change the variables below to configure this script.
#

### Configuration settings ###

# flags needed to use the !explain and !what command
set dict(explain_flag) "-|-"

# flags needed to use the !define and !learn command
set dict(learn_flag) "-|-"

# flags needed to use the !delword command
set dict(delete_flag) "m|m"

# flags needed to use the !deldict command
set dict(deldict_flag) "n|n"

# channels where the dictionary is disabled (seperate with spaces)
set dict(nopub) ""

# prefix for the dictionary file (.dict should be ok)
set dict(file) ".dict"

# suffix to add behind the the word+explanation. 
# %b = bold, %u = underline, %nick = user nick, %chan = which channel
set dict(suffix) "- by %b%nick%b" 
#set dict(suffix) "- by %b%nick%b in %chan"

# use a shared database for the channels, or create database for each channel?
# set to 1: yes, shared database or 0: no, create database for each channel
set dict(shared_db) 1

### End Configuration settings ###


### Begin Tcl code ###

set dict(version) "0.8.4"

bind pub $dict(learn_flag)   "!learn"   dict:learn
bind pub $dict(learn_flag)   "!define"  dict:learn
bind pub $dict(explain_flag) "!what"    dict:explain
bind pub $dict(explain_flag) "!explain" dict:explain
bind pub $dict(explain_flag) "??"       dict:explain
bind pub $dict(delete_flag)  "!delword" dict:delword
bind pub $dict(deldict_flag) "!deldict" dict:deldict

proc dict:learn {nick uhost hand chan text} {
  global lastbind dict

  if {[lsearch -exact $dict(nopub) [string tolower $chan]] >= 0} { return 0 }
  if {[string length [string trim [lindex $text 1]]] == 0} {
    putquick "NOTICE $nick :Syntax: $lastbind <keyword> <explanation>"
    return 0
  }

  if {$dict(shared_db)} { 
    set dict(dbfile) "$dict(file)"
  } else {
    set dict(dbfile) "$dict(file)$chan"
  }

  dict:checkfile $dict(dbfile)

  set lword [lindex $text 0]
  set lexplain [lrange $text 1 end] 

  set fd [open $dict(dbfile) r]
  while {[gets $fd line] >= 0} {
    if {[string match -nocase "\002$lword\002:*" $line]} {
      putquick "PRIVMSG $chan :$nick: I know \'$lword\' already."
      close $fd
      return 0
    } 
  }
  close $fd
  
  set wword "\002$lword\002: [join $lexplain] $dict(suffix)"
  
  regsub -all "%b" $wword "\002" wword
  regsub -all "%u" $wword "\037" wword
  regsub -all "%nick" $wword $nick wword
  regsub -all "%chan" $wword $chan wword

  set fd [open $dict(dbfile) a]
  puts $fd $wword
  close $fd

  putquick "NOTICE $nick :Okay, word \'$lword\' added!"
  putlog "Word \'$lword\' in $chan added!"

  return 0
}

proc dict:explain {nick uhost hand chan text} {
  global lastbind dict

  if {[lsearch -exact $dict(nopub) [string tolower $chan]] >= 0} { return 0 }
  if {[string length [string trim $text]] == 0} {
    putquick "NOTICE $nick :Syntax: $lastbind <keyword>"
    return 0
  }

  if {$dict(shared_db)} {
    set dict(dbfile) "$dict(file)"
  } else {
    set dict(dbfile) "$dict(file)$chan"
  }

  dict:checkfile $dict(dbfile)

  set word [lindex $text 0]
  
  set fd [open $dict(dbfile) r]
  while {[gets $fd line] >= 0} {
    if {[string match -nocase "\002$word\002:*" $line]} {
      putquick "PRIVMSG $chan :[join $line]"
      close $fd
      return 0
    } 
  }
  close $fd
 
  putquick "PRIVMSG $chan :$nick: I don\'t know what \'$word\' means."

  return 0
}

proc dict:delword {nick uhost hand chan text} {
  global lastbind dict

  if {[lsearch -exact $dict(nopub) [string tolower $chan]] >= 0} { return 0 }
  if {[string length [string trim $text]] == 0} {
    putquick "NOTICE $nick :Syntax: $lastbind <keyword>"
    return 0
  }

  if {$dict(shared_db)} {
    set dict(dbfile) "$dict(file)"
  } else {
    set dict(dbfile) "$dict(file)$chan"
  }

  dict:checkfile $dict(dbfile)

  set word [lindex $text 0]

  set fdr [open $dict(dbfile) r]
  set fdw [open $dict(dbfile).tmp w]

  while {[gets $fdr line] >= 0} {
    if {![string match -nocase "\002$word\002:*" $line]} { 
      puts $fdw $line 
    } else {
      putquick "NOTICE $nick :Word \'$word\' deleted!"
      putlog "Word \'$word\' in $chan deleted!"
    }
  }
  
  close $fdw
  close $fdr
  
  file copy -force "$dict(dbfile).tmp" "$dict(dbfile)"
  file delete -force "$dict(dbfile).tmp"
}

proc dict:deldict {nick uhost hand chan text} {
  global dict

  if {[lsearch -exact $dict(nopub) [string tolower $chan]] >= 0} { return 0 }

  if {$dict(shared_db)} {
    file delete -force $dict(file)
    putquick "NOTICE $nick :Dictionary database file (shared) deleted!"
    putlog "Dictionary database file (shared) deleted!"
  } else {
    file delete -force "$dict(file)$chan"
    putquick "NOTICE $nick :Dictionary database file deleted!"
    putlog "Dictionary database file deleted!"
  }
}

proc dict:checkfile {dbfile} {
  if {![file exists $dbfile]} {
    set fd [open $dbfile w]
    close $fd
    putlog "Dictionary database file created!"
  }
}

putlog "Dictionary version $dict(version) loaded!"
