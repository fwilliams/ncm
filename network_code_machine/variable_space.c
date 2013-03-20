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
		varspace->at[i].varbuf[0].length = 0;
		varspace->at[i].varbuf[1].length = 0;

		varspace->at[i].read_buf = &varspace->at[i].varbuf[0];
		varspace->at[i].write_buf = &varspace->at[i].varbuf[1];
		spin_lock_init(&varspace->at[i].write_lock);
	}

	return VARSPACE_OK;
}

/* Restores all resources used by the variable space */
int destroy_variable_space(varspace_t* varspace) {
	return VARSPACE_OK;
}

/* Gets the data stored for the variable id argument */
int get_variable_data(varspace_t* varspace, u32 var_id, u8* out_data, size_t* out_length) {
	variable_t* var;

	rcu_read_lock();

	var = &varspace->at[var_id];

	memcpy(
		out_data,
		rcu_dereference(var->read_buf)->data,
		rcu_dereference(var->read_buf)->length
	);

	*out_length = rcu_dereference(var->read_buf)->length;

	out_data[*out_length] = '\0';

	rcu_read_unlock();

	return VARSPACE_OK;
}

/* Sets the data for the variable id argument to the data argument */
int set_variable_data(varspace_t* varspace, u32 var_id, u8* data, size_t length) {
	variable_t* var;

	var = &varspace->at[var_id];

	spin_lock(&var->write_lock);
		memcpy(var->write_buf->data, data, length);
		var->write_buf->length = length;
		var->write_buf = var->read_buf;

		rcu_assign_pointer(var->read_buf, var->write_buf);
	spin_unlock(&var->write_lock);

	synchronize_rcu();

	return VARSPACE_OK;
}
