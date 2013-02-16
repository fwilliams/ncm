kernel_dir := /usr/src/linux-3.6.11-rt

module_name := netcode

obj-m := $(module_name).o
$(module_name)-y := interpreter.o netcode_module.o

all:
	make -C $(kernel_dir) M=$(PWD) modules
clean:
	make -C $(kernel_dir) M=$(PWD) clean
install:
	insmod $(module_name).ko
uninstall:
	rmmod $(module_name)
	
	