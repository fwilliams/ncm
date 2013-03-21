#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "netcode_helper.h"
#include "interpreter.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Francis Williams, 2013");
MODULE_DESCRIPTION("Network Code interpreter module");

#define PROGRAM_LEN 10

struct interpreter ncm_interp;
struct netcode_instr program[PROGRAM_LEN];

static int chrdev_major;

static ssize_t varspace_chrdev_write(struct file *filp, const char __user *buff, size_t len, loff_t *off) {
	char kbuf[len+1];
	char obuf[len+1];
	int olen;
	kbuf[len] = '\0';

	copy_from_user(kbuf, buff, len);

	printk("Writing %s to variable %d\n", &kbuf[1], kbuf[0]);
	set_variable_data(&ncm_interp.variable_space, 1, &kbuf[1], len-1);
	get_variable_data(&ncm_interp.variable_space, 1, obuf, &olen);

	return len;
}

static struct file_operations fops = {
	.write = varspace_chrdev_write,
};


int init_module() {
	printk( KERN_ALERT "NCM started at time %llu\n", now_us() );

	program[0].type = NOP;
	program[1].type = FUTURE;
	program[1].args[0] = 1000000;
	program[1].args[1] = 6;

	program[2].type = FUTURE;
	program[2].args[0] = 10000000;
	program[2].args[1] = 6;

	program[3].type = WAIT;
	program[3].args[0] = 100000000;
	program[3].args[1] = 5;

	program[4].type = NOP;

	program[5].type = GOTO;
	program[5].args[0] = 9;

	program[6].type = NOP;
	program[7].type = NOP;
	program[8].type = HALT;

	program[9].type = END_OF_PROGRAM;

	chrdev_major = register_chrdev(0, VARSPACE_CHRDEV_NAME, &fops);

	printk("Registered char device with major number %d\n", chrdev_major);

	start_interpreter(&ncm_interp, program, PROGRAM_LEN);

	return 0;
}

void cleanup_module(void) {
	stop_interpreter(&ncm_interp);

	unregister_chrdev(chrdev_major, VARSPACE_CHRDEV_NAME);

	printk( KERN_ALERT "NCM ended at time %llu\n", now_us() );
}
