# $Id: tv.tcl,v 1.2 2003-03-19 17:03:09 peter Exp $

# tv.tcl / TV gids script for an eggdrop
# version 0.2 / 19/03/2003 / by Peter Postma <peter@webdeveloping.nl>

package require http

bind pub -|- "!tv" pub:tv
bind pub -|- "?tv" pub:tv

proc pub:tv {nick uhost hand chan text} {
  global lastbind

  if {[string length [string trim [lindex $text 0]]] == 0} {
    putquick "PRIVMSG $chan :syntax: $lastbind <kanaal> \[straks\]"
    return 0
  }

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
    "veronica" { set iptchan "Veronica" }
    "tmf"   { set iptchan "The Music Factory" }
    "mtv"   { set iptchan "MTV" }
    "bbc1"  { set iptchan "BBC 1" }
    "bbc2"  { set iptchan "BBC 2" }
    default { 
      putquick "PRIVMSG $chan :Kanaal niet beschikbaar."
      putquick "PRIVMSG $chan :Beschikbare kanalen zijn: nl1, nl2, nl3, rtl4, rtl5, sbs6, yorin, net5, v8, veronica, tmf, mtv, bbc1 & bbc2."
      return 0
    }
  }

  set page [http::config -useragent "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)"]
  set page [http::geturl $tvurl -timeout 15000]
  set lines [split [http::data $page] \n]
  set numlines [llength $lines]

  for {set i 0} {$i < $numlines} {incr i} {
    set line [lindex $lines $i]
    regexp -nocase "<b>(.*?)<a(.*?)>(.*?)</a>(.*?)</b>" $line blah tvtime blah tvprogram tvchan
    if {[regexp -nocase "<th(.*?)>(.*?)&nbsp;</th>" $line blah blah tvgenre]} {
      if {[string match -nocase "*($iptchan*)" $tvchan]} {
        putquick "PRIVMSG $chan :[string trim "$tvchan $tvtime [tv:fixchars $tvprogram] \[$tvgenre\]"]"
      }
    }
  }
}

proc tv:fixchars {text} {
  regsub -all "&amp;" $text "\\\&" text
  regsub -all "&eaml;" $text "\ë" text
  regsub -all "&uuml;" $text "\ü" text
  regsub -all "&eacute;" $text "\é" text
  return $text
}

putlog "TV script 0.2 loaded!"
