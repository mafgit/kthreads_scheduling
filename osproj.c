#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/signal.h>
#include <linux/sched/signal.h>
#include <linux/time.h>

#define NUM_THREADS 3

static struct task_struct *my_threads[NUM_THREADS];

struct thread_data
{
    int id;
    int remaining_iterations;
    int total_iterations;
    int arrival_time;
    struct list_head list;
    // struct list_head is used in the linux kernel for implementing linked lists
};

static LIST_HEAD(ready_list);
// LIST_HEAD is a macro provided by linux kernel.
// an empty linked list called ready_list is created
// ready_list is being used as a ready queue

int thread_func(void *data)
{
    struct thread_data *info = (struct thread_data *)data;
    printk(KERN_INFO "OSPROJ: Thread %d has arrived\n", info->id);
    // printk is a C function from the Linux kernel interface that prints messages to the kernel log
    // KERN_INFO is a macro that tells that the level of logging this message is "an informational message"
    // there are other levels too such as KERN_ERR, KERN_WARNING, etc

    for (int i = 0; i < info->total_iterations; ++i)
    {
        printk(KERN_INFO "OSPROJ: Thread %d running\n", info->id);
        msleep(1000);
    }

    printk(KERN_INFO "OSPROJ: Thread %d completed\n", info->id);
    return 0;
}

static struct thread_data threads_data[NUM_THREADS] = {
    {0, 5, 5, 0, LIST_HEAD_INIT(threads_data[0].list)},
    {1, 3, 3, 0, LIST_HEAD_INIT(threads_data[1].list)},
    {2, 7, 7, 0, LIST_HEAD_INIT(threads_data[2].list)}
    // LIST_HEAD_INIT: a macro provided by the linux kernel that initializes a list_head structure.
};

static void check_preemption(void)
{
    int shortest_iterations = INT_MAX;
    struct thread_data *entry, *shortest_thread = NULL;

    list_for_each_entry(entry, &ready_list, list)
    {
        if (entry->remaining_iterations < shortest_iterations)
        {
            shortest_iterations = entry->remaining_iterations;
            shortest_thread = entry;
        }
    }
    // list_for_each_entry is a macro provided by the Linux kernel for iterating over each element in a linked list. It takes three parameters:
    // ptr to current element in iteration, ptr to head, list

    if (shortest_thread && shortest_thread->id != current->pid)
    {
        printk(KERN_INFO "OSPROJ: Thread %d preempted Thread %d\n", shortest_thread->id, current->pid);
        send_sig(SIGSTOP, my_threads[current->pid], 0);        // send signal to stop the current thread
        send_sig(SIGCONT, my_threads[shortest_thread->id], 0); // send signal to resume the shortest thread
    }
}

static int preemptive_scheduler(void *data)
{
    while (!kthread_should_stop())
    {
        check_preemption();
        msleep(1000);
    }
    return 0;
}

static int __init init_thread(void)
{
    struct task_struct *scheduler_thread = kthread_run(preemptive_scheduler, NULL, "preemptive_scheduler");
    // kthread_run() = kthread_create() + wake_up_process()
    if (IS_ERR(scheduler_thread))
    {
        printk(KERN_ERR "OSPROJ: Failed to create preemptive scheduler thread\n");
        return PTR_ERR(scheduler_thread);
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        list_add_tail(&threads_data[i].list, &ready_list);
        // list_add_tail() is a macro provided by the Linux kernel for adding a new element to the end of a linked list. It takes two arguments: a pointer to the new element to be added and a pointer to the list head.
        // TODO: ask for thread_data in input
        // TODO: add arrival time functionality
        my_threads[i] = kthread_create(thread_func, &threads_data[i], "my_thread_%d", threads_data[i].id);
        if (IS_ERR(my_threads[i]))
        {
            printk(KERN_ERR "OSPROJ: Failed to create thread %d\n", i);
            return PTR_ERR(my_threads[i]);
        }
        wake_up_process(my_threads[i]);
        // wake up the process and move it to a set of runnable processes
    }

    return 0;
}

static void __exit cleanup_thread(void)
{
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        if (my_threads[i])
        {
            kthread_stop(my_threads[i]);
        }
    }
}

module_init(init_thread);
module_exit(cleanup_thread);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Riyyan & Abdullah");
MODULE_DESCRIPTION("Preemptive SJF/SRTF scheduling of kernel threads");