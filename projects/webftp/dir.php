<? 
  # $Id: dir.php,v 1.3 2003-10-11 17:08:49 peter Exp $
  #
  # WebFTP client - dir
  #

  $ftp->dir();

?>
  <font size="4">Server: <?= $server ?> &nbsp; Current Directory: <?= $ftp->curdir ?> </font>
  <hr />
  <table border="0" cellspacing="0" cellpadding="10">
  <tr>
    <td>
      <form action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
        Make new directory<br />
        <input size="25" name="mkdir" />
        <input type="hidden" name="dir" value="<?= $ftp->curdir ?>" />
        <input type="submit" name="newdir" value=" Ok " />
      </form>
    </td>
    <td>
      <form enctype="multipart/form-data" action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
        Upload file<br />
        <input type="file" name="<?= $tempfile ?>" id="<?= $tempfile ?>" class="form" size="25" />
        <input type="hidden" name="dir" value="<?= $ftp->curdir ?>" />
        <input type="submit" name="upload" value=" Ok " />
      </form>
    </td>
    <td>
      <br />
      <form action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
        <input type="submit" name="goroot" value=" Go to root " />
      </form>
    </td>
    <td>
      <br />
      <form action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
        <input type="submit" name="logout" value=" Logout " />
      </form>
    </td>
  </tr>
  </table>
  <hr />
  <table cellspacing="5" cellpadding="0" border="0">
  <tr>
    <td>&nbsp;</td>
    <td><i>File/directory</i></td>
    <td><i>Size</i></td>
    <td><i>Permissions</i></td>
    <td><i>Date modified</i></td>
    <td><i>Rename</i></td>
    <td><i>Chmod</i></td>
    <td><i>Delete</i></td>
  </tr>
  <tr>
    <td><img src="back.gif" alt="back" /></td>
    <td><a href="<?= $_SERVER['PHP_SELF'] ?>?cdup=<?= $ftp->curdir ?>">..</a></td>
  </tr>
  <?
    for ($i=0; $i<count($ftp->dirlist); $i++) {

      /* Yes, this works :-) */
      ereg("([-bcdl])([rwxstT-]{9}).+ ([0-9]*) ([a-zA-Z]+[0-9: ]* [0-9]{2}:?[0-9]{2}) (.+)",
           $ftp->dirlist[$i], $temp);

      $isdir   = ($temp[1] == 'd') ? 1 : 0;
      $chmod   = $temp[2];
      $size    = $temp[3];
      $time    = $temp[4];
      $filedir = $temp[5];

      if (empty($filedir)) continue;

      /* stupid hack */
      $dir = ($ftp->curdir == '/') ? "" : $ftp->curdir;
  ?>
  <tr>
    <td><img src="<?= ($isdir) ? "folder.gif" : "file.gif" ?>" alt="<?= ($isdir) ? "folder" : "file" ?>" /></td>
    <td><b><a href="<?= $_SERVER['PHP_SELF'] . "?dir=" . $ftp->curdir ?><?= (!$isdir) ? "&amp;file=" : "/" ?><?= $filedir ?>"><?= $filedir ?></a> &nbsp;</b></td>
    <td><?= round($size / 1024, 2) . " kB" ?> &nbsp;</td>
    <td><?= $chmod ?> &nbsp;</td>
    <td><?= $time  ?> &nbsp;</td>
    <td align="center"><a href="<?= $_SERVER['PHP_SELF'] . "?dir=" . $dir . "/&amp;rename=" . $filedir ?>#edit"><img src="rename.gif" alt="rename" border="0" /></a></td>
    <td align="center"><a href="<?= $_SERVER['PHP_SELF'] . "?dir=" . $dir . "/&amp;chmod="  . $filedir ?>#edit"><img src="chmod.gif" alt="chmod" border="0" /></a></td>
    <td align="center"><a href="<?= $_SERVER['PHP_SELF'] . "?isdir=" . $isdir . "&amp;dir=" . $dir . "/&amp;delete=" . $filedir ?>#edit"><img src="delete.gif" alt="delete" border="0" /></a></td>
  </tr>
  <? } ?>
  </table>
  <hr />
