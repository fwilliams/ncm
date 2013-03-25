# backwards compatibility
ifneq ($(KERNELRELEASE),)
include Kbuild
else
# /backwards compatibility

MODNAME=nc_net
# for laptop
# KDIR ?= /usr/src/linux-headers-3.5.0-23-generic 
# for vm
KDIR ?= /usr/src/linux-3.6.11-rt

all: build
build:
	make -C $(KDIR) M=$$PWD modules
install: 
	sync
	insmod $(MODNAME).ko
	sync
uninstall:
	sync
	rmmod $(MODNAME)
	sync
test:build
	sync
	make install
	sync
	make uninstall
testdmesg:build
	sync
	make install
	sync
	make uninstall
	dmesg
clean:
	make -C $(KDIR) M=$$PWD clean


# this is suppsed to work:
# install:
# 	make -C $(KDIR) M=$$PWD modules_install

# this might be useful one day
build_against_running_kernel:
	make -C /lib/modules/`uname -r`/build M=$$PWD modules
	make -C /lib/modules/`uname -r`/build M=$$PWD modules_install

# backwards compatibility
endif
# /backwards compatibility
