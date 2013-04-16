#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "sysfs.h"

//    sscanf(buf, "%d", &a->value);

static ssize_t ncm_sysfs_show(struct kobject *kobj, struct attribute *attr,
        char *buf)
{
	ncm_attr_t *a = container_of(attr, ncm_attr_t, attr);
 	u32 channels;
	if(memcmp(attr->name, "code", 4) == 0){
		// dump the interpreter array
		memcpy(buf, a->ncm_sysfs->program->instructions, sizeof(ncm_instr_t) * a->ncm_sysfs->program->length);
		return sizeof(ncm_instr_t) * a->ncm_sysfs->program->length;
	} else if(memcmp(attr->name, "control", 7) == 0){
		if(is_running(a->ncm_sysfs->ncm_interp)){
			memcpy(buf, "running\n", sizeof("running\n"));
			return sizeof("running\n");
		} else {
			memcpy(buf, "not running\n", sizeof("not running\n"));
			return sizeof("not running\n");
		}
	} else if(memcmp(attr->name, "params", 6) == 0){
		channels = a->ncm_sysfs->interp_params->network.channels;
		memcpy(buf, a->ncm_sysfs->interp_params, offsetof(ncm_net_params_t, net_device_name));
		// copy to the end of the known size part, overwriting all the pointers we don't want to save
		// note that net_device_name has to be the first pointer in unknown size part
		memcpy(buf + offsetof(ncm_net_params_t, net_device_name), a->ncm_sysfs->interp_params->network.net_device_name,
				channels * IFNAMSIZ);
		memcpy(buf + offsetof(ncm_net_params_t, net_device_name) + channels * IFNAMSIZ, a->ncm_sysfs->interp_params->network.channel_mac,
				channels * ETH_ALEN);
		return offsetof(ncm_net_params_t, net_device_name) + channels * (IFNAMSIZ + ETH_ALEN);
	} else {
		return 0;
	}
//    return scnprintf(buf, PAGE_SIZE, "%d\n", a->value);
}

static ssize_t ncm_sysfs_store(struct kobject *kobj, struct attribute *attr,
        const char *buf, size_t len)
{
 	ncm_attr_t *a = container_of(attr, ncm_attr_t, attr);
 	u32 channels;
 	ncm_net_params_t *network;
	if(memcmp(attr->name, "params", 6) == 0){
		// make sure we don't modify interpreter state while it's running
		if(!is_running(a->ncm_sysfs->ncm_interp)){
			// make sure the input is correctly formatted
			channels = ((ncm_interp_params_t*)buf)->network.channels;
			if(len == offsetof(ncm_net_params_t, net_device_name) + (IFNAMSIZ+ETH_ALEN) * channels){
				network = &a->ncm_sysfs->interp_params->network;
				// free the old params if there were any
				if(network->net_device_name != 0){
					kfree(network->net_device_name);
				}
				if(network->channel_mac != 0){
					kfree(network->channel_mac);
				}
				// copy the given data to the new params
				memcpy(a->ncm_sysfs->interp_params, buf, offsetof(ncm_net_params_t, net_device_name));

				// create new params
				network->net_device_name = kmalloc(IFNAMSIZ * channels, GFP_KERNEL);
				network->channel_mac = kmalloc(ETH_ALEN * channels, GFP_KERNEL);

				memcpy(network->net_device_name, buf + offsetof(ncm_net_params_t, net_device_name), IFNAMSIZ * channels);
				memcpy(network->channel_mac, buf + offsetof(ncm_net_params_t, net_device_name) +
						IFNAMSIZ * channels, ETH_ALEN * channels);
			} else {
				printk(KERN_WARNING "Invalid params file.");
			}
		} else {
			printk(KERN_WARNING "Tried to load params into running ncm program. Please stop it first.");
		}
	} else if(memcmp(attr->name, "code", 4) == 0){
		// make sure we don't modify interpreter state while it's running
		if(!is_running(a->ncm_sysfs->ncm_interp)){
			// free the instructions if there were any
			if(a->ncm_sysfs->program->instructions != 0){
				kfree(a->ncm_sysfs->program->instructions);
			}
			// create new instructions block
			a->ncm_sysfs->program->instructions = kmalloc(len, GFP_KERNEL);
			// copy the given data into it
			memcpy(a->ncm_sysfs->program->instructions, buf, len);
		} else {
			printk(KERN_WARNING "Tried to load code into running ncm program. Please stop it first.");
		}
	} else if(memcmp(attr->name, "control", 7) == 0){
		// if you write "run" into the command
		if(memcmp(buf, "run", 3) == 0){
			if(!is_running(a->ncm_sysfs->ncm_interp)){
				if(a->ncm_sysfs->program->instructions == NULL){
		            printk(KERN_WARNING "Cannot start because no network code program has been loaded.");
					return len;
				}
				if(a->ncm_sysfs->interp_params->network.net_device_name == NULL || a->ncm_sysfs->interp_params->network.channel_mac == NULL){
		            printk(KERN_WARNING "Cannot start because no network code parameters have been loaded.");
					return len;
				}
				start_interpreter(a->ncm_sysfs->ncm_interp, a->ncm_sysfs->program, a->ncm_sysfs->interp_params);
			}
		// if you write "stop" into the command
		} else if (memcmp(buf, "stop", 4) == 0){
			if(is_running(a->ncm_sysfs->ncm_interp)){
				stop_interpreter(a->ncm_sysfs->ncm_interp);
			}
		} else {
			return len;
		}
	} else {
		return len;
	}

    return len;// return ??
}

