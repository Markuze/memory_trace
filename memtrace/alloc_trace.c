#include <linux/init.h>
#include <linux/module.h>
#include <linux/relay.h>
#include <linux/debugfs.h>
#include <linux/kthread.h> //for testing
#include <asm/msr.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tracing I/O allocations");
MODULE_AUTHOR("Markuze Alex");

#define MEMTRACE_NAPI	(1 << 0)
#define MEMTRACE_WRITE	(1 << 1)
#define MEMTRACE_ALLOC  (1 << 2)

struct percpu_mdata {
	union {
		u64 curr;
		u8  unused[INTERNODE_CACHE_BYTES];
	};
};

struct trace_entry {
	union {
		u64	addr;
		struct {
			u32 tx;
			u32 rx;
		};
	};
	u32	tsc;
	u16	size;
	u16	flags;
};

#define SUBUFF_SIZE	(sizeof(struct trace_entry) << 20)
#define N_SUBBUFFS	1
#define STOP_MASK	((BIT(13) -1))

static struct rchan *rchan;
static struct dentry *dir, *active, *size;
static bool trace_active __read_mostly;
static u64 trace_global_len __read_mostly;

static struct percpu_mdata mdata[NR_CPUS] __cacheline_aligned_in_smp;

static inline void alloc_trace(struct trace_entry *entry)
{
	struct percpu_mdata *local = &mdata[smp_processor_id()];
	++local->curr;

	if (unlikely(local->curr >= trace_global_len)) {
		trace_active = 0;
		return;
	}
	if (!(local->curr & STOP_MASK)) {
		int i = NR_CPUS;
		u64 total = 0;
		while (i--) {
			total += mdata[i].curr;
		}
		if (unlikely(total >= trace_global_len)) {
			trace_active = 0;
		}

	}

	__relay_write(rchan, entry, sizeof(struct trace_entry));
}

void alloc_trace_alloc(u64 addr, u16 size, int is_tx, int is_alloc)
{
	struct trace_entry entry;

	if (!trace_active)
		return;

	entry.addr = addr;
	entry.size = size;
	entry.tsc = (u32)(rdtsc_ordered() >> 5);
	entry.flags = ((is_tx) ? MEMTRACE_WRITE : 0) |
			((is_alloc) ? MEMTRACE_ALLOC : 0);
	alloc_trace(&entry);

}
EXPORT_SYMBOL(alloc_trace_alloc);

void alloc_trace_napi(uint32_t tx, uint32_t rx)
{
	struct trace_entry entry;

	if (!trace_active)
		return;

	entry.tx = tx;
	entry.rx = rx;
	entry.tsc = (u32)(rdtsc_ordered() >> 5);
	entry.flags = MEMTRACE_NAPI;
	alloc_trace(&entry);
}
EXPORT_SYMBOL(alloc_trace_napi);

static struct dentry *create_handler(const char *name,
				     struct dentry *parent,
				     umode_t mode,
				     struct rchan_buf *buf,
				     int *is_global)
{
	return debugfs_create_file(name, mode, parent, buf,
				   &relay_file_operations);
}

static int remove_handler(struct dentry *dentry)
{
	debugfs_remove(dentry);
	return 0;
}

static struct rchan_callbacks relay_callbacks = {
	.create_buf_file = create_handler,
	.remove_buf_file = remove_handler,
};
//////////////////////////////////////////////////////
static struct task_struct *tasks[NR_CPUS];

#define TRACE_BATCH	256

static int tester_fn(void *arg)
{
	while( ! kthread_should_stop()) {
		int i = 0;
		for (i = 0; i < TRACE_BATCH; i++) {
			alloc_trace_alloc((u64)&tester_fn, i, 0, 0);
		}
		schedule();
	}
	return 0;
}

static int tester_init(void)
{
	int cpu;
	pr_info("spawning tracer threads\n");
	for (cpu = 0; cpu < num_online_cpus(); cpu++) {

		tasks[cpu] = kthread_create_on_node(tester_fn, NULL,
							cpu_to_node(cpu),
							"tester_%d", cpu);
		kthread_bind(tasks[cpu], cpu);
	}
	return 0;
}

static void tester_exit(void)
{
	int cpu;
	pr_info("stopping tracer threads\n");
	for (cpu = 0; cpu < num_online_cpus(); cpu++) {
		kthread_stop(tasks[cpu]);
	}
	pr_info("tracer threads done\n");
}

static bool test_running;
static int test_get(void *data, u64 *val)
{
	*val = test_running;
	return 0;
}
static int test_set(void *data, u64 val)
{
	test_running ^= 1;
	if (test_running)
		tester_init();
	else
		tester_exit();
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(test_fops, test_get,
			test_set, "%llu\n");
//////////////////////////////////////////////////////
static int __init memtrace_init(void)
{
	trace_global_len = SUBUFF_SIZE/sizeof(struct trace_entry);
	dir = debugfs_create_dir("alloc_trace", NULL);
	active = debugfs_create_bool("active", 0666, dir, &trace_active);
	size   = debugfs_create_u64("size", 0666, dir, &trace_global_len);
	rchan = relay_open("memtrace", dir, SUBUFF_SIZE, N_SUBBUFFS, &relay_callbacks, NULL);
	if (unlikely(!rchan)) {
		pr_err("Failed to relay_open...");
		return 0;
	}
	pr_info("memtarce loaded\n");
	pr_info("start testing...\n");
	debugfs_create_file("toggle_test", 0666, dir, NULL,
				   &test_fops);
	return 0;
}

static void __exit memtrace_exit(void)
{
	if (likely(rchan)) {
		relay_close(rchan);
	}
	debugfs_remove_recursive(dir);
	pr_info("memtrace out [mdata %ld trace %ld]\n",
		sizeof(struct percpu_mdata),
		sizeof(struct trace_entry));
}

module_init(memtrace_init);
module_exit(memtrace_exit);

