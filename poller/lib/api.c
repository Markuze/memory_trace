#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* uintmax_t */
#include <string.h>
#include <sys/mman.h>
#include <unistd.h> /* sysconf */

#include <sched.h>

#define MGMT	"/proc/io_poller/mngmnt"
#define num_pages  8

struct polled_io_entry {
	volatile int status;
	int len;
	//Add msg_head or whateva for udp.
	char buffer[0];
};

int main(void)
{
	char *addr, *ring;

//sock....
//may need to write ito MGMT and then map per pid_$cnt sock
	int i,  fd  = open(MGMT, O_RDWR|O_SYNC);
	if (fd < 0) {
		printf("failed to open %d\n", fd);
		return 1;
	}

	ring = mmap(0, num_pages << 12, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_POPULATE|MAP_NORESERVE|MAP_LOCKED, fd, 0);

	close(fd);

	printf("%p\n", ring);
	for (addr = ring, i = 0; i < num_pages; i++, addr+= (1<<12)) {
		//snprintf(addr, 64, "Hello\n");
		printf(">>%s\n", addr);
	}
//end sock
	for (addr = ring , i = 0; addr < (ring + (num_pages << 12)); addr+= (64), i++) {
//sendto
		struct polled_io_entry *io_entry = (struct polled_io_entry *)addr;
		io_entry->len = (64 - sizeof(struct polled_io_entry));
		snprintf(io_entry->buffer, 64, "Hello[%d]", i);
		io_entry->status = (addr < (ring + (num_pages << 12))) ? 1 : 2;
//sendto
	}
	printf("sent %d\n packets\n", i);
//close may need to wait for all paclets to go away...
	for (addr = ring , i = 0; addr < (ring + (num_pages << 12)); addr+= (64), i++) {
		struct polled_io_entry *io_entry = (struct polled_io_entry *)addr;
		while (io_entry->status) {
			printf("Status of entry %d is %d", i, io_entry->status);
			sched_yield();
		}

	}

	return 0;
}
