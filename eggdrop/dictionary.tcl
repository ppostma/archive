# $Id: dictionary.tcl,v 1.1.1.1 2003-03-19 14:50:33 peter Exp $

# dictionary.tcl / dictionary script for an eggdrop
# version 0.8.3 / 24/08/2002 / by Peter Postma <peter@webdeveloping.nl>    
#
# changes from 0.8 - 0.8.1: (12/08/2002)
#   - problem with brackets fixed
#   - underline option added to suffix
#   - some code clean up
#
# changes from 0.8.1 - 0.8.2: (18/08/2002)
#   - shared database option added
#
# changes from 0.8.2 - 0.8.3: (24/08/2002)
#   - code optimized
#
# With this script, your users can create their own dictionary. 
# They can add words and can let explain them. 
# Users with a special flag (configurable) can delete words.
# 
# To add words use:      !define or !learn
# To explain a word use: !explain or !what
# To delete a word use:  !delword
# To delete the whole dictionary in a channel use: !deldict
#
# Please change the variables below to configure this script.
#

### Configuration settings:

# flags needed to use the !explain and !what command
set explain_flag "-|-"

# flags needed to use the !define and !learn command
set learn_flag   "-|-"

# flags needed to use the !delword command
set delete_flag  "m|m"

# flags needed to use the !deldict command
set deldict_flag "n|n"

# channels where the dictionary is disabled (seperate with spaces)
set dict_nopub ""

# prefix for the dictionary file (.dict should be ok)
set dict_file ".dict"

# suffix to add behind the the word+explanation. 
# %b = bold, %u = underline, %nick = user nick, %channel = which channel 
set dict_suffix "- by %b%nick%b" 
#set dict_suffix "- by %b%nick%b in %channel"

# use a shared database for the channels, or create database for each channel?
# set to 1: yes, shared database or 0: no, create database for each channel
set dict_shared_db 1

### End Configuration settings


### Begin TCL code (DO NOT CHANGE SOMETHING AFTER THIS LINE)

set dictversion "0.8.3"

bind pub $learn_flag   "!learn"   pub:dict_learn
bind pub $learn_flag   "!define"  pub:dict_learn
bind pub $explain_flag "!what"    pub:dict_explain
bind pub $explain_flag "!explain" pub:dict_explain
bind pub $explain_flag "??"       pub:dict_explain
bind pub $delete_flag  "!delword" pub:dict_delword
bind pub $deldict_flag "!deldict" pub:dict_deldict

proc pub:dict_learn {nick uhost hand chan text} {
  global lastbind dict_nopub dict_file dict_suffix dict_shared_db

  if {[lsearch -exact $dict_nopub [string tolower $chan]] >= 0} { return 0 }
  if {[string length [string trim [lindex $text 1]]] == 0} {
    putquick "notice $nick :Syntax: $lastbind <keyword> <explanation>"
    return 0
  }

  if {$dict_shared_db} { 
    set dict_dbfile "$dict_file"
  } else {
    set dict_dbfile "$dict_file$chan"
  }

  check_dict_file $dict_dbfile

  set lword [lindex $text 0]
  set lexplain [lrange $text 1 end] 

  set read_dict [open $dict_dbfile r]
  while {[gets $read_dict line] >= 0} {
    if {[string match -nocase "\002$lword\002:*" $line]} {
      putquick "PRIVMSG $chan :$nick: I know \'$lword\' already."
      close $read_dict
      return 0
    } 
  }
  close $read_dict
  
  set wword "\002$lword\002: $lexplain $dict_suffix"
  
  regsub -all "%b" $wword "\002" wword
  regsub -all "%nick" $wword $nick wword
  regsub -all "%u" $wword "\037" wword
  regsub -all "%channel" $wword $chan wword

  set write_dict [open $dict_dbfile a]
  puts $write_dict $wword
  close $write_dict

  putquick "notice $nick :Okay, word \'$lword\' added!"
  putlog "Word \'$lword\' in $chan added!"

  return 0
}

proc pub:dict_explain {nick uhost hand chan text} {
  global lastbind dict_nopub dict_file dict_shared_db

  if {[lsearch -exact $dict_nopub [string tolower $chan]] >= 0} { return 0 }
  if {[string length [string trim $text]] == 0} {
    putquick "notice $nick :Syntax: $lastbind <keyword>"
    return 0
  }

  if {$dict_shared_db} {
    set dict_dbfile "$dict_file"
  } else {
    set dict_dbfile "$dict_file$chan"
  }

  check_dict_file $dict_dbfile

  set eword [lindex $text 0]
  regsub -all \\135 $eword {\]} mword
  regsub -all \\133 $mword {\[} mword
  
  set read_dict [open $dict_dbfile r]
  while {[gets $read_dict line] >= 0} {
    if {[string match -nocase "\002$mword\002:*" $line]} {
      putquick "PRIVMSG $chan :[join $line]"
      close $read_dict
      return 0
    } 
  }
  close $read_dict
 
  putquick "PRIVMSG $chan :$nick: I don\'t know what \'$eword\' means."

  return 0
}

proc pub:dict_delword {nick uhost hand chan text} {
  global lastbind dict_nopub dict_file dict_shared_db

  if {[lsearch -exact $dict_nopub [string tolower $chan]] >= 0} { return 0 }
  if {[string length [string trim $text]] == 0} {
    putquick "notice $nick :Syntax: $lastbind <keyword>"
    return 0
  }

  if {$dict_shared_db} {
    set dict_dbfile "$dict_file"
  } else {
    set dict_dbfile "$dict_file$chan"
  }

  check_dict_file $dict_dbfile

  set dword [lindex $text 0]
  regsub -all \\135 $dword {\]} mword
  regsub -all \\133 $mword {\[} mword

  set read_dict [open $dict_dbfile r]
  set write_dict [open $dict_dbfile.tmp w]

  while {[gets $read_dict line] >= 0} {
    if {![string match -nocase "\002$mword\002:*" $line]} { 
      puts $write_dict $line 
    } else {
      putquick "notice $nick :Word \'$dword\' deleted!"
      putlog "Word \'$dword\' in $chan deleted!"
    }
  }
  
  close $write_dict
  close $read_dict
  
  file copy -force "$dict_dbfile.tmp" "$dict_dbfile"
  file delete -force "$dict_dbfile.tmp"
}

proc pub:dict_deldict {nick uhost hand chan text} {
  global dict_nopub dict_file dict_shared_db

  if {[lsearch -exact $dict_nopub [string tolower $chan]] >= 0} { return 0 }

  if {$dict_shared_db} {
    file delete -force $dict_file
    putquick "notice $nick :Dictionary database file (shared) deleted!"
    putlog "Dictionary database file (shared) deleted!"
  } else {
    file delete -force "$dict_file$chan"
    putquick "notice $nick :Dictionary database file deleted!"
    putlog "Dictionary database file deleted!"
  }
}

proc check_dict_file {dbfile} {
  if {![file exists $dbfile]} {
    set write_dict [open $dbfile w]
    close $write_dict
    putlog "Dictionary database file created"
  }
}

putlog "Dictionary version $dictversion: LOADED!"
