  #!/usr/bin/perl -w
  use CGI;

  $upload_dir = "/Users/inskim/inskim/5circle/Webserv/upload_files";

  $query = new CGI;

  $filename = $query->param("filename");
  $filename =~ s/.*[\/\\](.*)/$1/;
  $upload_filehandle = $query->upload("filename");

  open UPLOADFILE, ">$upload_dir/$filename";

  while ( <$upload_filehandle> )
  {
    print UPLOADFILE;
  }

  close UPLOADFILE;

  print $query->header ( );
  print <<END_HTML

  <HTML>
  <HEAD>
  <TITLE>Thanks!</TITLE>
  </HEAD>

  <BODY>

  <P>Thanks for uploading your photo!</P>
  <img src="/upload/$filename" border="0">

  </BODY>
  </HTML>

  END_HTML