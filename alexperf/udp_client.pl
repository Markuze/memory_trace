#!/usr/bin/perl -w

use strict;
use autodie;
use IO::Socket;
use Time::HiRes;

$| = 1; #?

my $serverip = '127.0.0.1';	#VMA
my $serverport = 5555;
my $size = (64 * 1024) - 28;

my $message = IO::Socket::INET->new(Proto=>"udp", PeerPort=>$serverport,
				     PeerAddr=>$serverip);
my $start = Time::HiRes::gettimeofday();
my $time = $start;
my $total = 0;
while (($time - $start) < 3) {
	$message->send("A"x(64*1024 -32));
	$time = Time::HiRes::gettimeofday() - $time;
	$total += (64*1024 -32);
}
	printf "syscall time sent %.3f \n", $total/(1024 * 1024 * 1024);
$message->close();
