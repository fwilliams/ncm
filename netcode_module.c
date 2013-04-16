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

int init_module() {
	int sysfs_ret = 0;

#ifdef VM1
	make_program(&program, &interp_params.network, TYPE_ARCH1);
#endif
#ifdef VM2
	make_program(&program, &interp_params.network, TYPE_ARCH2);
#endif

	init_interpreter(&ncm_interp);

	sysfs_ret = nc_init_sysfs(&ncm_interp, &program, &interp_params);

#ifndef VM
	start_interpreter(&ncm_interp, &program, &interp_params);
#endif

	return sysfs_ret;
}

void cleanup_module(void) {

	ncm_sysfs_cleanup();

	if(is_running(&ncm_interp))
		stop_interpreter(&ncm_interp);

	debug_print("NCM ended at time %llu", now_us() );
}



