# backwards compatibility
ifneq ($(KERNELRELEASE),)
include Kbuild
else
# /backwards compatibility

MODNAME=nc_net
KDIR ?= /usr/src/linux-3.6.11-rt

all: build
build:
	make -C $(KDIR) M=$$PWD modules
install:
	insmod $(MODNAME).ko
uninstall:
	rmmod $(MODNAME)
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
