<?
  # $Id: ftp.php,v 1.5 2003-10-13 19:55:16 peter Exp $
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

      $_SESSION['login']    = "Y";
      $_SESSION['server']   = $server;
      $_SESSION['username'] = $username;
      $_SESSION['password'] = $password;

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
      global $temp_dir;

      header("Content-type: application/octetstream");
      header("Content-disposition: attachment; filename=$file");
      header("Pragma: no-cache");
      header("Expires: 0");

      $full  = $dir . "/" . $file;
      if (!isset($temp_dir)) $temp_dir = "/tmp";
      $tname = $temp_dir . "/tmp_ftp_" . $file;
      $tfile = @fopen($tname, "w");

      @ftp_fget($this->id, $tfile, $full, FTP_BINARY);
      @fclose($tfile);

      $i = 0;
      $data = @readfile($tname);
      while ($data[$i]) echo($data[$i++]);

      @unlink($tname);
      exit();		/* stop execution or else our file may be screwed */
    }

    function upload($dir, $file)
    {
      if (is_uploaded_file($_FILES[$file]['tmp_name'])) {

        $up = @ftp_put($this->id, $dir . "/" . $_FILES[$file]['name'],
                       $_FILES[$file]['tmp_name'], FTP_BINARY);
        if (!$up)
          $this->error .= "<p>Upload of '<b>" . $_FILES[$file]['name'] . "</b>' failed.</p>";

      } else {
        $this->error .= "<p>'<b>" . $_FILES[$file]['tmp_name'] .
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
    }
  }

?>
