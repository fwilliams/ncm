/*
 * variable_space.c
 * The variable space for the Network Code interpreter
 *  Created on: 2013-03-11
 *      Author: Francis Williams
 */

#include "variable_space.h"

/* Initializes a new variable_space */
int init_variable_space(varspace_t* varspace) {
	int i;

	for(i = 0; i<MAX_VARIABLES; i++) {
		varspace->at[i].length[0] = 0;
		varspace->at[i].length[1] = 0;

		rwlock_init(&varspace->at[i].buffer_locks[0]);
		rwlock_init(&varspace->at[i].buffer_locks[1]);

		atomic_set(&varspace->at[i].current_buffer, 0);

		write_lock(&varspace->at[i].buffer_locks[0]);
	}

	return VARSPACE_OK;
}

/* Gets the data stored for the variable id argument */
int get_variable_data(varspace_t* varspace, u32 var_id, void* out_data, size_t* out_length) {
	struct variable* var;
	int curr;

	var = &varspace->at[var_id];
	curr = atomic_read(&var->current_buffer);

	read_lock(&var->buffer_locks[!curr]);

	memcpy(out_data, var->data[curr], var->length[!curr]);
	*out_length = var->length[!curr];

	read_unlock(&var->buffer_locks[!curr]);

	return VARSPACE_OK;
}

/* Sets the data for the variable id argument to the data argument */
int set_variable_data(varspace_t* varspace, u32 var_id, void* data, size_t length) {
	struct variable* var;
	int curr;

	var = &varspace->at[var_id];
	curr = atomic_read(&var->current_buffer);

	memcpy(var->data[curr], data, length);
	var->length[curr] = length;

	write_lock(&var->buffer_locks[!curr]);

	atomic_set(&var->current_buffer, !curr);

	write_unlock(&var->buffer_locks[curr]);


	return VARSPACE_OK;
}
