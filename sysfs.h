#include "interpreter.h"

#ifndef SYSFS_H_
#define SYSFS_H_

typedef struct ncm_attr {
    struct attribute attr;
    struct ncm_sysfs *ncm_sysfs;
} ncm_attr_t;

typedef struct ncm_sysfs {
	struct kobject *nc_kobj;
	struct kobj_type nc_kobj_type;
	struct sysfs_ops nc_kobj_ops;

	struct attribute *attrs[3];
	struct ncm_attr nc_sysfs_attr_code;
	char nc_sysfs_attr_code_name[5];
	struct ncm_attr nc_sysfs_attr_control;
	char nc_sysfs_attr_control_name[8];

	ncm_program_t *program;
} ncm_sysfs_t;

int nc_init_sysfs(ncm_sysfs_t *sysfs, ncm_program_t *program);

void ncm_sysfs_cleanup(ncm_sysfs_t *sysfs);

#endif /* SYSFS_H_ */
