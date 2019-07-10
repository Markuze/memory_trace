#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/module.h>

#include <linux/uaccess.h>
#include <linux/cpumask.h>

MODULE_AUTHOR("Markuze Alex markuze@cs.technion.ac.il");
MODULE_DESCRIPTION("Deferred I/O poller");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

#define POLLER_DIR_NAME "io_poller"
#define procname "mngmnt"

static struct proc_dir_entry *cbn_dir;

static ssize_t poller_write(struct file *file, const char __user *buf,
                              size_t len, loff_t *ppos)
{
	trace_printk("%s\n", __FUNCTION__);
	return len;
}

static ssize_t poller_read(struct file *file, char __user *buf,
                             size_t buflen, loff_t *ppos)
{
	int cpu, cnt = 0, out = 0;
	if (!buf)
		return -EINVAL;
#if 0
        /*Stats: will always print something,
         * cat seems to call read again if prev call was != 0
         * ppos points to file->f_pos*/
        if ((*ppos)++ & 1)
                return 0;
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
	return out * cnt;
}

int noop_open(struct inode *inode, struct file *file) { return 0; }

static const struct file_operations poller_fops = {
        .owner   = THIS_MODULE,
        .open    = noop_open,
        .read    = poller_read,
        .write   = poller_write,
        .llseek  = noop_llseek,
};

static __init int poller_init(void)
{
	dir = proc_mkdir_mode(POLLER_DIR_NAME, 00555, NULL);
	if (!proc_create(procname, 0666, dir, &poller_fops))
		goto err;

	return 0;
err:
	return -1;
}

static __exit void poller_exit(void)
{
	remove_proc_entry(procname, NULL);
}

module_init(poller_init);
module_exit(poller_exit);
