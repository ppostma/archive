<?
  # $Id: login.php,v 1.1.1.1 2003-03-19 12:58:21 peter Exp $
  #
  # WebFTP client - login form
  #

  require_once('conf.php');
  require_once('header.php');
?>
<br />
<div align="center">
  <table border="0" width="40%" cellspacing="0" cellpadding="0">
  <tr>
    <td width="100%" align="center">
      <p><font size="5"><b>Web FTP Login</b></font></p>
    </td>
  </tr>
  <tr>
    <td>
      <div align="center"><hr /></div>
    </td>
  </tr>  
  </table>
  <table border="0" width="40%" cellspacing="0" cellpadding="0">
  <tr>
    <td width="100%" align="center"> 
       <form action="<?= $_SERVER['PHP_SELF'] ?>" method="post">
         <table border="0" cellspacing="0" cellpadding="5">
         <tr>
           <td>Server</td>
           <td><input size="30" name="server" value="<?= $server ?>" <?= ($change) ? "" : "readonly=\"readonly\"" ?> /></td>
         </tr>
         <tr>
           <td>Username</td>
           <td><input size="30" name="username" /></td>
         </tr>
         <tr>
           <td>Password</td>
           <td><input size="30" name="password" type="password" /></td>
         </tr>
         </table>
         <p><input type="submit" name="login" value="Login" /></p>
       </form>
    </td>
  </tr>
  </table>
</div>
<? 
  if (!empty($ftp->error))
    echo "<p>Error:</p> " . $ftp->error;

  require_once('footer.php'); 
?>
