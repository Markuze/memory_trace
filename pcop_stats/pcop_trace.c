#include "linux/kernel.h"
#include "linux/module.h"
#include "linux/debugfs.h"
#include "linux/slab.h"
#include "linux/mm.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Debugging PCOP configs");
MODULE_AUTHOR("Markuze Alex");

struct dentry *dir, *stats, *update;

static int ta_dump_show(struct seq_file *m, void *v)
{
	get_mag_stats();
	return 0;
}

static int ta_dump_open(struct inode *inode, struct file *filp)
{
	return single_open(filp, ta_dump_show, NULL);
}

static const struct file_operations ta_dump_fops = {
	.owner		= THIS_MODULE,
	.open		= ta_dump_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static ssize_t write_op(struct file *file,
                 	const char __user *user_buf, size_t size, loff_t *ppos)
{
	char *kbuf;
	int values[6] = {0};

	if (size <= 1 || size >= PAGE_SIZE)
		return -EINVAL;
	kbuf = memdup_user_nul(user_buf, size);
	if (IS_ERR(kbuf))
		return PTR_ERR(kbuf);

	get_options(kbuf, ARRAY_SIZE(values), values);
	kfree(kbuf);
	trace_printk("%d values %d %d %d %d %d\n", values[0], values[1], values[2], values[3], values[4], values[5]);

	set_pcop_limits(&values[1]);
	return size;
}

static const struct file_operations set_fops = {
	.owner		= THIS_MODULE,
	.llseek		= default_llseek,
	.write		= write_op,
};

int pcop_debug_init(void)
{
	dir = debugfs_create_dir("pcop_debug", NULL);
	if (unlikely(!dir))
		return PTR_ERR(dir);

	stats = debugfs_create_file("get_stats", 0666, dir, NULL /* no specific val for this file */,
					&ta_dump_fops);
	update = debugfs_create_file("set_stats", 0666, dir, NULL /* no specific val for this file */,
					&set_fops);
	return 0;
}

void pcop_debug_exit(void)
{
	debugfs_remove_recursive(dir);
}
//arch/x86/mm/pkeys.c great example
module_init(pcop_debug_init);
module_exit(pcop_debug_exit);
