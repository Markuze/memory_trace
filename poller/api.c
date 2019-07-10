#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* uintmax_t */
#include <string.h>
#include <sys/mman.h>
#include <unistd.h> /* sysconf */

#define MGMT	"/proc/io_poller/mngmnt"
int main(void)
{
	char *addr;
	int fd  = open(MGMT, O_RDWR|O_SYNC);
	if (fd < 0) {
		printf("failed to open %d\n", fd);
		return 1;
	}

	addr = mmap(0, 4 << 12, PROT_READ|PROT_WRITE|MAP_POPULATE, MAP_SHARED, fd, 0);

	close(fd);

	printf("%p\n", addr);
	snprintf(addr, 64, "Hello\n");
	return 1;
}
