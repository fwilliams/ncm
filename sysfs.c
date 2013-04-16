#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>

#include "sysfs.h"

//    sscanf(buf, "%d", &a->value);


static ncm_sysfs_t sysfs;

// the attrs here are wrapped twice: once inside a kobj_attribute and once inside a ncm_attr_t
static struct attribute *attrs[MAX_VARIABLES+1];

static struct attribute_group attr_group = {
	.name = "varspace",
	.attrs = attrs,
};

static ssize_t ncm_sysfs_var_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	u32 klen, var;
	sscanf(attr->attr.name, "%i", &var);
	get_variable_data(&sysfs.ncm_interp->variable_space, var, buf, &klen);
	return klen;
}

static ssize_t ncm_sysfs_var_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len) {
	u32 var;
	sscanf(attr->attr.name, "%i", &var);
	set_variable_data(&sysfs.ncm_interp->variable_space, var, (u8*)buf, len);
	return len;
}

static ssize_t ncm_sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
 	u32 channels;
	if(strcmp(attr->attr.name, "code") == 0){
		// dump the interpreter array
		memcpy(buf, sysfs.program->instructions, sizeof(ncm_instr_t) * sysfs.program->length);
		return sizeof(ncm_instr_t) * sysfs.program->length;
	} else if(strcmp(attr->attr.name, "control") == 0){
		if(is_running(sysfs.ncm_interp)){
			memcpy(buf, "running\n", sizeof("running\n"));
			return sizeof("running\n");
		} else {
			memcpy(buf, "not running\n", sizeof("not running\n"));
			return sizeof("not running\n");
		}
	} else if(strcmp(attr->attr.name, "params") == 0){
		channels = sysfs.interp_params->network.channels;
		memcpy(buf, sysfs.interp_params, offsetof(ncm_net_params_t, net_device_name));
		// copy to the end of the known size part, overwriting all the pointers we don't want to save
		// note that net_device_name has to be the first pointer in unknown size part
		memcpy(buf + offsetof(ncm_net_params_t, net_device_name), sysfs.interp_params->network.net_device_name,
				channels * IFNAMSIZ);
		memcpy(buf + offsetof(ncm_net_params_t, net_device_name) + channels * IFNAMSIZ, sysfs.interp_params->network.channel_mac,
				channels * ETH_ALEN);
		return offsetof(ncm_net_params_t, net_device_name) + channels * (IFNAMSIZ + ETH_ALEN);
	} else {
		return 0;
	}
//    return scnprintf(buf, PAGE_SIZE, "%d\n", a->value);
}

static ssize_t ncm_sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t len) {
 	u32 channels;
 	ncm_net_params_t *network;
	if(strcmp(attr->attr.name, "params") == 0){
		// make sure we don't modify interpreter state while it's running
		if(!is_running(sysfs.ncm_interp)){
			// make sure the input is correctly formatted
			channels = ((ncm_interp_params_t*)buf)->network.channels;
			if(len == offsetof(ncm_net_params_t, net_device_name) + (IFNAMSIZ+ETH_ALEN) * channels){
				network = &sysfs.interp_params->network;
				// free the old params if there were any
				if(network->net_device_name != 0){
					kfree(network->net_device_name);
				}
				if(network->channel_mac != 0){
					kfree(network->channel_mac);
				}
				// copy the given data to the new params
				memcpy(sysfs.interp_params, buf, offsetof(ncm_net_params_t, net_device_name));

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
	} else if(strcmp(attr->attr.name, "code") == 0){
		// make sure we don't modify interpreter state while it's running
		if(!is_running(sysfs.ncm_interp)){
			// free the instructions if there were any
			if(sysfs.program->instructions != 0){
				kfree(sysfs.program->instructions);
			}
			// create new instructions block
			sysfs.program->instructions = kmalloc(len, GFP_KERNEL);
			sysfs.program->length = len;
			// copy the given data into it
			memcpy(sysfs.program->instructions, buf, len);
		} else {
			printk(KERN_WARNING "Tried to load code into running ncm program. Please stop it first.");
		}
	} else if(strcmp(attr->attr.name, "control") == 0){
		// if you write "run" into the command
		if(memcmp(buf, "run", 3) == 0){
			if(!is_running(sysfs.ncm_interp)){
				if(sysfs.program->instructions == NULL){
		            printk(KERN_WARNING "Cannot start because no network code program has been loaded.");
					return len;
				}
				if(sysfs.interp_params->network.net_device_name == NULL || sysfs.interp_params->network.channel_mac == NULL){
		            printk(KERN_WARNING "Cannot start because no network code parameters have been loaded.");
					return len;
				}
				start_interpreter(sysfs.ncm_interp, sysfs.program, sysfs.interp_params);
			}
		// if you write "stop" into the command
		} else if (memcmp(buf, "stop", 4) == 0){
			if(is_running(sysfs.ncm_interp)){
				stop_interpreter(sysfs.ncm_interp);
			}
		} else {
			return len;
		}
	} else {
		return len;
	}

    return len;// return ??
}

struct kobj_attribute nc_sysfs_attr_code = {
	.attr = {"code", 0644},
	.show = ncm_sysfs_show,
	.store = ncm_sysfs_store
};
struct kobj_attribute nc_sysfs_attr_params = {
	.attr = {"params", 0644},
	.show = ncm_sysfs_show,
	.store = ncm_sysfs_store
};
struct kobj_attribute nc_sysfs_attr_control = {
	.attr = {"control", 0644},
	.show = ncm_sysfs_show,
	.store = ncm_sysfs_store
};
char sysfs_var_names[MAX_VARIABLES][ATTR_NAME_LEN];

int nc_init_sysfs(ncm_interpreter_t *ncm_interp,
		ncm_program_t *program,  ncm_interp_params_t *interp_params) {
    int err = 0, i;
    struct kobj_attribute *kattr;

    // pack the struct in the needed format to set up the sysfs device
    sysfs.program = program;
    sysfs.interp_params = interp_params;
    sysfs.ncm_interp = ncm_interp;

	sysfs.nc_kobj = kobject_create_and_add("network_code", NULL);
	if (!sysfs.nc_kobj)
		return -ENOMEM;

	for(i=0; i<MAX_VARIABLES; i++){
		kattr = kzalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
		snprintf(sysfs_var_names[i], ATTR_NAME_LEN, "%i" , i);
		kattr->attr.name = sysfs_var_names[i];
		kattr->attr.mode = 0666;
		kattr->show	= ncm_sysfs_var_show;
		kattr->store = ncm_sysfs_var_store;
		attrs[i] = &kattr->attr;
	}
	attrs[i] = NULL;

	err = sysfs_create_file(sysfs.nc_kobj, &nc_sysfs_attr_code.attr);
	if (err) goto err;
	err = sysfs_create_file(sysfs.nc_kobj, &nc_sysfs_attr_control.attr);
	if (err) goto err;
	err = sysfs_create_file(sysfs.nc_kobj, &nc_sysfs_attr_params.attr);
	if (err) goto err;
	err = sysfs_create_group(sysfs.nc_kobj, &attr_group);
	if (err) goto err;

    return err;
err:
	kobject_put(sysfs.nc_kobj);
	sysfs.nc_kobj = NULL;
    return err;
}

void ncm_sysfs_cleanup(void){
	//free attrs
	int i;
	for(i=0; i<MAX_VARIABLES; i++){
		if(attrs[i] != 0)
			kfree(container_of(attrs[i], struct kobj_attribute, attr));
	}
	if (sysfs.nc_kobj) {
		kobject_put(sysfs.nc_kobj);
	}
}