int nc_init_sysfs(ncm_sysfs_t *sysfs, ncm_interpreter_t *ncm_interp,
		ncm_program_t *program,  ncm_interp_params_t *interp_params) {
    int err = -1;

    // pack the struct in the needed format to set up the sysfs device
    sysfs->program = program;
    sysfs->interp_params = interp_params;
    sysfs->ncm_interp = ncm_interp;
    memcpy(sysfs->nc_sysfs_attr_code_name, "code", 5);
    sysfs->nc_sysfs_attr_code.attr.name=sysfs->nc_sysfs_attr_code_name;
    sysfs->nc_sysfs_attr_code.attr.mode=0644;
    sysfs->nc_sysfs_attr_code.ncm_sysfs=sysfs;
    memcpy(sysfs->nc_sysfs_attr_code_params, "params", 7);
    sysfs->nc_sysfs_attr_params.attr.name=sysfs->nc_sysfs_attr_code_params;
    sysfs->nc_sysfs_attr_params.attr.mode=0644;
    sysfs->nc_sysfs_attr_params.ncm_sysfs=sysfs;
    memcpy(sysfs->nc_sysfs_attr_control_name, "control", 8);
    sysfs->nc_sysfs_attr_control.attr.name=sysfs->nc_sysfs_attr_control_name;
    sysfs->nc_sysfs_attr_control.attr.mode=0644;
    sysfs->nc_sysfs_attr_control.ncm_sysfs=sysfs;
    sysfs->attrs[0] = &sysfs->nc_sysfs_attr_code.attr;
    sysfs->attrs[1] = &sysfs->nc_sysfs_attr_control.attr;
    sysfs->attrs[2] = &sysfs->nc_sysfs_attr_params.attr;
    sysfs->attrs[3] = NULL;
	sysfs->nc_kobj_ops.show = ncm_sysfs_show;
	sysfs->nc_kobj_ops.store = ncm_sysfs_store;
    sysfs->nc_kobj_type.sysfs_ops = &sysfs->nc_kobj_ops;
    sysfs->nc_kobj_type.default_attrs = sysfs->attrs;

    sysfs->nc_kobj = kzalloc(sizeof(*sysfs->nc_kobj), GFP_KERNEL);
    if (sysfs->nc_kobj) {
        kobject_init(sysfs->nc_kobj, &sysfs->nc_kobj_type);
        if (kobject_add(sysfs->nc_kobj, NULL, "%s", "network_code")) {
             err = -1;
             printk(KERN_WARNING "Sysfs creation failed");
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
