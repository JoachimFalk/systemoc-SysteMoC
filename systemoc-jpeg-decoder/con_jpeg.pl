#! /usr/bin/perl -w 

use IO::File;

if ($#ARGV < 0) {
  print "usage: con_jpeg.pl file1 [file2 [file3 [file4 [...]]]]\n";
}


for(my $i = 0; $i <= $#ARGV; $i++){
  my $fh = IO::File->new($ARGV[$i]);

  if (!$fh->eof){
    # File is not empty
    # Insert start of file-marker
    print "-1, \n";
  }

  while (!$fh->eof) {
    my $byte;

    $fh->read($byte, 1, 0);

    print ord($byte), ", \n";
  }


}
