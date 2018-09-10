sudo taskset -c 3 ./udp_server.pl 5555 2> /dev/null &
sudo taskset -c 3 ./udp_server.pl 5556 2> /dev/null &
sudo taskset -c 3 ./udp_server.pl 5557 2> /dev/null &
sudo taskset -c 3 ./udp_server.pl 5558 2> /dev/null &
wait
