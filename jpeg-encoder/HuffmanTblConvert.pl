#! /usr/bin/perl -w

use strict;

sub fromhexdigit {
  my $cc = ord($_);
  
  return $cc - ord("0") if $cc >= ord("0") && $cc <= ord("9");
  return $cc - ord("a") + 10 if $cc >= ord("a") && $cc <= ord("f");
  return $cc - ord("A") + 10 if $cc >= ord("A") && $cc <= ord("F");
  return undef;
}

sub fromhex {
  my $sum = 0;
  
  foreach my $digit (map { fromhexdigit $_; } split( //, $_[0]) ) {
    $sum = $sum * 16 + $digit;
  }
  return $sum;
}

sub frombin {
  my $sum = 0;
  
  foreach my $digit ( split( //, $_[0]) ) {
    $sum = $sum * 2 + $digit;
  }
  return $sum;
}

my %huffmantbl;

while ( my $line = <> ) {
  chomp $line;
  $line =~ m/^0x([[:xdigit:]]{2})\s+(\d+)\s*([01]*)/g;
  $huffmantbl{fromhex($1)} = [ length($3), frombin($3) ];
}

for ( my $i = 0; $i < 256; $i++ ) {
  $huffmantbl{$i} = [0,0] unless defined $huffmantbl{$i};
}

foreach my $key (sort { $a <=> $b } keys %huffmantbl) {
  my $value = $huffmantbl{$key};
  if ( $value->[0] == 0 ) {
    print "{0,0},\n";
  } else {
    printf "{% 3d, 0x%04X }, // 0x%02X -> %18s\n",
      $value->[0], $value->[1],
      $key, sprintf( "0b%0$value->[0]b", $value->[1] );
  }
}
