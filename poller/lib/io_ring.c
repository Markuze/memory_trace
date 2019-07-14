#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* uintmax_t */
#include <string.h>
#include <sys/mman.h>
#include <unistd.h> /* sysconf */
#include <netinet/in.h>


#include <sched.h>

#define MGMT	"/proc/io_poller/mngmnt"
#define num_pages  2

struct polled_io_entry {
	volatile int status;
	int len;
	struct 	sockaddr_in in;
	//Add msg_head or whateva for udp.
	char buffer[0];
};

struct ir_socket {
	char *mdata; //Must for vaiable size packets and TCP - Read only page
	char *ring;	//ring base
	void *tail;	//user poll updates
	void *head;	//user send updates
	int fd;
};

static struct ir_socket this_b;
static struct ir_socket *this;

int ir_socket(int af, int sock_type, int protocol)
{
	int fd  = open(MGMT, O_RDWR|O_SYNC);
	//may need to write ito MGMT and then map per pid_$cnt sock
	if (fd < 0) {
		printf("failed to open %d\n", fd);
		return -1;
	}
	this = &this_b;
	this->ring = mmap(0, num_pages << 12, PROT_READ|PROT_WRITE,
				MAP_SHARED|MAP_POPULATE|MAP_NORESERVE|MAP_LOCKED, fd, 0);
	this->tail = this->ring;
	this->head = this->ring;
	this->fd = fd;

	return fd;
	//close(fd);
}

ssize_t ir_sendto(int sockfd, const void *buf, size_t len, int flags,
		       const struct sockaddr *dest_addr, socklen_t addrlen)
{
	struct polled_io_entry *entry;

restart:
	entry  = this->head;

	while (entry->status != 0) {
		sched_yield();
	}

	entry->len = len; //+ sizeof(struct polled_io_entry);
	this->head += len + sizeof(struct polled_io_entry);

	//TODO: When not fixed size need to do tail check...
	//	must maske sure there is enough space...
	//	(Do not bother with wraparound/split copy....)
	if (((unsigned long)(this->head)) >= ((unsigned long)(this->ring +  (num_pages << 12)))) {
		entry->status = 2;
		this->head = this->ring;
		goto restart;
	}

	memcpy(entry->buffer, buf, len);
	//TODO: make  sure addrlen doesnt overflow...
	memcpy(&entry->in, dest_addr, addrlen);
	entry->status = 1;
	return len;
}

#if 0
ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{

}
#endif
#if 0
int main(void)
{
	char *addr, *ring;

//sock....
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
#endif
