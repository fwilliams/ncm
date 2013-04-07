/*
 * guards.c
 *
 *  Created on: 2013-03-22
 *      Author: Francis Williams
 */

#include "guards.h"

bool handle_chanel_xy_rcv_buf_empty(u32 channel_id) {
	return false;
}

bool handle_send_buffer_empty(void) {
	return false;
}

bool handle_test_count(ncm_interpreter_t* interpreter, u32 counter_id, u32 test_value) {
	return (interpreter->counters.at[counter_id] == test_value);
}

bool handle_status_test(ncm_interpreter_t* interpreter, u32 test_value) {
	u16 mask = test_value & 0x0FFFF;
	return is_error_set(interpreter, mask);
}

bool handle_equal_var_var(ncm_interpreter_t* interpreter, u32 var_id_1, u32 var_id_2) {
	return (cmp_variables(&interpreter->variable_space, var_id_1, var_id_2) == 0);
}

bool handle_greater_var_var(ncm_interpreter_t* interpreter, u32 var_id_1, u32 var_id_2) {
	return (cmp_variables(&interpreter->variable_space, var_id_1, var_id_2) > 0);
}

bool handle_less_var_var(ncm_interpreter_t* interpreter, u32 var_id_1, u32 var_id_2) {
	return (cmp_variables(&interpreter->variable_space, var_id_1, var_id_2) < 0);
}

bool handle_test_var(ncm_interpreter_t* interpreter, u32 var_id, u32 test_mask) {
	return test_variable(&interpreter->variable_space, var_id, test_mask);
}

bool handle_var_queue_empty(u32 var_id) {
	return false;
}

bool handle_var_queue_full(u32 var_id) {
	return false;
}

/*
 * Tests if a guard is true or false
 */
bool test_guard(ncm_interpreter_t* interpreter, u32 guard_id, u32* args, u32* status) {
	*status = GUARD_OK;

	switch(guard_id) {
	case ALWAYS_TRUE:
		return true;
	case ALWAYS_FALSE:
		return false;
	case CHANNEL_XY_RCV_BUF_EMTPY:
		return handle_chanel_xy_rcv_buf_empty(args[0]);
	case SEND_BUFFER_EMPTY:
		return handle_send_buffer_empty();
	case TEST_COUNT:
		return handle_test_count(interpreter, args[0], args[1]);
	case STATUS_TEST:
		return handle_status_test(interpreter, args[0]);
	case EQUAL_VAR_VAR:
		return handle_equal_var_var(interpreter, args[0], args[1]);
	case GREATER_VAR_VAR:
		return handle_greater_var_var(interpreter, args[0], args[1]);
	case LESS_VAR_VAR:
		return handle_less_var_var(interpreter, args[0], args[1]);
	case TEST_VAR:
		return handle_test_var(interpreter, args[0], args[1]);
	case VAR_QUEUE_EMPTY:
		return handle_var_queue_empty(args[0]);
	case VAR_QUEUE_FULL:
		return handle_var_queue_full(args[0]);
	default:
		*status = INVALID_GUARD;
		return false;
	}
}




