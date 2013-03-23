#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "netcode_helper.h"
#include "interpreter.h"
#include "guards.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Francis Williams, 2013");
MODULE_DESCRIPTION("Network Code interpreter module");

#define PROGRAM_LEN 16

struct interpreter ncm_interp;
struct netcode_instr program[PROGRAM_LEN];

static int chrdev_major;

static ssize_t varspace_chrdev_read(struct file* filep, char* buff, size_t len, loff_t* off) {
	u8 kbuf[MAX_VAR_SIZE_BYTES];
	u32 klen;

	get_variable_data(&ncm_interp.variable_space, 1, kbuf, &klen);

	if(copy_to_user(buff, kbuf, min(len, klen)) != 0) {
		printk("Failed reading from ncm_varspace\n");
		return 0;
	}

	//kbuf[klen] = '\0';
	//printk("Read %s with length %d from variable %d\n", kbuf, min(len, klen), *off);

	return min(len, klen);
}

static ssize_t varspace_chrdev_write(struct file* filp, const char __user* buff, size_t len, loff_t* off) {
	u8 kbuf[len+1];
	u32 var_id;

	copy_from_user(kbuf, buff, len);

	if(kbuf[0] == 49) {
		var_id = 0;
	} else {
		var_id = 1;
	}

	set_variable_data(&ncm_interp.variable_space, var_id, &kbuf[1], len-1);

	kbuf[len] = '\0';
	printk("Wrote %s to variable %d\n", &kbuf[1], kbuf[0]);

	return len;
}

static struct file_operations fops = {
	.read	= varspace_chrdev_read,
	.write	= varspace_chrdev_write,
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

	program[9].type = CLEAR_COUNTER;
	program[9].args[0] = 1;

	program[10].type = IF;
	program[10].args[0] = TEST_COUNT;
	program[10].args[1] = 13;
	program[10].args[2] = 1;
	program[10].args[3] = 10;

	program[11].type = ADD_TO_COUNTER;
	program[11].args[0] = 1;
	program[11].args[1] = 1;

	program[12].type = GOTO;
	program[12].args[0] = 10;

	program[13].type = IF;
	program[13].args[0] = GREATER_VAR_VAR;
	program[13].args[1] = 15;
	program[13].args[2] = 0;
	program[13].args[3] = 1;

	program[14].type = GOTO;
	program[14].args[0] = 13;

	program[15].type = END_OF_PROGRAM;

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
