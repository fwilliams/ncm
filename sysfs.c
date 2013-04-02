#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "sysfs.h"

static ssize_t ncm_sysfs_show(struct kobject *kobj, struct attribute *attr,
        char *buf)
{
	ncm_attr_t *ncm_a;
	if(memcmp(attr->name, "code", 4) == 0){
		ncm_a = container_of(attr, ncm_attr_t, attr);
		// dump the interpreter array
		memcpy(buf, ncm_a->ncm_sysfs->program->instructions, sizeof(ncm_instr_t) * ncm_a->ncm_sysfs->program->length);
		return sizeof(ncm_instr_t) * ncm_a->ncm_sysfs->program->length;
	} else if(memcmp(attr->name, "control", 7) == 0){
		// TODO: return the state of the interpreter: running, stopped or whatever
		memcpy(buf, "running\n", sizeof("running\n"));
		return sizeof("running\n");
	}
//    return scnprintf(buf, PAGE_SIZE, "%d\n", a->value);
}

static ssize_t ncm_sysfs_store(struct kobject *kobj, struct attribute *attr,
        const char *buf, size_t len)
{
	// memcopy into a new program array
//	nc_sysfs_attr_t *a = container_of(attr, nc_sysfs_attr_t, attr);
//    sscanf(buf, "%d", &a->value);
    return sizeof(int);// return ??
}

int nc_init_sysfs(ncm_sysfs_t *sysfs, ncm_program_t *program) {
    int err = -1;

    // pack the struct in the needed format to set up the sysfs device
    sysfs->program = program;
    memcpy(sysfs->nc_sysfs_attr_code_name, "code", 5);
    sysfs->nc_sysfs_attr_code.attr.name=sysfs->nc_sysfs_attr_code_name;
    sysfs->nc_sysfs_attr_code.attr.mode=0644;
    sysfs->nc_sysfs_attr_code.ncm_sysfs=sysfs;
    memcpy(sysfs->nc_sysfs_attr_control_name, "control", 8);
    sysfs->nc_sysfs_attr_control.attr.name=sysfs->nc_sysfs_attr_control_name;
    sysfs->nc_sysfs_attr_control.attr.mode=0644;
    sysfs->nc_sysfs_attr_control.ncm_sysfs=sysfs;
    sysfs->attrs[0] = &sysfs->nc_sysfs_attr_code.attr;
    sysfs->attrs[1] = &sysfs->nc_sysfs_attr_control.attr;
    sysfs->attrs[2] = NULL;
	sysfs->nc_kobj_ops.show = ncm_sysfs_show;
	sysfs->nc_kobj_ops.store = ncm_sysfs_store;
    sysfs->nc_kobj_type.sysfs_ops = &sysfs->nc_kobj_ops;
    sysfs->nc_kobj_type.default_attrs = sysfs->attrs;

    sysfs->nc_kobj = kzalloc(sizeof(*sysfs->nc_kobj), GFP_KERNEL);
    if (sysfs->nc_kobj) {
        kobject_init(sysfs->nc_kobj, &sysfs->nc_kobj_type);
        if (kobject_add(sysfs->nc_kobj, NULL, "%s", "network_code")) {
             err = -1;
             printk("Sysfs creation failed\n");
             kobject_put(sysfs->nc_kobj);
             sysfs->nc_kobj = NULL;
        } else {
			err = 0;
        }
    }
    return err;
}

void ncm_sysfs_cleanup(ncm_sysfs_t *sysfs){
	if (sysfs->nc_kobj) {
		kobject_put(sysfs->nc_kobj);
		kfree(sysfs->nc_kobj);
	}
}
