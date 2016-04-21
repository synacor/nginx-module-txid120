#!/usr/bin/perl -w
use strict;

use MIME::Base64;
use Time::HiRes qw(time);

$|=1;
my %test;

my $bin = "txid120-test";

print "build    : ";
unlink($bin);
system("gcc -o $bin -Wall test.c ngx_http_txid120_logic.c") == 0 or die "failed to compile";
my (undef, undef, undef, undef, undef, undef, undef, $bin_size, undef, $bin_mtime) = stat($bin);
die "compiled file was not recently modified ($bin_mtime but now=".time.")" unless time - $bin_mtime < 5;
die "compiled file is zero bytes" unless $bin_size;
die "compiled file was not executable" unless -x $bin;
printf "%d bytes written %.02f seconds ago \e[32m[ OK ]\e[0m\n", $bin_size, time - $bin_mtime;
$test{compile} = 1;

my $time_start = time;
chomp(my $txid_raw = `./$bin`);
my $time_end = time;
unlink($bin);

die "expected 20 bytes of raw txid" unless length($txid_raw) == 20;
die "invalid characters in raw txid" unless $txid_raw =~ /^[0-9\:\@A-Za-z]{20}$/;
$test{raw} = 1;

(my $txid_b64 = $txid_raw) =~ tr/0-9:@A-Za-z/A-Za-z0-9+\//;

my $txid = decode_base64($txid_b64);

die "expected 15 bytes of decoded txid" unless length($txid) == 15;
$test{decode} = 1;

my $time_all = 0;
for (my $i=0; $i<=6; $i++) {
  $time_all += ord(substr($txid, $i, 1)) * 2**(8*(6-$i));
}
my $time_usec = $time_all % 1e6;
my $time_sec  = ($time_all - $time_usec) / 1e6;

$test{time} = $time_start < $time_sec+$time_usec/1e6 && $time_sec+$time_usec/1e6 < $time_end;
$test{rand} = randbit_prob(count_bits(substr($txid,7,7)), 8*8) > 1e-7;

my $test_num = keys %test;
my $test_pass = 0;
for my $test (keys %test) {
  if ($test{$test}) {
    $test{$test} = "\e[32m[ OK ]\e[0m";
    $test_pass++;
  } else {
    $test{$test} = "\e[31m[FAIL]\e[0m";
  }
}

print  "txid raw : $txid_raw $test{raw}\n";
print  "txid b64 : $txid_b64\n";
print  "txid hex : " . unpack("H*", $txid) . " $test{decode}\n";
print  "txid thex: " . unpack("H*", substr($txid, 0, 7)) . "\n";
printf "strt time: %.06f\n", $time_start;
printf "txid time: %d.%06d %s\n", $time_sec, $time_usec, $test{time};
printf "end  time: %.06f\n", $time_end;
print  "txid rand: " . unpack("H*", substr($txid, 7, 8)) . " $test{rand}\n";

if ($test_pass == $test_num && $test_num) {
  print "\nAll \e[32m$test_pass/$test_num\e[0m tests pass.\n";
  exit 0;
} else {
  print "\n\e[31m$test_pass/$test_num\e[0m tests passed!\n";
  exit 1;
}

sub factorial {
  $_[0] > 1 ? ($_[0] * factorial($_[0]-1)) : 1;
}
sub binom_coeff {
  my ($n, $k) = @_;
  die unless 0 <= $k && $k <= $n;
  return factorial($n) / (factorial($k) * factorial($n - $k));
}
sub count_bits {
  my $n = 0;
  $n += vec($_[0], $_, 1) for 1..length($_[0])*8;
  return $n;
}
sub randbit_prob {
  my ($on, $tot) = @_;
  binom_coeff($tot, $on) / 2**$tot
}
