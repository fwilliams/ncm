#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>

#include "netcode_helper.h"
#include "interpreter.h"
#include "future_queue.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Francis Williams, 2013");
MODULE_DESCRIPTION("Network Code interpreter module");

#define PROGRAM_LEN 4

struct interpreter_t ncm_interp;
struct instr_t program[PROGRAM_LEN];

int init_module() {
	printk( KERN_ALERT "NCM started at time %lu\n", now_ms() );

	program[0].type = NOP;
	program[1].type = FUTURE;
	program[1].args[0] = 1000;
	program[2].type = HALT;
	program[3].type = END_OF_PROGRAM;

	start_interpreter(&ncm_interp, program, PROGRAM_LEN);

	return 0;
}

void cleanup_module(void) {
	stop_interpreter(&ncm_interp);

	printk( KERN_ALERT "NCM ended at time %lu\n", now_ms() );
}
