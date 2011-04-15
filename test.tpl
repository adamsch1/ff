<html>
<br><br><hr>

<% if $errors.email %>
<% $errors.email %><br>
<% endif %>

<% if $errors.password %>
<% $errors.password %><br>
<% endif %>

<form action="http://127.0.0.1:6969/blog/post" method="post">
  Email <input type="text" name="email" value="<% $email %>"><br>
  Password <input type="password" name="password" value="<% $password %>"><br>
  <input type="submit" value="Submit">
</form>

</html>
