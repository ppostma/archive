<?
  # $Id: rename.php,v 1.1.1.1 2003-03-19 12:58:21 peter Exp $
  #
  # WebFTP client - rename form
  #
?>
  <a name="edit"></a>
  <p>Renaming: <b><?= $_GET['dir'] . $_GET['rename'] ?></b></p>
  <form action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
   To: <input size="50" name="new" value="<?= $_GET['dir'] . $_GET['rename'] ?>" />
   <input type="hidden" name="old" value="<?= $_GET['dir'] . $_GET['rename'] ?>" />
   <input type="hidden" name="dir"  value="<?= $_GET['dir'] ?>" />
   <input type="submit" name="rename" value=" Rename " />
  </form>
