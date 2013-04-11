/*
 * hax.h
 *
 *  Created on: 2013-03-26
 *      Author: francis
 */

#include <linux/string.h>

#ifndef HAX_H_
#define HAX_H_

#define TYPE_ARCH1 1
#define TYPE_ARCH2 2

void make_program(ncm_program_t* program, ncm_net_params_t* params, int type) {
	u8 vm1_mac[] = { 0x08, 0x00, 0x27, 0x46, 0xBC, 0x02 };
	u8 vm2_mac[] = { 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B };
//	u8 vm1_mac[] = {0x0a, 0x00, 0x27, 0x00, 0x00, 0x00};
//	u8 vm2_mac[] = {0x0a, 0x00, 0x27, 0x00, 0x00, 0x00};
	u8 devname1[16] = "eth0";
	u8 devname2[16] = "eth0";
	int i;


	switch(type) {
	case TYPE_ARCH1:
		params->channels = 1;
		program->length = 9;
		params->channel_mac = kmalloc(ETH_ALEN * params->channels, GFP_KERNEL);
		params->net_device_name = kmalloc(IFNAMSIZ * params->channels, GFP_KERNEL);
		program->instructions = kmalloc(sizeof(ncm_instr_t) * program->length, GFP_KERNEL);

		memcpy(params->net_device_name[0], devname1, IFNAMSIZ);
		memcpy(params->channel_mac[0], vm2_mac, ETH_ALEN);

		/*
		 * ArchVM Program
		 */
		i = 0;
		program->instructions[i].type = CREATE;
		program->instructions[i].args[0] = 0;
		program->instructions[i].args[1] = 0;
		i++;
		program->instructions[i].type = SEND;
		program->instructions[i].args[0] = 0;
		program->instructions[i].args[1] = 0;
		i++;
		program->instructions[i].type = SYNC;
		program->instructions[i].args[0] = SYNC_MASTER;
		program->instructions[i].args[1] = 0;
		i++;
		program->instructions[i].type = FUTURE;
		program->instructions[i].args[0] = 100000;
		program->instructions[i].args[1] = 5;
		i++;
		program->instructions[i].type = HALT;
		i++;
		program->instructions[i].type = RECEIVE;
		program->instructions[i].args[0] = 0;
		program->instructions[i].args[1] = 1;
		i++;
		program->instructions[i].type = CREATE;
		program->instructions[i].args[0] = 0;
		program->instructions[i].args[1] = 0;
		i++;
		program->instructions[i].type = SEND;
		program->instructions[i].args[0] = 0;
		program->instructions[i].args[1] = 0;
		i++;
		program->instructions[i].type = IF;
		program->instructions[i].args[0] = ALWAYS_TRUE;
		program->instructions[i].args[1] = 3;
		break;

	case TYPE_ARCH2:
		params->channels = 1;
		program->length = 7;
		params->channel_mac = kmalloc(ETH_ALEN * params->channels, GFP_KERNEL);
		params->net_device_name = kmalloc(IFNAMSIZ * params->channels, GFP_KERNEL);
		program->instructions = kmalloc(sizeof(ncm_instr_t) * program->length, GFP_KERNEL);

		memcpy(params->net_device_name[0], devname2, IFNAMSIZ);
		memcpy(params->channel_mac[0], vm1_mac, ETH_ALEN);

		/*
		 * ArchVM2 Program
		 */
		i = 0;
		program->instructions[i].type = SYNC;
		program->instructions[i].args[0] = SYNC_SLAVE;
		program->instructions[i].args[1] = 10000000;
		i++;
		program->instructions[i].type = CREATE;
		program->instructions[i].args[0] = 0;
		program->instructions[i].args[1] = 0;
		i++;
		program->instructions[i].type = SEND;
		program->instructions[i].args[0] = 0;
		program->instructions[i].args[1] = 0;
		i++;
		program->instructions[i].type = FUTURE;
		program->instructions[i].args[0] = 100000;
		program->instructions[i].args[1] = 5;
		i++;
		program->instructions[i].type = HALT;
		i++;
		program->instructions[i].type = RECEIVE;
		program->instructions[i].args[0] = 0;
		program->instructions[i].args[1] = 1;
		i++;
		program->instructions[i].type = IF;
		program->instructions[i].args[0] = ALWAYS_TRUE;
		program->instructions[i].args[1] = 1;

		break;
	}
}


#endif /* HAX_H_ */
