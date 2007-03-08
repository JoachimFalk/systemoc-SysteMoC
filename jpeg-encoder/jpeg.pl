#! /usr/bin/perl -w
# vim: set sw=2 ts=8:

use strict;
use Math::Trig;
use Image::Magick;

my $prec = 9;

sub DCT1Dfact {
  my ( $x, $u ) = @_;
  my $Xk = (2*$x+1)*$u;
  my $c = cos($Xk*pi/16)/2;
  $c = $c/sqrt(2) if $u == 0;
  my $ca = abs($c);
  $ca = int($ca * (1 << $prec) + 0.5);
  return $ca, $c < 0;
}

{
  my ( $x, $u );
  my %facts;
  
  for ( $u = 0; $u <= 7; ++$u ) {
    for ( $x = 0; $x <= 7; ++$x ) {
      my ( $ca, $sign ) = DCT1Dfact($x,$u);
      unless ( defined $facts{$ca} ) {
	$facts{$ca} = {0=>[],1=>[],2=>[],3=>[],
		       4=>[],5=>[],6=>[],7=>[]};
      }
      $facts{$ca}{$u} = [] unless defined $facts{$ca}{$u};
      push @{$facts{$ca}{$u}}, $sign ? -$x : $x;
    }
  }

  print "\n";
  my $i = 0;
  foreach my $value ( sort { $a <=> $b } keys %facts ) {
    printf "%03s", $value;
#    foreach my $value2 ( sort { $a <=> $b } keys %facts ) {
#      print "\t", $value/$value2;
#    }
     foreach my $xstep ( sort { $a <=> $b } keys %{$facts{$value}} ) {
       printf " $xstep(%19s)", join( ',', @{$facts{$value}{$xstep}} );
     }
    print "\n";
  }
  print "\n";
}

{
  my ( $x, $u );
  my @maxresult = (0,0,0,0,0,0,0,0);

  for ( $u = 0; $u <= 7; ++$u ) {
    for ( $x = 0; $x <= 7; ++$x ) {
      my ( $c, $sign ) = DCT1Dfact($x,$u);
      
      if ( $sign ) {
	$maxresult[$u] += int( (0 * $c)/(1 << ($prec - 3)) + 0.5 );
      } else {
	$maxresult[$u] += int( (255 * $c)/(1 << ($prec - 3)) + 0.5 );
      }
    }
  }
  print "8-DCT maxresult:";
  print join( ", ", map { int ( $_ / 8 + 0.5 ) } @maxresult ), "\n";
}

{
  my ( $x, $y, $u, $v );
  my @maxresult = (0,0,0,0,0,0,0,0);

  for ( $u = 0; $u <= 7; ++$u ) {
    for ( $v = 0; $v <= 7; ++$v ) {
      for ( $x = 0; $x <= 7; ++$x ) {
	for ( $y = 0; $y <= 7; ++$y ) {
	  my ( $ca, $asign ) = DCT1Dfact($x,$u);
	  my ( $cb, $bsign ) = DCT1Dfact($y,$v);
	  my ( $c, $sign ) = ( $ca*$cb, $asign ^ $bsign );



	}
      }
    }
  }
}
