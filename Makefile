obj-m+=main.o
 
all:
	make -C /lib/modules/6.5.0-18-generic/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/6.5.0-18-generic/build/ M=$(PWD) clean

