<?
  # $Id: timer.php,v 1.1.1.1 2003-03-19 12:58:21 peter Exp $
  #
  # Timer class
  # 

  class Timer
  {
    var $starttime;
    var $endtime;

    function Timer()
    {
      $this->reset();
    }

    function getmicrotime()
    {
      list($usec, $sec) = explode(" ", microtime());
      return ((float)$usec + (float)$sec);
    }    
 
    function start()
    {
      $this->starttime = $this->getmicrotime();
    }

    function stop()
    {
      $this->endtime = $this->getmicrotime();
    }

    function time()
    {
      return ($this->endtime - $this->starttime);
    }

    function reset()
    {
      $this->endtime = 0;
      $this->starttime = 0;
    }
  }

?>
