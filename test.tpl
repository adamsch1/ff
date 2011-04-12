<html>
<br><br><hr>
<% if $errors %>
There was a problem bub.<br>
<% endif %>

<% if $errors.password %>
Password is required<br>
<% endif %>

<form action="http://127.0.0.1:6969/blog/post" method="post">
  Email <input type="text" name="email"><br>
  Password <input type="password" name="password"><br>
  <input type="submit" value="Submit">
</form>

</html>
