#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/signal.h>
#include <linux/sched/signal.h>

#define NUM_THREADS 3

static struct task_struct *my_threads[NUM_THREADS];

struct thread_data {
    int id;
    int remaining_iterations;
    int total_iterations;
    int (*fn)(void *data);
    struct list_head list;
    // struct list_head is a fundamental structure used in the Linux kernel for implementing linked lists
};

static LIST_HEAD(ready_list);
// LIST_HEAD is a macro provided by linux kernel.
// a linked list called ready_list is created

int t1func(void *data) {
    int iterations = *((int *)data);
    for (int i = 0; i < iterations; ++i) {
        printk(KERN_INFO "OSPROJ: Thread 1 running\n");
        msleep(1000);
    }
    printk(KERN_INFO "OSPROJ: Thread 1 completed\n");
    return 0;
}

int t2func(void *data) {
    int iterations = *((int *)data);
    for (int i = 0; i < iterations; ++i) {
        printk(KERN_INFO "OSPROJ: Thread 2 running\n");
        msleep(1000);
    }
    printk(KERN_INFO "OSPROJ: Thread 2 completed\n");
    return 0;
}

int t3func(void *data) {
    int iterations = *((int *)data);
    for (int i = 0; i < iterations; ++i) {
        printk(KERN_INFO "OSPROJ: Thread 3 running\n");
        msleep(1000);
    }
    printk(KERN_INFO "OSPROJ: Thread 3 completed\n");
    return 0;
}

static struct thread_data threads_data[NUM_THREADS] = {
    {0, 5, 5, t1func, LIST_HEAD_INIT(threads_data[0].list)},
    {1, 3, 3, t2func, LIST_HEAD_INIT(threads_data[1].list)},
    {2, 7, 7, t3func, LIST_HEAD_INIT(threads_data[2].list)}
    // LIST_HEAD_INIT: a macro provided by the Linux kernel that initializes a list_head structure.
};

void send_signal(int sig, struct task_struct *task) {
    send_sig(sig, task, 0); // Send the signal to the specified task
}

static void check_preemption(void) {
    int shortest_iterations = INT_MAX;
    struct thread_data *entry, *shortest_thread = NULL;

    list_for_each_entry(entry, &ready_list, list) {
        if (entry->remaining_iterations < shortest_iterations) {
            shortest_iterations = entry->remaining_iterations;
            shortest_thread = entry;
        }
    }
    // This is a macro provided by the Linux kernel for iterating over each element in a linked list. It takes three parameters:
    // ptr to current element in iteration, ptr to head, list
    if (shortest_thread && shortest_thread->id != current->pid) {
        printk(KERN_INFO "OSPROJ: Thread %d preempted Thread %d\n", shortest_thread->id, current->pid);
        send_signal(SIGSTOP, my_threads[current->pid]); // Stop the current thread
        send_signal(SIGCONT, my_threads[shortest_thread->id]); // Resume the shortest thread
    }
}

static int preemptive_scheduler(void *data) {
    while (!kthread_should_stop()) {
        check_preemption();
        msleep(1000);
    }
    return 0;
}

static int __init init_thread(void) {
    struct task_struct *scheduler_thread = kthread_run(preemptive_scheduler, NULL, "preemptive_scheduler");
    if (IS_ERR(scheduler_thread)) {
        printk(KERN_ERR "OSPROJ: Failed to create preemptive scheduler thread\n");
        return PTR_ERR(scheduler_thread);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        list_add_tail(&threads_data[i].list, &ready_list);

        my_threads[i] = kthread_create(threads_data[i].fn, &threads_data[i].total_iterations, "my_thread_%d", threads_data[i].id);
        if (IS_ERR(my_threads[i])) {
            printk(KERN_ERR "OSPROJ: Failed to create thread %d\n", i);
            return PTR_ERR(my_threads[i]);
        }
        wake_up_process(my_threads[i]);
    }

    return 0;
}

static void __exit cleanup_thread(void) {
    for (int i = 0; i < NUM_THREADS; ++i) {
        if (my_threads[i]) {
            kthread_stop(my_threads[i]);
        }
    }
}

module_init(init_thread);
module_exit(cleanup_thread);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Riyyan & Abdullah");
MODULE_DESCRIPTION("Preemptive scheduling of kernel threads using SJF method");

// TODO: add and edit comments for better understanding. understand static.