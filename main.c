#include<linux/module.h>     // included for all kernel modules
#include<linux/kthread.h>    // for thread functions
#include<linux/init.h>       // for __init and __exit
#include<linux/delay.h>      // for msleep(milliseconds)
#include<linux/sched.h>      // for struct tesk_struct

// kthread_create(thread_function, &tid, (const char*) kthread_name);
// wake_up_process(kthread) to run thread
// kthread_stop(&kthread)
// sudo insmod abcde.ko to start thread
// sudo rmmod abcde.ko to kill thread

// global array of kernel threads
static struct task_struct kthreads_arr[5];

int thread_function(void* void_tid_ptr) {
  unsigned int i = 0;
  int tid = *(int*) void_tid_ptr;
  
  while (!kthread_should_stop()) {
    printk(KERN_INFO "Thread %d Still running...! %d secs\n", tid, i);
    i++;
    if (i == 45)
      break;

    msleep(1000); // sleeping for a second 45 times
  }
  printk(KERN_INFO "Thread %d has stopped\n", tid);
  return 0;
}

// initialize one thread at a time.
int initialize_thread(struct task_struct * kthread, int tid) {
  char kthread_name[25];
  sprintf(kthread_name, "kthread_%d", tid);
  kthread = kthread_create(thread_function, & tid, (const char * ) kthread_name);
  if (kthread != NULL) {
    wake_up_process(kthread);
    printk(KERN_INFO "%s is running\n", kthread_name);
  } else {
    printk(KERN_INFO "kthread %s could not be created\n", kthread_name);
    return -1;
  }
  return 0;
}


// initializes all threads that are in the global array
static int __init module_init_fn(void) {
  int i = 0;
  printk(KERN_INFO "Initializing thread module\n");
  
  for (i = 0; i < 5; i++) {
    if (initialize_thread(&kthreads_arr[i], i) == -1) {
      return -1;
    }
  }

  printk(KERN_INFO "All threads are running\n");
  return 0;
}

static void __exit module_exit_fn(void) {
  int i = 0;
  int ret = 0;
  printk(KERN_INFO "exiting thread module\n");

  for (i = 0; i < 5; i++) {
    ret = kthread_stop( & kthreads_arr[i]);
    if (!ret) {
      printk("can't stop thread %d", i);
    }
  }

  printk(KERN_INFO "stopped all of the threads\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("RIYYAN SIDDIQUI & M. ABDULLAH FAROOQUI");
MODULE_DESCRIPTION("OS PROJECT - PREEMPTIVE SCHEDULING OF KERNEL THREADS");

module_init(module_init_fn);
module_exit(module_exit_fn);