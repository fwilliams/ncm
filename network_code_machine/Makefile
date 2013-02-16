kernel_dir := /usr/src/linux-3.6.11-rt

module_name := netcode_module

obj-m := interpreter.o $(module_name).o

all:
	make -C $(kernel_dir) M=$(PWD) modules
clean:
	make -C $(kernel_dir) M=$(PWD) clean
install:
	insmod $(module_name).ko
uninstall:
	rmmod $(module_name)
	
	