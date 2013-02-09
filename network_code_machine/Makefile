module_name := hello_kernel
obj-m := $(module_name).o

all:
	make -C /usr/src/linux-3.6.11-rt M=$(PWD) modules
clean:
	make -C /usr/src/linux-3.6.11-rt M=$(PWD) clean
install:
	insmod $(module_name).ko
uninstall:
	rmmod $(module_name)
	
	