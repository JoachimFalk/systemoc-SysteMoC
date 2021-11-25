#! /usr/bin/env perl
# vim: set sw=2 ts=8 sts=2 syn=perl expandtab:

use strict;
use warnings;

my $citation = 0;
while (<>) {
  $citation = 1 if m@^\\citation\{@;
}
if ($ENV{PS_OR_PDF} eq 'pdf') {
  print "$ENV{STEM}.pdf:";
} else {
  print "$ENV{STEM}.dvi:";
}
print " $ENV{STEM}.bbl" if $citation;
