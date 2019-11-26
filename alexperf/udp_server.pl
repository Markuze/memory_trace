#!/usr/bin/perl -w

use strict;
use autodie;
use IO::Socket;

my $serverport = 5555;
$serverport = shift @ARGV if ($#ARGV == 0);

my $server = IO::Socket::INET->new(LocalPort=>$serverport, Proto=>"udp");

my ($datagram, $flags);
my $size = 64 * (1024);
my $total = 0;
my $pkt = 0;
$SIG{INT} = sub {printf "    Total = %.3f pkt $pkt\n",$total/(1024 * 1024 * 1024)};
$SIG{KILL} = sub {printf "[k]Total = %.3f pkt $pkt\n",$total/(1024 * 1024 * 1024)};
$SIG{TERM} = sub {printf "[t]Total = %.3f pkt $pkt\n",$total/(1024 * 1024 * 1024)};
while ($server->recv($datagram, $size, $flags)) {
	my $ipaddr = $server->peerhost;
	my $ipport = $server->peerport;
	$total += length($datagram);
	$pkt++;
	print "received ", length($datagram), "bytes from $ipaddr:$ipport\n";
}

