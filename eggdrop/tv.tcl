# $Id: tv.tcl,v 1.1.1.1 2003-03-19 14:50:33 peter Exp $

# tv.tcl / TV gids script for an eggdrop
# version 0.1 / 01/10/2002 / by Peter Postma <peter@webdeveloping.nl>
#
# no readme etc.. yet... this is a very beta version ;-)

package require http

bind pub -|- "!tv" pub:tv
bind pub -|- "?tv" pub:tv

proc pub:tv {nick uhost hand chan text} {

  if {[string trim [lindex $text 1]] == "straks"} {
    set tvurl "http://applic.portal.omroep.nl/gids/strax.php"
  } else {
    set tvurl "http://applic.portal.omroep.nl/gids/nu.php"
  }

  switch [string tolower [lindex $text 0]] {
    "nl1"   { set iptchan "Nederland 1" }
    "nl2"   { set iptchan "Nederland 2" }
    "nl3"   { set iptchan "Nederland 3" }
    "rtl4"  { set iptchan "RTL 4" }
    "rtl5"  { set iptchan "RTL 5" }
    "sbs6"  { set iptchan "SBS 6" }
    "yorin" { set iptchan "Yorin" }
    "net5"  { set iptchan "Net 5" }
    "v8"    { set iptchan "V8" }
    default { set iptchan "V8" }
  }

  set page [http::config -useragent "Eggdrop 1.6.12/HTTP"]
  set page [http::geturl $tvurl -timeout 15000]
  set lines [split [http::data $page] \n]
  set numlines [llength $lines]

  for {set i 0} {$i < $numlines} {incr i} {
    set line [lindex $lines $i]
    regexp -nocase "<b>(.*?)<a(.*?)>(.*?)</a>(.*?)</b>" $line blah tvtime blah tvprogram tvchan
    if {[regexp -nocase "<th(.*?)>(.*?)&nbsp;</th>" $line blah blah tvgenre]} {
      if {[string match -nocase "*($iptchan*)" $tvchan]} {
        putquick "PRIVMSG $chan :[string trim "$tvchan $tvtime [tv:convchars $tvprogram] \[$tvgenre\]"]"
      }
    }
  }
}

proc tv:convchars {text} {
  regsub -all "&amp;" $text "\\\&" text
  regsub -all "&eaml;" $text "\ë" text
  regsub -all "&uuml;" $text "\ü" text
  regsub -all "&eacute;" $text "\é" text
  return $text
}
