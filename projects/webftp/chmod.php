<?
  # $Id: chmod.php,v 1.1.1.1 2003-03-19 12:58:21 peter Exp $
  #
  # WebFTP client - chmod form
  #
?>
  <a name="edit"></a>
  <p>CHMOD file/directory: <b><?= $_GET['dir'] . $_GET['chmod'] ?></b></p>
  <form action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
    CHMOD value: <input size="4" name="mode" maxlength="4" value="0" />
    <input type="hidden" name="file" value="<?= $_GET['chmod'] ?>" />
    <input type="hidden" name="dir"  value="<?= $_GET['dir'] ?>" />&nbsp;
    <input type="submit" name="chmod" value=" chmod " />
  </form>
  <br />
  <table border="1">
  <caption>CHMOD Chart</caption>
  <tbody>
    <tr>
      <td>&nbsp;</td>
      <td align="center">Owner</td>
      <td align="center">Group</td>
      <td align="center">Public</td>
    </tr>
    <tr>
      <td>Read=4</td>
      <td align="center">x</td>
      <td align="center">x</td>
      <td align="center">x</td>
    </tr>
    <tr>
      <td>Write=2</td>
      <td align="center">x</td>
      <td>&nbsp;</td>
      <td>&nbsp;</td>
    </tr>
    <tr>
      <td>Execute=1</td>
      <td align="center">x</td>
      <td align="center">x</td>
      <td align="center">x</td>
    </tr>
    <tr>
     <td>Totals</td>
      <td>(4+2+1)=7</td>
      <td>(4+1)=5</td>
      <td>(4+1)=5</td>
    </tr>
  </tbody>
  </table>
