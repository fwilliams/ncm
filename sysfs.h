#include "interpreter.h"

#ifndef SYSFS_H_
#define SYSFS_H_

#define ATTR_NAME_LEN 10

typedef struct ncm_sysfs {
	struct kobject *nc_kobj;
	ncm_program_t *program;
	ncm_interp_params_t *interp_params;
	ncm_interpreter_t *ncm_interp;
} ncm_sysfs_t;

int nc_init_sysfs(ncm_interpreter_t *ncm_interp, ncm_program_t *program,
		ncm_interp_params_t *interp_params);

void ncm_sysfs_cleanup(void);

#endif /* SYSFS_H_ */
