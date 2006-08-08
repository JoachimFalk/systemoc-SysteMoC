#! /usr/bin/env perl


# script is used to sum up configuration overhead for a given vcd trce file of rc

use warnings;
use strict;

# fix the @INC path
BEGIN {
  use Cwd 'abs_path';
  use File::Basename;
  use File::Spec;

  @INC = ( File::Spec->catdir( abs_path( dirname $0 ), "../../site_perl" ),
    grep { $_ ne '.' } @INC );
}

package main;

use warnings;
use strict;
use vars qw($prog);

use File::Spec;
use File::Basename;
use VCD;

# Get the program name
$prog = basename($0);

sub bin2int {
  my $result = 0;
  $_[0] =~ m/^b?([01]+)/;
  foreach my $digit (split('', $1)) {
    $result = ($result << 1) + $digit;
  }
  return $result;
}

#my $charA = 65;
#my $charP = 80;
my $charc = 99;

my $maxtime = 0;

foreach (@ARGV) {
  
  my $time = 0;
  my $sum = 0;
  my $vcd = VCD->new(File::Spec->catfile($_));

  my %test = %{$vcd->getAllSignalDef('conf\d+_\d+\s*(?:\[\d+:\d+\])?')};
  my @configs;
  
  foreach ( keys(%test) ) {
    push @configs, $test{$_}{"name"};
  }

  foreach my $data ( @{$vcd->getSignal(@configs)} ) {

    my $diff_time  = $data->[0] - $time;
    $time = $data->[0];
    my $state = bin2int($data->[1]);

    #if($state == $charA){
    #  $sums[0] += $diff_time;
    #}elsif($state == $charP){
    #  $sums[1] += $diff_time;
    #}elsif($state == $charc){
    if($state == $charc){  
      $sum += $diff_time;
    }

  }

  open(OUTF, ">$_.rt"); #open for write, overwrite
  print OUTF $sum."\n";
  close(OUTF);
  print "configuration time=".$sum." for ".$_."\n";
  $maxtime = $maxtime > $sum ? $maxtime : $sum;
}

open(OUTF, ">maxRCT.rt"); #open for write, overwrite
print OUTF $maxtime."\n";
close(OUTF);
print "max configuration time=".$maxtime."\n";

