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

static ncm_interpreter_t ncm_interp;
static netcode_instr_t instructions[PROGRAM_LEN];
static netcode_program_t program;
static interp_params_t	interp_params;

static int chrdev_major;

static ssize_t varspace_chrdev_read(struct file* filep, char* buff, size_t len, loff_t* off) {
	u8 kbuf[MAX_VAR_SIZE_BYTES];
	u32 klen;

	get_variable_data(&ncm_interp.variable_space, 1, kbuf, &klen);

	if(copy_to_user(buff, kbuf, min(len, klen)) != 0) {
		debug_print("Failed reading from ncm_varspace\n");
		return 0;
	}

	//kbuf[klen] = '\0';
	//debug_print("Read %s with length %d from variable %d\n", kbuf, min(len, klen), *off);

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
	debug_print("Wrote %s to variable %d\n", &kbuf[1], kbuf[0]);

	return len;
}

static struct file_operations fops = {
	.read	= varspace_chrdev_read,
	.write	= varspace_chrdev_write,
};


int init_module() {
	struct ncm_net_params params = {
			.net_device_name 	= 	{ "enp0s3" }, // "wlan0" // "eth0"  //sometimes enp0s3
			.mac_address		=	{ 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B }, // mac address
			.channel_mac		=	{
					{ 0x30, 0x85, 0xA9, 0x8E, 0x87, 0x95 } // mac address
					// { 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B } // original vm
					// { 0x08, 0x00, 0x27, 0xCf, 0x5c, 0xD7 } // clone vm
					// { 0x0a, 0x00, 0x27, 0x00, 0x00, 0x00 } // virtual interaface laptop addres
			}
	};
	interp_params.network = params;


	debug_print( KERN_ALERT "NCM started at time %llu\n", now_us() );

	instructions[0].type = NOP;
	instructions[1].type = FUTURE;
	instructions[1].args[0] = 1000000;
	instructions[1].args[1] = 6;

	instructions[2].type = FUTURE;
	instructions[2].args[0] = 10000000;
	instructions[2].args[1] = 6;

	instructions[3].type = WAIT;
	instructions[3].args[0] = 100000000;
	instructions[3].args[1] = 5;

	instructions[4].type = NOP;

	instructions[5].type = GOTO;
	instructions[5].args[0] = 9;

	instructions[6].type = NOP;
	instructions[7].type = NOP;
	instructions[8].type = HALT;

	instructions[9].type = CLEAR_COUNTER;
	instructions[9].args[0] = 1;

	instructions[10].type = IF;
	instructions[10].args[0] = TEST_COUNT;
	instructions[10].args[1] = 13;
	instructions[10].args[2] = 1;
	instructions[10].args[3] = 10;

	instructions[11].type = ADD_TO_COUNTER;
	instructions[11].args[0] = 1;
	instructions[11].args[1] = 1;

	instructions[12].type = GOTO;
	instructions[12].args[0] = 10;

	instructions[13].type = IF;
	instructions[13].args[0] = GREATER_VAR_VAR;
	instructions[13].args[1] = 15;
	instructions[13].args[2] = 0;
	instructions[13].args[3] = 1;

	instructions[14].type = GOTO;
	instructions[14].args[0] = 0;

	instructions[15].type = END_OF_PROGRAM;

	program.instructions = instructions;
	program.length = PROGRAM_LEN;

	chrdev_major = register_chrdev(0, VARSPACE_CHRDEV_NAME, &fops);
	debug_print("Registered char device with major number %d\n", chrdev_major);

	start_interpreter(&ncm_interp, &program, &interp_params);

	return 0;
}

void cleanup_module(void) {
	stop_interpreter(&ncm_interp);

	unregister_chrdev(chrdev_major, VARSPACE_CHRDEV_NAME);

	debug_print( KERN_ALERT "NCM ended at time %llu\n", now_us() );
}
