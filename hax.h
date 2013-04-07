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


	switch(type) {
	case TYPE_ARCH1:
		params->channels = 1;
		program->length = 8;
		params->channel_mac = kmalloc(ETH_ALEN * params->channels, GFP_KERNEL);
		params->net_device_name = kmalloc(IFNAMSIZ * params->channels, GFP_KERNEL);
		program->instructions = kmalloc(sizeof(ncm_instr_t) * program->length, GFP_KERNEL);

		memcpy(params->net_device_name[0], devname1, IFNAMSIZ);
		memcpy(params->mac_address, vm1_mac, ETH_ALEN);
		memcpy(params->channel_mac[0], vm2_mac, ETH_ALEN);

		/*
		 * ArchVM Program
		 */
		program->instructions[0].type = CREATE;
		program->instructions[0].args[0] = 0;
		program->instructions[0].args[1] = 0;

		program->instructions[1].type = SEND;
		program->instructions[1].args[0] = 0;
		program->instructions[1].args[1] = 0;

		program->instructions[2].type = SYNC;
		program->instructions[2].args[0] = SYNC_MASTER;
		program->instructions[2].args[1] = 0;

		program->instructions[3].type = WAIT;
		program->instructions[3].args[0] = 100000;
		program->instructions[3].args[1] = 4;

		program->instructions[4].type = RECEIVE;
		program->instructions[4].args[0] = 0;
		program->instructions[4].args[1] = 1;

		program->instructions[5].type = CREATE;
		program->instructions[5].args[0] = 0;
		program->instructions[5].args[1] = 0;

		program->instructions[6].type = SEND;
		program->instructions[6].args[0] = 0;
		program->instructions[6].args[1] = 0;

		program->instructions[7].type = GOTO;
		program->instructions[7].args[0] = 3;
		break;

	case TYPE_ARCH2:
		params->channels = 1;
		program->length = 6;
		params->channel_mac = kmalloc(ETH_ALEN * params->channels, GFP_KERNEL);
		params->net_device_name = kmalloc(IFNAMSIZ * params->channels, GFP_KERNEL);
		program->instructions = kmalloc(sizeof(ncm_instr_t) * program->length, GFP_KERNEL);

		memcpy(params->net_device_name[0], devname2, IFNAMSIZ);
		memcpy(params->mac_address, vm2_mac, ETH_ALEN);
		memcpy(params->channel_mac[0], vm1_mac, ETH_ALEN);

		/*
		 * ArchVM2 Program
		 */
		program->instructions[0].type = SYNC;
		program->instructions[0].args[0] = SYNC_SLAVE;
		program->instructions[0].args[1] = 10000000;

		program->instructions[1].type = CREATE;
		program->instructions[1].args[0] = 0;
		program->instructions[1].args[1] = 0;

		program->instructions[2].type = SEND;
		program->instructions[2].args[0] = 0;
		program->instructions[2].args[1] = 0;

		program->instructions[3].type = WAIT;
		program->instructions[3].args[0] = 100000;
		program->instructions[3].args[1] = 4;

		program->instructions[4].type = RECEIVE;
		program->instructions[4].args[0] = 0;
		program->instructions[4].args[1] = 1;

		program->instructions[5].type = GOTO;
		program->instructions[5].args[0] = 1;

		break;
	}
}


#endif /* HAX_H_ */
