# Preemptive Scheduling of Kernel Threads

### Setup

- `sudo apt update`
- `sudo apt-get install build-essential`
- `sudo apt-get install linux-headers-$(uname -r)` (install linux headers)
- `sudo apt-get install gcc-12`
- `apt install dwarves` and ```cp /sys/kernel/btf/vmlinux /usr/lib/modules/`uname -r`/build/``` to remove warning of skipping BTF generation ...
- `make` to generate files
- `make clean` to remove generated files

### Run

- `sudo insmod osproj.ko` to start thread
- `sudo rmmod osproj` to kill thread
- `ps -ef`
- `lsmod` to show currently loaded kernel modules
- `sudo dmesg`