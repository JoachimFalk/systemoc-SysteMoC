#! /usr/bin/perl -w 

use IO::File;

my $fh = IO::File->new($ARGV[0]);

while (!$fh->eof) {
  my $byte;

  $fh->read($byte, 1, 0);

  print ord($byte), ", \n";


}
