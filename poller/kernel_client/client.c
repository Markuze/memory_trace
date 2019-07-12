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
MODULE_DESCRIPTION("Deferred I/O client");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

#define POLLER_DIR_NAME "io_client"
#define procname "udp_client"

static struct proc_dir_entry *proc_dir;
static struct task_struct *client_task;

static ssize_t client_write(struct file *file, const char __user *buf,
                              size_t len, loff_t *ppos)
{
	trace_printk("%s\n", __FUNCTION__);
	wake_up_process(client_task);
	return len;
}

static ssize_t client_read(struct file *file, char __user *buf,
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
	/*TODO: When process dies memory and buffer are leaking...*/
	trace_printk("%s\n", __FUNCTION__);
}

static inline void udp_client(void)
{
#define PORT	8080
#define MAXLINE 1024
#define SERVER_ADDR (10<<24|1<<16|4<<8|38) /*10.1.4.38*/
	int rc, i = 0;
	char buffer[MAXLINE];
	struct socket *tx = NULL;
	struct sockaddr_in srv_addr = {0};
	struct msghdr msg = { 0 };
	struct kvec kvec;

        srv_addr.sin_family             = AF_INET;
        srv_addr.sin_addr.s_addr        = htonl(SERVER_ADDR);
        srv_addr.sin_port               = htons(port);

	msg.msg_name = &srv_addr;
	msg.msg_len = sizeof(struct sockaddr);

	kvec.iov_len = PAGE_SIZE;
	if (! (kvec.iov_base = page_address(alloc_page(GFP_KERNEL))))
		goto err;

	if ((rc = sock_create_kern(&init_net, PF_INET, SOCK_DGRAM, IPPROTO_UDP, &tx))) {
		goto err;
        }

	snprintf(kvec.iov_base, 64, "HELLO!");
	//for (i = 0; i < (1<<25); i++) {
		kernel_sendmsg(tx, msg, 1, 42),
	//}
	trace_printk("Hello message sent.(%d)\n", i);
	return;
err:
	trace_printk("ERROR %d\n", rc);
	return;
}

static int poll_thread(void *data)
{
	while (!kthread_should_stop()) {

		udp_client();

		set_current_state(TASK_INTERRUPTIBLE);
		if (!kthread_should_stop())
			schedule();
		__set_current_state(TASK_RUNNING);
	}
	return 0;
}

static const struct file_operations client_fops = {
	.owner	= THIS_MODULE,
	.open	= noop_open,
	.read	= client_read,
	.write	= client_write,
	.llseek	= noop_llseek,
};

static __init int client_init(void)
{
	proc_dir = proc_mkdir_mode(POLLER_DIR_NAME, 00555, NULL);

	client_task  = kthread_run(poll_thread, priv, "poll_thread");

	if (!proc_create(procname, 0666, proc_dir, &client_fops))
		goto err;

	return 0;
err:
	return -1;
}

static __exit void client_exit(void)
{
	struct priv_data *priv, *next;

	remove_proc_subtree(POLLER_DIR_NAME, NULL);
	kthread_stop(priv->task);
}

module_init(client_init);
module_exit(client_exit);
