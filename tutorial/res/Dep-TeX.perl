#! /usr/bin/env perl
# vim: set sw=2 ts=8 sts=2 syn=perl expandtab:

use strict;
use warnings;

use vars qw($GRAPHICPOSTFIX);

$GRAPHICPOSTFIX = $ENV{PS_OR_PDF};
$GRAPHICPOSTFIX = "pdf" unless defined $GRAPHICPOSTFIX && $GRAPHICPOSTFIX;

my $SRCDIR = $ENV{SRCDIR} || ".";
my @TEXDEPS = ();
my @PRODUCTIONS = (
      [qr{^(.*)$}               => sub { "$1" }],
      [qr{^(.*)\.pdf$}          => sub { "$1.ps" }],
      [qr{^(.*)\.pdf$}          => sub { "$1.eps" }],
      [qr{^(.*)\.pdf$}          => sub { "$1.fig" }],
      [qr{^(.*)\.pdf$}          => sub { "$1.dot" }],
      [qr{^(.*)\.pdf$}          => sub { "$1.plt" }],
      [qr{^(.*)\.pdf$}          => sub { "$1.dia" }],
      [qr{^(.*)\.pdf$}          => sub { "$1.svg" }],
      [qr{^(.*)\.pdf$}          => sub { "$1.odg" }],
      [qr{^(.*)\.eps$}          => sub { "$1.fig" }],
      [qr{^(.*)\.eps$}          => sub { "$1.dot" }],
      [qr{^(.*)\.eps$}          => sub { "$1.plt" }],
      [qr{^(.*)\.eps$}          => sub { "$1.dia" }],
      [qr{^(.*)\.eps$}          => sub { "$1.svg" }],
      [qr{^(.*)\.eps$}          => sub { "$1.odg" }],
      [qr{^(.*)-fig\.tex$}      => sub { "$1.fig" }],
      [qr{^(.*)-tex\.pdf$}      => sub { "$1.fig" }],
      [qr{^(.*)-tex\.ps$}       => sub { "$1.fig" }],
      # .dot can be converted to .fig
      [qr{^(.*)-fig\.tex$}      => sub { "$1.dot" }],
      [qr{^(.*)-tex\.pdf$}      => sub { "$1.dot" }],
      [qr{^(.*)-tex\.ps$}       => sub { "$1.dot" }]);

sub texlocate {
  my ( $locate, @ftypes ) = @_;
  return undef if $locate =~ m/^#[0-9]$/;
  $locate =~ s{\\%}{%}g;
  foreach my $ft (undef, @ftypes) {
    my $input = $locate;
    next if  (defined $ft) &&  ($input =~ m/\.(\w+)$/ && $1 ne "tikz");
    next if !(defined $ft) && !($input =~ m/\.\w+$/) && ($#ftypes >= 0);
    $input .= ".".$ft if defined $ft;
    foreach my $i (split(/:/, ".:$ENV{TEXINPUTS}")) {
      foreach my $m (@PRODUCTIONS) {
        my ($p,$t) = @$m;
        if ( $input =~ $p ) {
          my $inputtransformed = &{$t};
          if ( -f "$SRCDIR/$i/$inputtransformed" ||
               -f "$i/$inputtransformed" ) {
            return "$i/$input";
          }
        }
      }
    }
    foreach my $i (qw($ENV{TEX_STDINCLUDES})) {
      foreach my $m (@PRODUCTIONS) {
        my ($p,$t) = @$m;
        if ( $input =~ $p ) {
          my $inputtransformed = &{$t};
          if ( -f "$SRCDIR/$i/$inputtransformed" ||
               -f "$i/$inputtransformed" ) {
            return undef;
          }
        }
      }
    }
  }
  return $locate;
}
while (<stdin>) {
  s@(^|[^\\])%.*$@$1@;
  s@\\graphicPostfix\b\s*@$GRAPHICPOSTFIX@g;
  s@\\resdir\b\s*@$ENV{RESDIR}/@g;
  s{\\includegraphics(?:|<[^>]*>)(?:|\[[^]]*\])\{([^\}]*)\}}{
      if ($GRAPHICPOSTFIX eq 'ps') {
        print " ", texlocate($1,$GRAPHICPOSTFIX,"eps","png");
      } elsif ($GRAPHICPOSTFIX eq 'eps') {
        print " ", texlocate($1,$GRAPHICPOSTFIX,"ps","png");
      } else {
        print " ", texlocate($1,$GRAPHICPOSTFIX,"png");
      }
   }ge;
  s{\\includepdf(?:|<[^>]*>)(?:|\[[^]]*\])\{([^\}]*)\}}{
      print " ", texlocate($1,"pdf");
   }ge;
  s{\\verbatiminput\{([^\}]*)\}}{
      print " ", texlocate($1);
   }ge;
  s{\\(?:input|include)\{([^\}]*)\}}{
      my $input = texlocate($1, "tex");
      if (defined $input) {
        if ($input =~ m/\.tex$/) {
          print " $input-dep";
        } else {
          print " $input";
        }
        push @TEXDEPS, "$input";
      }
   }ge;
  s{\\addplot.*\btable(?:\[[^]]*\])?\{([^\}]*)\}}{
      print " ", texlocate($1);
   }ge;
}
print "\n";
print map { "-include $_-dep\n" } @TEXDEPS;
