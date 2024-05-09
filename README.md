# Preemptive Scheduling of Kernel Threads

### Some commands to get started:

`sudo apt update`

`sudo apt-get install build-essential`

`sudo apt-get install linux-headers-$(uname -r)` (install linux headers)

- `sudo apt-get install gcc-12`

- (warning! don't run) `sudo apt remove --autoremove linux-headers-*` (remove all linux headers)

- `sudo insmod osproj.ko` to start thread

- `sudo rmmod osproj` to kill thread

- `ps -ef`

- `lsmod` to show currently loaded kernel modules