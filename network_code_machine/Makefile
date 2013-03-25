kernel_dir 			:= /usr/src/linux-3.6.11-rt

module_name 		:= netcode

obj-m				:= $(module_name).o
$(module_name)-y	:= netcode_helper.o future_queue.o variable_space.o counter.o guards.o nc_net.o interpreter.o netcode_module.o
ccflags-y 			:= -D__DEBUG__ -Wframe-larger-than=9000

all:
	make -C $(kernel_dir) M=$(PWD) modules
clean:
	make -C $(kernel_dir) M=$(PWD) clean
install:
	insmod $(module_name).ko
uninstall:
	rmmod $(module_name)
update:
	make uninstall clean all install
delete:
	make clean uninstall