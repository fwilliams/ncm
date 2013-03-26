/*
 * msg_space.c
 *
 *  Created on: 2013-03-23
 *      Author: Francis Williams
 */

#include <linux/if_ether.h>
#include "msg_space.h"

int init_message_space(message_space_t* msgspace) {
	s32 i;

	for(i = 0; i < MAX_MESSAGES; i++) {
		rwlock_init(&msgspace->at[i].lock);
		msgspace->at[i].length = 0;
		msgspace->at[i].data = msgspace->at[i].buff + ETH_HLEN;
	}

	return MSG_SPACE_OK;
}

int copy_message_from_var(message_space_t* msgspace, varspace_t* varspace, u32 msg_id, u32 var_id) {
	write_lock(&msgspace->at[msg_id].lock);

	get_variable_data(varspace, var_id, msgspace->at[msg_id].data, &msgspace->at[msg_id].length);

	write_unlock(&msgspace->at[msg_id].lock);

	return MSG_SPACE_OK;
}
