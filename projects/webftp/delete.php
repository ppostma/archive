<?
  # $Id: delete.php,v 1.1.1.1 2003-03-19 12:58:21 peter Exp $
  #
  # WebFTP client - delete form
  #
?>
  <a name="edit"></a>
<?
  echo "<p>Are you sure you want to delete ";
  echo ($_GET['isdir'] == 0) ? "file" : "directory";
  echo " <b>" . $_GET['dir'] . $_GET['delete'] . "</b>";
  if ($_GET['isdir'] == 1) echo " and all contents in it";
  echo " ?</p>";
?>
  <form action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
   <input type="hidden" name="dir" value="<?= $_GET['dir'] ?>" />
   <input type="hidden" name="file"  value="<?= $_GET['delete'] ?>" />
   <input type="hidden" name="isdir" value="<?= $_GET['isdir'] ?>" />
   <input type="submit" name="delete" value=" Delete " />
  </form>
