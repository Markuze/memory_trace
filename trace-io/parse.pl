#!/usr/bin/perl -w

use strict;
use autodie;

my $file = "./trace_damn";

open (my $fh, "<", $file);

for (<$fh>) {
	my @traces = /<(\w.*?t)/g;
	foreach my $match (@traces) {
		printf "$match\n";
	}
}
