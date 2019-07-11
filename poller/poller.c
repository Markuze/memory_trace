#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <linux/uaccess.h>
#include <linux/cpumask.h>

MODULE_AUTHOR("Markuze Alex markuze@cs.technion.ac.il");
MODULE_DESCRIPTION("Deferred I/O poller");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

#define POLLER_DIR_NAME "io_poller"
#define procname "mngmnt"

static struct proc_dir_entry *proc_dir;
static struct kmem_cache *priv_cache;
static LIST_HEAD(priv_list);

enum POLL_RING_STATUS {
	POLL_RING_STATUS_USER		= 0,
	POLL_RING_STATUS_KERNEL		= 1,
	POLL_RING_STATUS_KERNEL_RESET	= 2,
};

struct priv_data {
	struct list_head list;
	struct page *page;
	struct task_struct *task;
	struct vm_area_struct *vma;
	struct polled_io_entry *loc;
};

static ssize_t poller_write(struct file *file, const char __user *buf,
                              size_t len, loff_t *ppos)
{
	trace_printk("%s\n", __FUNCTION__);
	return len;
}

static ssize_t poller_read(struct file *file, char __user *buf,
                             size_t buflen, loff_t *ppos)
{
	struct priv_data *priv = file->private_data;
	if (!buf)
		return -EINVAL;

        /* Stats: will always print something,
         * cat seems to call read again if prev call was != 0
         * ppos points to file->f_pos */
        if ((*ppos)++ & 1)
                return 0;

	wake_up_process(priv->task);
#if 0
	for_each_online_cpu(cpu) {
		char line[256];
		int len;

		len = snprintf(line, 256, "online cpu %d\n", cpu);
		if (buflen <= cnt + len + 50 * (TOP/STEP) ) {
			trace_printk("<%d>check failed...\n", cpu);
			break;
		}
		cnt += len;
		copy_to_user(buf + cnt, line, len);

		trace_printk("online cpu %d\n", cpu);
		out |= dump_per_core_trace(cpu, &cnt, buf, buflen);
	}
#endif
	return 0;
}

static int noop_open(struct inode *inode, struct file *file)
{
	trace_printk("%s\n", __FUNCTION__);
	return 0;
}

static void vm_open(struct vm_area_struct *vma)
{
	trace_printk("%s\n", __FUNCTION__);
}

static void vm_close(struct vm_area_struct *vma)
{
	trace_printk("%s\n", __FUNCTION__);
}

static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct page *page = (struct page *)vmf->vma->vm_private_data;
	unsigned long idx = vmf->address;

	idx -= vmf->vma->vm_start;
	idx >>= PAGE_SHIFT;

	trace_printk("%s [%lu][%lu]\n", __FUNCTION__, vmf->address, idx);

	vmf->page = &page[idx];

	return 0;
}

static struct vm_operations_struct vm_ops =
{
	.close	= vm_close,
	.fault	= vm_fault,
	.open	= vm_open,
};

struct polled_io_entry {
	int status;
	int len;
	//Add msg_head or whateva for udp.
	char buffer[0];
};

static inline int poll_ring(struct priv_data *priv)
{
	int cnt = 0;
	struct polled_io_entry *entry = priv->loc;

	while (entry->status) {
		void *next = (entry->status == POLL_RING_STATUS_KERNEL) ?
				(void *)((unsigned long)entry +
				sizeof(struct polled_io_entry) + entry->len):
				page_address(priv->page);

		trace_printk("status %d len %d next %p\n", entry->status, entry->len, next);
		entry->status = POLL_RING_STATUS_USER;
		++cnt;
		entry = next;
	}
	priv->loc = entry;
	return cnt;
}

static int poll_thread(void *data)
{
	struct priv_data *priv = data;
	int pkts = 0;

	while (!kthread_should_stop()) {
		pkts = poll_ring(priv);
		usleep_range(16, 128);
#if 0
	set_current_state(TASK_INTERRUPTIBLE);
	if (!kthread_should_stop())
		schedule();
	__set_current_state(TASK_RUNNING);
#endif
	//yield
	}
	return 0;
}

static int poller_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct priv_data *priv;

	trace_printk("%s: (%s)[%lu:%lu][%d]\n", __FUNCTION__, (filp == vma->vm_file) ? "yes" : "no",
			vma->vm_start, vma->vm_end,
			get_order(vma->vm_end - vma->vm_start));

	/* Disaalow double mmap...; check for vma->priv*/

	priv = kmem_cache_alloc(priv_cache, GFP_KERNEL);
	if (unlikely(!priv))
		return VM_FAULT_OOM;

	priv->page = alloc_pages(GFP_KERNEL|__GFP_COMP, get_order(vma->vm_end - vma->vm_start));
	if (unlikely(!priv->page)) {
		pr_err("Leaking memory....\n");
		return VM_FAULT_OOM;
	}
	priv->loc = page_address(priv->page);
	priv->task  = kthread_run( poll_thread, priv, "poll_thread");
	priv->vma = vma;
	vma->vm_private_data = priv;
	filp->private_data = priv;
	list_add(&priv->list, &priv_list);


	vma->vm_ops = &vm_ops;

	return 0;
}

static const struct file_operations poller_fops = {
	.owner	= THIS_MODULE,
	.open	= noop_open,
	.read	= poller_read,
	.write	= poller_write,
	.mmap 	= poller_mmap,
	.llseek	= noop_llseek,
};

static __init int poller_init(void)
{
	proc_dir = proc_mkdir_mode(POLLER_DIR_NAME, 00555, NULL);

	priv_cache = kmem_cache_create("priv_cache",
                                                sizeof(struct priv_data), 0, 0, NULL);

	if (!proc_create(procname, 0666, proc_dir, &poller_fops))
		goto err;

	return 0;
err:
	return -1;
}

static __exit void poller_exit(void)
{
	struct priv_data *priv, *next;

	remove_proc_subtree(POLLER_DIR_NAME, NULL);

	list_for_each_entry_safe(priv, next, &priv_list, list) {
		list_del(&priv->list);
		trace_printk("%p\n", priv->task);
		if (priv->task)
			kthread_stop(priv->task);
		kmem_cache_free(priv_cache, priv);
	}
	kmem_cache_destroy(priv_cache);
}

module_init(poller_init);
module_exit(poller_exit);
