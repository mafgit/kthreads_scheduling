#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/sched.h> // for wake_up_process
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/signal.h>
#define N 3

// sudo apt update
// sudo apt-get install build-essential
// sudo apt-get install linux-headers-$(uname -r) (install linux headers)
// sudo apt-get install gcc-12
// (warning! don't run) sudo apt remove --autoremove linux-headers-* (remove all linux headers)

// kthread_create(thread_function, &tid, (const char*) kthread_name);
// wake_up_process(kthread) to run thread
// kthread_stop(&kthread)
// sudo insmod main.ko to start thread
// sudo rmmod main to kill thread
// ps -ef
// lsmod to show currently loaded kernel modules

struct task_struct threads[N];
static int burst_times[N] = {2000, 3000, 1000}; // in milliseconds
static int arrival_times[N] = {0, 1000, 2000};  // in milliseconds
struct task_struct *running = NULL;
int running_i = 0;

typedef int (*fn)(void); // for functions' array
int thread1_fn(void *arg) {}
int thread2_fn(void *arg) {}
int thread3_fn(void *arg) {}
fn thread_fns[N] = {&thread1_fn, &thread2_fn, &thread3_fn};

static int __init kthreads_init(void)
{
    for (int i = 0; i < N; i++)
    {
        if (burst_times[i] <= 0)
            continue;

        int *thread_id = kmalloc(sizeof(int), GFP_KERNEL);
        if (!thread_id)
        {
            printk(KERN_ERR "Failed to allocate memory for thread ID\n");
            return -ENOMEM;
        }
        threads[i] = kthread_create(thread_fns[i], thread_id, "my_kthread_%d", i);
        if (IS_ERR(my_threads[i]))
        {
            printk(KERN_ERR "Failed to create kernel thread %d\n", i);
            kfree(thread_id);
            return PTR_ERR(my_threads[i]);
        }
    }

    for (int t = 0; t < 20; t++)
    {
        for (int i = 0; i < N; i++)
        {
            if (running == NULL)
            {
                running = threads[i];
                running_i = i;
                wake_up_process(my_threads[i]);
            }

            msleep(arrival_times[i]);
            if (running_i != i)
            {
                if (burst_times[running_i] > burst_times[i])
                {
                    kthread_stop(running);
                    wake_up_process(my_threads[i]);
                    running_i = i;
                    running = my_threads[i];
                }
            }
        }
    }
    return 0;
}

static void __exit kthreads_exit(void)
{
    printk(KERN_INFO "Exiting my kernel module\n");

    // Stop and release kernel threads
    for (int i = 0; i < N; ++i)
    {
        if (threads[i])
        {
            kthread_stop(threads[i]);
            threads[i] = NULL;
        }
    }
}

module_init(kthreads_init);
module_exit(kthreads_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("RIYYAN AND ABDULLAH");
MODULE_DESCRIPTION("Preemptive SJF scheduling of kernel threads");