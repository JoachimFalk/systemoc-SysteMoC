#! /usr/bin/perl -w 

use warnings;
use strict;

# fix the @INC path
BEGIN {
  use File::Spec;
  use File::Basename;
  use Cwd qw();
  
  @INC = grep { $_ ne '.' } @INC;
  unshift @INC, File::Spec->catfile(Cwd::abs_path(dirname($0)), "site_perl");
}

use IO::File;
use Getopt::Long;

use vars qw($prog);

# Get the program name
$prog = basename($0);


sub Usage {
  print $#ARGV."\n";
  my $error = (@_);

  print STDERR "\n".
  print STDERR "usage: $prog [-h|-?|--help] [-s|--switch] file1 [file2 [file3 [file4 [...]]]]\n";
  exit($error ? 1 : 0);
}

my $opt = {};

sub print_value {
  my $addr = shift;
  my $value = shift;

  if($opt->{'switch_output'}){
    print "    case $addr: temp_val = $value; break;\n";
  }else{
    print "$value, \n";
  }

}

my $p = new Getopt::Long::Parser;
$p->configure("gnu_getopt");
my $rc = $p->getoptions(
  'help|h|?'                => \$opt->{'help'},
  'switch|s'           => \$opt->{'switch_output'},
);

if (!$rc || $opt->{'help'} || 
    ($#ARGV <0)) {
  &Usage(!$opt->{'help'});
  
}


my $addr = 0;


if($opt->{'switch_output'}){
  print "short picture_memory(unsigned int addr){\n";
  print "  short temp_val = 0;\n";
  print "  switch(addr) {\n";
}


for(my $i = 0; $i <= $#ARGV; $i++){
  my $fh = IO::File->new($ARGV[$i]);

  if (!$fh->eof){
    # File is not empty
    # Insert start of file-marker
    print_value($addr,-1);
    $addr++;
  }

  while (!$fh->eof) {
    my $byte;

    $fh->read($byte, 1, 0);

    print_value($addr,ord($byte));
    $addr++;
  }


}



if($opt->{'switch_output'}){
  print "    default: temp_val = 0; break;\n";

  print "  }\n";
#  print "  block_data_size = $addr;\n";
  print "  return temp_val;\n";
  print "}\n";
}
