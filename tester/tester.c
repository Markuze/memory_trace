#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/topology.h>
#include <linux/cpumask.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Testing tracing I/O allocations");
MODULE_AUTHOR("Markuze Alex");

void alloc_trace_alloc(u64 addr, u16 size, int is_tx, int is_alloc);

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

static int __init tester_init(void)
{
	int cpu;
	for (cpu = 0; cpu < num_online_cpus(); cpu++) {

		tasks[cpu] = kthread_create_on_node(tester_fn, NULL,
							cpu_to_node(cpu),
							"tester_%d", cpu);
		kthread_bind(tasks[cpu], cpu);
	}
	return 0;
}

static void __exit tester_exit(void)
{
	int cpu;
	for (cpu = 0; cpu < num_online_cpus(); cpu++) {
		kthread_stop(tasks[cpu]);
	}
}

module_init(tester_init);
module_exit(tester_exit);
