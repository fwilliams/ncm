/*
 * variable_space.c
 * The variable space for the Network Code interpreter
 *  Created on: 2013-03-11
 *      Author: Francis Williams
 */

#include "variable_space.h"
#include "netcode_helper.h"

/* Initializes a new variable_space */
int init_variable_space(ncm_varspace_t* varspace) {
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
int destroy_variable_space(ncm_varspace_t* varspace) {
	return VARSPACE_OK;
}

/* Gets the data stored for the variable id argument */
int get_variable_data(ncm_varspace_t* varspace, u32 var_id, u8* out_data, size_t* out_length) {
	variable_t* var;

	rcu_read_lock();

	var = &varspace->at[var_id];

	memcpy(
		out_data,
		rcu_dereference(var->read_buf)->data,
		rcu_dereference(var->read_buf)->length
	);

	*out_length = rcu_dereference(var->read_buf)->length;

	rcu_read_unlock();

	return VARSPACE_OK;
}

/* Sets the data for the variable id argument to the data argument */
int set_variable_data(ncm_varspace_t* varspace, u32 var_id, u8* data, size_t length) {
	variable_t* var;
	vardata_t* tmp;

	var = &varspace->at[var_id];

	spin_lock(&var->write_lock);
		memcpy(var->write_buf->data, data, length);
		rcu_dereference(var->write_buf)->length = length;

		tmp = rcu_dereference(var->read_buf);
		rcu_assign_pointer(var->read_buf, var->write_buf);
		rcu_assign_pointer(var->write_buf, tmp);

	spin_unlock(&var->write_lock);


	synchronize_rcu();

	return VARSPACE_OK;
}

/*
 * Returns 0 if two variables are equal, returns a positive number if var_1 > var_2, and
 * returns a negative number if var_1 < var_2.
 *
 * NOTE: Variable comparison is done via byte-wise subtraction: var_1[i] - var_2[i]
 * until a non-zero result is found. The sign of the result determines the return value
 * of this function.
 */
int cmp_variables(ncm_varspace_t* varspace, u32 var_id_1, u32 var_id_2) {
	s32 i, result;
	variable_t *var1, *var2;
	size_t len1, len2;

	rcu_read_lock();

	var1 = &(varspace->at[var_id_1]);
	var2 = &(varspace->at[var_id_2]);

	len1 = rcu_dereference(var1->read_buf)->length;
	len2 = rcu_dereference(var2->read_buf)->length;

	result = 0;

	if(len1 > len2) {
		for(i = 0; i < (len1 - len2); i++) {
			if(rcu_dereference(var1->read_buf)->data[i] != 0) {
				rcu_read_unlock();
				return rcu_dereference(var1->read_buf)->data[i];
			}
		}
		result = memcmp(&rcu_dereference(var1->read_buf)->data[(len1 - len2)], rcu_dereference(var2->read_buf)->data, len2);
	} else if(len1 < len2) {
		for(i = 0; i < (len2 - len1); i++) {
			if(rcu_dereference(var2->read_buf)->data[i] != 0) {
				rcu_read_unlock();
				return -rcu_dereference(var2->read_buf)->data[i];
			}
		}
		result = memcmp(rcu_dereference(var1->read_buf)->data, &rcu_dereference(var2->read_buf)->data[(len2 - len1)], len1);
	} else if(len1 == len2) {
		result = memcmp(rcu_dereference(var1->read_buf)->data, rcu_dereference(var2->read_buf)->data, len1);
	}

	rcu_read_unlock();

	return result;
}

/********************************************************************
 * Start test_variable() handlers
 *******************************************************************/
bool handle_test_var_is_zero(ncm_varspace_t* varspace, u32 var_id) {
	s32 i;
	variable_t *var;

	rcu_read_lock();

	var = &(varspace->at[var_id]);

	for(i = 0; i < rcu_dereference(var->read_buf)->length; i++) {
		if(rcu_dereference(var->read_buf)->data[i] != 0) {
			rcu_read_unlock();
			return false;
		}
	}

	rcu_read_unlock();

	return true;
}

bool handle_test_var_is_nonzero(ncm_varspace_t* varspace, u32 var_id) {
	s32 i;
	variable_t *var;

	rcu_read_lock();

	var = &(varspace->at[var_id]);

	for(i = 0; i < rcu_dereference(var->read_buf)->length; i++) {
		if(rcu_dereference(var->read_buf)->data[i] != 0) {
			rcu_read_unlock();
			return true;
		}
	}

	rcu_read_unlock();

	return false;
}

bool handle_test_var_even_parity(ncm_varspace_t* varspace, u32 var_id) {
	u32 parity, i;
	variable_t *var;

	rcu_read_lock();

	var = &(varspace->at[var_id]);
	parity = 0;

	for(i = 0; i < rcu_dereference(var->read_buf)->length; i++) {
		parity += get_parity(rcu_dereference(var->read_buf)->data[i]);
	}

	rcu_read_unlock();

	return !(parity & 1);
}

bool handle_test_var_odd_parity(ncm_varspace_t* varspace, u32 var_id) {
	u32 parity, i;
	variable_t *var;

	rcu_read_lock();

	var = &(varspace->at[var_id]);
	parity = 0;

	for(i = 0; i < rcu_dereference(var->read_buf)->length; i++) {
		parity += get_parity(rcu_dereference(var->read_buf)->data[i]);
	}

	rcu_read_unlock();

	return (parity & 1);
}
/********************************************************************
 * End test_variable() handlers
 *******************************************************************/

/*
 * Tests if the given condition is true for the variable. The possible
 * condition masks are:
 * 	* VAR_IS_ZERO		-- Returns true if the variable is zero. Note: A variable of length 0 is considered equal to 0
 * 	* VAR_IS_NONZERO	-- Returns true if the variable is non-zero
 * 	* VAR_EVEN_PARITY	-- Returns true if the variable has even parity
 *	* VAR_ODD_PARITY	-- Returns true if the variable has odd parity
 */
bool test_variable(ncm_varspace_t* varspace, u32 var_id, u32 condition_mask) {
	switch(condition_mask) {
	case VAR_IS_ZERO:
		return handle_test_var_is_zero(varspace, var_id);
	case VAR_IS_NONZERO:
		return handle_test_var_is_nonzero(varspace, var_id);
	case VAR_EVEN_PARITY:
		return handle_test_var_even_parity(varspace, var_id);
	case VAR_ODD_PARITY:
		return handle_test_var_odd_parity(varspace, var_id);
	}
	return false;
}
