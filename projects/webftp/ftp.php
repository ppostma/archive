<?
  # $Id: ftp.php,v 1.1.1.1 2003-03-19 12:58:21 peter Exp $
  #
  # WebFTP client - ftp class
  #

  class FTP_Client
  {
    var $id, $conn;
    var $curdir, $dirlist;
    var $error;

    function FTP_Client()
    {
      $this->error = "";
    }

    function open($server, $username, $password)
    {
      $this->id   = @ftp_connect($server);
      $this->conn = @ftp_login($this->id, $username, $password);

      if (!$this->id)
        $this->error .= "<p>Connection to " . $server . " failed.</p>";
      if (!$this->conn)
        $this->error .= "<p>The username/password was not accepted.</p>";

      if (!$this->id || !$this->conn) return false;

      setcookie("login",    "yeah");
      setcookie("server",   $server);
      setcookie("username", $username);
      setcookie("password", $password);

      return true;
    }

    function dir()
    {
      $this->curdir  = @ftp_pwd($this->id);
      $this->dirlist = @ftp_rawlist($this->id, $this->curdir);
    }

    function cd($dir)
    {
      $dir = stripslashes($dir);
      if (!@ftp_chdir($this->id, $dir))
        $this->error .= "<p>Changing directory to <b>" . $dir . "</b> failed.</p>";
    }

    function cdup($dir)
    {
      $this->cd($dir);
      if (!@ftp_cdup($this->id))
        $this->error .= "<p>Changing directory UP failed.</p>";
    }

    function mkdir($dir)
    {
      if (!@ftp_mkdir($this->id, $dir))
        $this->error .= "<p>Mkdir <b>" . $dir . "</b> failed.</p>";
    }

    function rename($old, $new)
    {
      if (!@ftp_rename($this->id, $old, $new))
        $this->error .= "<p>Renaming <b>" . $old . "</b> to <b>" . $new . "</b> failed.</p>";
    }

    function chmod($filedir, $mode)
    {
      if (!@ftp_site($this->id, "CHMOD " . $mode . " " . $filedir))
        $this->error .= "<p>CHMOD <b>" . $filedir . "</b> to value<b>" . $mode . "</b> failed.</p>";
    }

    function rm($filedir)
    {
      if (@ftp_size($this->id, $filedir) == -1) {
        $this->rmdir($filedir);
      } else {
        if (!@ftp_delete($this->id, $filedir))
          $this->error .= "<p>Attempted to delete file <b>" . $filedir . "</b> but it failed</p>";
      }
    }

    function rmdir($dir) {
      $afiles = @ftp_nlist($this->id, $dir);
      if (is_array($afiles)) {
        for ($i=0; $i<count($afiles); $i++) {
          $tfile = $afiles[$i];
          if (@ftp_size($this->id, $tfile) == -1) {
            $this->rmdir($tfile);
          } else {
            if (!@ftp_delete($this->id, $tfile))
              $this->error .= "<p>Attempted to delete file <b>" . $tfile . "</b> but it failed</p>";
          }
        }
      }
      if (!@ftp_rmdir($this->id, $dir))
       $this->error .= "<p>Attempted to delete directory <b>" . $dir . "</b> but it failed</p>";
    }

    function download($dir, $file)
    {
      header("Content-type: application/octetstream");
      header("Content-disposition: attachment; filename=$file");
      header("Pragma: no-cache");
      header("Expires: 0");

      $full  = $dir . "/" . $file;
      $tname = "/tmp/tmp_ftp_" . $file;
      $tfile = @fopen($tname, "w");

      @ftp_fget($this->id, $tfile, $full, FTP_BINARY);
      @fclose($tfile);

      $i = 0;
      $data = @readfile($tname);
      while ($data[$i]) echo($data[$i++]);

      @unlink($tname);
      exit;   /* stop execution or else our file may be screwed */
    }

    /* This is not really correct */
    function upload($dir)
    {
      if (is_uploaded_file($_FILES['file1']['tmp_name'])) {

        $up = @ftp_put($this->id, $dir . "/" . $_FILES['file1']['name'],
                       $_FILES['file1']['tmp_name'], FTP_BINARY);
        if (!$up)
          $this->error .= "<p>Upload of '<b>" . $_FILES['file1']['name'] . "</b>' failed.</p>";

      } else {
        $this->error .= "<p>'<b>" . $_FILES['file1']['tmp_name'] .
                        "</b>' is not an uploaded file.</p>";
      }
    }

    function quit()
    {
      @ftp_quit($this->id);
    }

    function logout()
    {
      $this->quit();
      setcookie("login",    "");
      setcookie("server",   "");
      setcookie("username", "");
      setcookie("password", "");
    }
  }

?>
