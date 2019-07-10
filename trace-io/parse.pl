#!/usr/bin/perl -w

use strict;
use autodie;
no warnings 'portable';

my $file = "./trace_vanila";

open (my $fh, "<", $file);

my @l1 = ();
my @l2 = ();

sub sim_l1 {
	my ($addr, $size) = @_;

	for ( my $i = 0; $i < $size ; $i++ ) {
		$l1[($i + $addr) &0x3f]++; # potentialy we could store the l1 addresses.
		$l2[($i + $addr) &0xff]++; # potentialy we could store the l1 addresses.
	}
}

for (<$fh>) {
	my @traces = /<(\w.*?t)/g;
#if (/t 1/) {
#	printf "$_";
#}
#next;
	foreach my $match (@traces) {
		next unless ($match =~ /\w+: (\w+) \[(\d+)\]/);
		my $addr = $1;
		my $size = $2/64;
		next if (hex($addr) == 0);
		$addr = hex($addr) >> 6;
		my $entry = $addr & 0x3f;
		#printf "$match (%x : $entry + $size)\n", $addr << 6;
		sim_l1($addr, $size);
	}
}
#exit;
print ("x, y\n");
my $i = 0;
for (@l1) {
	printf "$i, $_\n";
	$i++;
}
