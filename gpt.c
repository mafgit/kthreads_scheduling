#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *my_thread;
static int thread_should_run = 1;
static DEFINE_MUTEX(thread_mutex);

static int my_thread_func(void *data) {
    while (!kthread_should_stop()) {
        mutex_lock(&thread_mutex);
        if (!thread_should_run) {
            mutex_unlock(&thread_mutex);
            msleep(100); // Sleep for a short time to avoid spinning
            continue;
        }
        mutex_unlock(&thread_mutex);

        // Do some work here
        printk(KERN_INFO "Thread running\n");
        msleep(1000); // Sleep for 1 second
    }
    return 0;
}

static int __init init_thread(void) {
    my_thread = kthread_create(my_thread_func, NULL, "my_thread");
    if (IS_ERR(my_thread)) {
        printk(KERN_ERR "Failed to create thread\n");
        return PTR_ERR(my_thread);
    }

    wake_up_process(my_thread);
    return 0;
}

static void __exit cleanup_thread(void) {
    mutex_lock(&thread_mutex);
    thread_should_run = 0;
    mutex_unlock(&thread_mutex);

    wake_up_interruptible(&my_thread->wait);
    kthread_stop(my_thread);
}

module_init(init_thread);
module_exit(cleanup_thread);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple kernel thread example with pause and resume using mutexes");
