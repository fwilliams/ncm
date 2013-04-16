#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "netcode_helper.h"
#include "interpreter.h"
#include "guards.h"
#include "hax.h"
#include "sysfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Francis Williams, 2013");
MODULE_DESCRIPTION("Network Code interpreter module");

static ncm_interpreter_t ncm_interp;
static ncm_program_t program;
static ncm_interp_params_t	interp_params;
static ncm_sysfs_t sysfs;

static int chrdev_major;

static ssize_t varspace_chrdev_read(struct file* filep, char __user* buff, size_t len, loff_t* off) {
	u8 kbuf[MAX_VAR_SIZE_BYTES+1];
	u32 klen;

	get_variable_data(&ncm_interp.variable_space, 0, kbuf, &klen);

	kbuf[klen] = '\0';

	if(copy_to_user(buff, kbuf, min(len, klen)) != 0) {
		debug_print("Failed reading from ncm_varspace");
		return -EFAULT;
	} else {
		debug_print("Read %s with length %d from variable %d", kbuf, min(len, klen), (int) *off);
		return min(len, klen);
	}
}

static ssize_t varspace_chrdev_write(struct file* filp, const char __user* buff, size_t len, loff_t* off) {
	u8 kbuf[len+1];

	copy_from_user(kbuf, buff, len);

	set_variable_data(&ncm_interp.variable_space, 0, &kbuf[0], len);

	kbuf[len] = '\0';
	debug_print("Wrote %s to variable 0", &kbuf[0]);

	return len;
}

static struct file_operations fops = {
	.read	= varspace_chrdev_read,
	.write	= varspace_chrdev_write,
};


int init_module() {
	int sysfs_ret;

#ifdef VM1
	make_program(&program, &interp_params.network, TYPE_ARCH1);
#endif
#ifdef VM2
	make_program(&program, &interp_params.network, TYPE_ARCH2);
#endif


	chrdev_major = register_chrdev(0, VARSPACE_CHRDEV_NAME, &fops);
	debug_print("Registered char device with major number %d", chrdev_major);

	init_interpreter(&ncm_interp);

	sysfs_ret = nc_init_sysfs(&sysfs, &ncm_interp, &program, &interp_params);

#ifndef VM
	start_interpreter(&ncm_interp, &program, &interp_params);
#endif

	return sysfs_ret;
}

void cleanup_module(void) {

	ncm_sysfs_cleanup(&sysfs);

	if(is_running(&ncm_interp))
		stop_interpreter(&ncm_interp);

	unregister_chrdev(chrdev_major, VARSPACE_CHRDEV_NAME);

	debug_print("NCM ended at time %llu", now_us() );
}



