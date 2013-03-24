/*
 * msg_space.c
 *
 *  Created on: 2013-03-23
 *      Author: Francis Williams
 */

#include "msg_space.h"

int init_message_space(message_space_t* msgspace) {
	s32 i;

	for(i = 0; i < MAX_MESSAGES; i++) {
		rwlock_init(&msgspace->at[i].lock);
		msgspace->at[i].length = 0;
	}

	return MSG_SPACE_OK;
}

int copy_message_from_var(message_space_t* msgspace, varspace_t* varspace, u32 msg_id, u32 var_id) {
	write_lock(&msgspace->at[msg_id].lock);

	get_variable_data(varspace, var_id, msgspace->at[msg_id]->data, &msgspace->at[msg_id]->length);

	write_unlock(&msgspace->at[msg_id].lock);

	return MSG_SPACE_OK;
}

int copy_message_to_var(message_space_t* msgspace, varspace_t* varspace, u32 msg_id, u32 var_id) {
	read_lock(&msgspace->at[msg_id].lock);

	set_variable_data(varspace, var_id, msgspace->at[msg_id].data, msgspace->at[msg_id].length);

	read_unlock(&msgspace->at[msg_id].lock);

	return MSG_SPACE_OK;
}

int get_message(message_space_t* msgspace, u32 msg_id, u8* out_data, u32* out_len) {
	read_lock(&msgspace->at[msg_id].lock);

	memcpy(out_data, msgspace->at[msg_id].data, msgspace->at[msg_id].length);
	*out_len = msgspace->at[msg_id].length;

	read_lock(&msgspace->at[msg_id].lock);

	return MSG_SPACE_OK;
}

int set_message(message_space_t* msgspace, u32 msg_id, u8* data, u32 len) {
	write_lock(&msgspace->at[msg_id].lock);

	memcpy(msgspace->at[msg_id].data, data, len);
	msgspace->at[msg_id].length = len;

	write_unlock(&msgspace->at[msg_id].lock);

	return MSG_SPACE_OK;
}



