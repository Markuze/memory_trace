#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

struct trace_entry {
	union {
		uint64_t	addr;
		struct {
			uint32_t tx;
			uint32_t rx;
		};
	};
	uint32_t tsc;
	uint16_t size;
	uint16_t flags;
};

#define SUBUFF_SIZE	(sizeof(struct trace_entry) << 20)
#define N_SUBBUFFS	1

static inline void dump_entry(struct trace_entry *entry)
{
	printf("tsc %x \naddr %lx [%d]\n\n", entry->tsc, entry->addr, entry->size);
}

#define TRACE_PATH "/sys/kernel/debug/alloc_trace/"
int main(void)
{
	int acc = 0;
	char name[128];
	int fd;
	struct trace_entry *entry;


	if ((acc = access(TRACE_PATH, F_OK))) {
		printf("file %s doesnt exist(%d)...\n", TRACE_PATH, acc);
		return 0;
	}

	sprintf(name, "%smemtrace%d",TRACE_PATH, 0);
	fd = open(name, O_RDONLY);

	entry = mmap(NULL, SUBUFF_SIZE * N_SUBBUFFS, PROT_READ, MAP_SHARED, fd, 0);
	printf("fd %d for %s mapped to %p\n", fd, name, entry);
	for (acc = 0; acc < 16; acc++) {
		dump_entry(entry++);
	}
	return 0;
}

