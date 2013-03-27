#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>

#include "netcode_helper.h"
#include "interpreter.h"
#include "guards.h"
#include "hax.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Francis Williams, 2013");
MODULE_DESCRIPTION("Network Code interpreter module");

#define PROGRAM_LEN 16

static ncm_interpreter_t ncm_interp;
static ncm_instr_t instructions[PROGRAM_LEN];
static ncm_program_t program;
static ncm_interp_params_t	interp_params;

static int chrdev_major;

static ssize_t varspace_chrdev_read(struct file* filep, char* buff, size_t len, loff_t* off) {
	u8 kbuf[MAX_VAR_SIZE_BYTES];
	u32 klen;

	get_variable_data(&ncm_interp.variable_space, 1, kbuf, &klen);
	kbuf[klen] = '\0';

	if(copy_to_user(buff, kbuf, min(len, klen)) != 0) {
		debug_print("Failed reading from ncm_varspace");
		return 0;
	}

	debug_print("Read %s with length %d from variable %d", kbuf, min(len, klen), (int) *off);

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
	debug_print("Wrote %s to variable %d", &kbuf[1], kbuf[0]);

	return len;
}

static struct file_operations fops = {
	.read	= varspace_chrdev_read,
	.write	= varspace_chrdev_write,
};


int init_module() {

#ifdef VM1
	make_program(instructions, &interp_params.network, TYPE_ARCH1);
#endif
#ifdef VM2
	make_program(instructions, &interp_params.network, TYPE_ARCH2);
#endif

	program.instructions = instructions;
	program.length = PROGRAM_LEN;

	chrdev_major = register_chrdev(0, VARSPACE_CHRDEV_NAME, &fops);
	debug_print("Registered char device with major number %d", chrdev_major);


	start_interpreter(&ncm_interp, &program, &interp_params);
	debug_print( KERN_ALERT "NCM started at time %llu", now_us() );

	return 0;
}

void cleanup_module(void) {
	stop_interpreter(&ncm_interp);

	unregister_chrdev(chrdev_major, VARSPACE_CHRDEV_NAME);

	debug_print( KERN_ALERT "NCM ended at time %llu", now_us() );
}
