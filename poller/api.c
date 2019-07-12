#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* uintmax_t */
#include <string.h>
#include <sys/mman.h>
#include <unistd.h> /* sysconf */

#define MGMT	"/proc/io_poller/mngmnt"
#define num_pages  8

struct polled_io_entry {
	int status;
	int len;
	//Add msg_head or whateva for udp.
	char buffer[0];
};

int main(void)
{
	char *addr, *ring;
	int i,  fd  = open(MGMT, O_RDWR|O_SYNC);
	if (fd < 0) {
		printf("failed to open %d\n", fd);
		return 1;
	}

	ring = mmap(0, num_pages << 12, PROT_READ|PROT_WRITE|MAP_POPULATE, MAP_SHARED, fd, 0);

	close(fd);

	printf("%p\n", ring);
	for (addr = ring, i = 0; i < num_pages; i++, addr+= (1<<12)) {
		//snprintf(addr, 64, "Hello\n");
		addr[0] = 0;
	}
#if 0
	for (addr = ring; addr < (ring + 64) /*(num_pages << 12))*/; addr+= 64) {
		struct polled_io_entry *io_entry = (struct polled_io_entry *)addr;
		io_entry->status = 1;
		io_entry->status = 64;
		snprintf(io_entry->buffer, 64, "Hello\n");
	}

	return 1;
#endif
}
