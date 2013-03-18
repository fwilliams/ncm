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

static int major;

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
	return 0;
}

static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off) {
	return len;
}

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
};

int init_module() {
	printk( KERN_ALERT "NCM started at time %llu\n", now_us() );

	major = register_chrdev(0, "ncm_varspace", &fops);
	printk( KERN_ALERT "char dev major number %d\n", major );

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

	start_interpreter(&ncm_interp, program, PROGRAM_LEN);

	return 0;
}

void cleanup_module(void) {
	unregister_chrdev(major, "ncm_varspace");

	stop_interpreter(&ncm_interp);

	printk( KERN_ALERT "NCM ended at time %llu\n", now_us() );
}
