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

void make_program(netcode_instr_t* instructions, ncm_net_params_t* params, int type) {
	u8 vm1_mac[] = { 0x08, 0x00, 0x27, 0x46, 0xBC, 0x02 };
	u8 vm2_mac[] = { 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B };
	u8 devname1[] = "enp0s3";
	u8 devname2[] = "enp0s3";

	switch(type) {
	case TYPE_ARCH1:
		memcpy(params->net_device_name[0], devname1, 6);
		*params->mac_address = *vm1_mac;
		*params->channel_mac[0] = *vm2_mac;

		/*
		 * ArchVM Program
		 */
		instructions[0].type = CREATE;
		instructions[0].args[0] = 0;
		instructions[0].args[1] = 0;

		instructions[1].type = SEND;
		instructions[1].args[0] = 0;
		instructions[1].args[1] = 0;

		instructions[2].type = SYNC;
		instructions[2].args[0] = SYNC_MASTER;
		instructions[2].args[1] = 0;

		instructions[3].type = WAIT;
		instructions[3].args[0] = 10000000;
		instructions[3].args[1] = 4;

		instructions[4].type = RECEIVE;
		instructions[4].args[0] = 0;
		instructions[4].args[1] = 1;

		instructions[5].type = CREATE;
		instructions[5].args[0] = 0;
		instructions[5].args[1] = 0;

		instructions[6].type = SEND;
		instructions[6].args[0] = 0;
		instructions[6].args[1] = 0;

		instructions[7].type = GOTO;
		instructions[7].args[0] = 2;
		break;

	case TYPE_ARCH2:
		memcpy(params->net_device_name[0], devname2, 4);
		*params->mac_address = *vm2_mac;
		*params->channel_mac[0] = *vm1_mac;

		/*
		 * ArchVM2 Program
		 */
		instructions[0].type = SYNC;
		instructions[0].args[0] = SYNC_SLAVE;
		instructions[0].args[1] = 10000000;

		instructions[1].type = RECEIVE;
		instructions[1].args[0] = 0;
		instructions[1].args[1] = 1;

		instructions[1].type = CREATE;
		instructions[1].args[0] = 0;
		instructions[1].args[1] = 0;

		instructions[2].type = SEND;
		instructions[2].args[0] = 0;
		instructions[2].args[1] = 0;

		instructions[3].type = WAIT;
		instructions[3].args[0] = 10000000;
		instructions[3].args[1] = 1;
		break;
	}
}


#endif /* HAX_H_ */
