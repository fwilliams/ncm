kernel_dir 			:= /usr/src/linux-3.6.11-rt

module_name 		:= netcode$(vm)

varpsace_major_num	:= 250

obj-m				:= $(module_name).o
$(module_name)-y	:= netcode_helper.o future_queue.o variable_space.o counter.o guards.o nc_net.o interpreter.o netcode_module.o
ccflags-y 			:= -D__DEBUG__ -DVM$(vm) -Wframe-larger-than=9000

all:
	make -C $(kernel_dir) M=$(PWD) modules
clean:
	make -C $(kernel_dir) M=$(PWD) clean
install:
	dmesg --clear
	insmod $(module_name).ko
	mknod /dev/ncm_varspace c $(varpsace_major_num) 0
uninstall:
	rmmod $(module_name)
	rm -f /dev/ncm_varspace
update:
	make uninstall clean all install
delete:
	make clean uninstall
watch:
	watch 'dmesg | tail -n 80'
