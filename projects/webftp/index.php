<?
  # $Id: index.php,v 1.3 2003-10-11 17:22:09 peter Exp $
  #
  # WebFTP client - index
  #

  require_once('timer.php');

  $ptime = new Timer();
  $ptime->start();

  require_once('conf.php');
  require_once('ftp.php');

  $ftp = new FTP_Client();

  session_start();

  if (isset($_POST['login']) || isset($_SESSION['login'])) {

    $username = (isset($_SESSION['username'])) ? $_SESSION['username']
                                               : $_POST['username'];
    $password = (isset($_SESSION['password'])) ? $_SESSION['password']
                                               : $_POST['password'];

    /* If the user is allowed to change the server then do so */
    if ($change) {
      $server = (isset($_SESSION['server'])) ? $_SESSION['server']
                                             : $_POST['server']; 
    }

    /* Connect to server with username and pass */
    if (!$ftp->open($server, $username, $password)) {
      require_once('login.php');
      exit;
    }


    /* --- 1. ---- */
    /* Actions that don't need to parse html */

    if (isset($_GET['dir']) && isset($_GET['file'])) {
      /* Download a file */

      $ftp->download($_GET['dir'], $_GET['file']);

    } elseif (isset($_GET['cdup'])) {
      /* Go to upper directory */

      $ftp->cdup($_GET['cdup']);

    } elseif (isset($_POST['goroot'])) {
      /* Go to root directory */

      $ftp->cd("/");
    
    } elseif (isset($_POST['logout'])) {
      /* Logout */

      session_unset();
      session_destroy();

      $ftp->logout();
      require_once('login.php');
      exit;

    } elseif (isset($_POST['upload'])) {
      /* Upload a file */

      $ftp->upload($_POST['dir'], $tempfile);

    } elseif (isset($_POST['newdir'])) {
      /* Make new directory */

      $ftp->mkdir($_POST['dir'] . "/" . $_POST['mkdir']);

    } elseif (isset($_POST['rename'])) {
      /* Rename form submitted */

      if (isset($_POST['new']) && isset($_POST['old']) && isset($_POST['dir'])) {
        if (!empty($_POST['new']) && !empty($_POST['old']) && !empty($_POST['dir'])) {
          $ftp->rename($_POST['old'], $_POST['new']);
        }
      }
    } elseif (isset($_POST['chmod'])) {
      /* CHMOD form submitted */

      if (isset($_POST['mode']) && isset($_POST['dir']) && isset($_POST['file'])) {
        if (!empty($_POST['mode']) && !empty($_POST['dir']) && !empty($_POST['file'])) {
          $ftp->chmod($_POST['dir'] . $_POST['file'], $_POST['mode']);
        }
      }
    } elseif (isset($_POST['delete'])) {
      /* Delete form submitted */

      if (isset($_POST['dir']) && isset($_POST['file'])) {
        if (!empty($_POST['dir']) && !empty($_POST['file'])) {
          $ftp->rm($_POST['dir'] . $_POST['file']);
        }
      }
    }

    /* Change directory when there's one in GET or POST */
    if (isset($_GET['dir'])) $ftp->cd($_GET['dir']);
    if (isset($_POST['dir'])) $ftp->cd($_POST['dir']);

    /* Parse header */
    require_once('header.php');

    /* List directory contents (and the rest) */
    require_once('dir.php');


    /* --- 2. ---- */
    /* Actions that need to parse html */

    if (isset($_GET['rename'])) {
      /* Rename file or directory */

      require_once('rename.php');

    } elseif (isset($_GET['chmod'])) {
      /* CHMOD file or directory */

      require_once('chmod.php');

    } elseif (isset($_GET['delete'])) {
      /* Delete file or directory */

      require_once('delete.php');
    }

  } else {
    /* Not logged in so go to login form */

    require_once('login.php');
    exit;
  }

  if (!empty($ftp->error))
    echo "<p>Error:</p> " . $ftp->error;

  $ptime->stop();
  echo "<div align=\"right\">Parse Time: ";
  echo round($ptime->time() * 1000 , 2);
  echo " milliseconds</div>";

  require_once('footer.php');

?>
