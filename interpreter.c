/*
 * interpreter.c
 * The Network Code interpreter
 *
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */
#include <linux/sched.h>

#include "interpreter.h"
#include "guards.h"
#include "nc_net.h"

/*
 * Statuses that can be returned by instruction
 */
enum instr_result {
	INSTR_OK,
	INSTR_ERROR,
	INSTR_CHANGE_PC
};


/*****************************************************************************
 * Interpreter instruction handlers
 * These functions map 1:1 with Network code instructions
 *****************************************************************************/

static enum instr_result handle_nop(ncm_interpreter_t* interpreter) {
	debug_print("NOP instruction reached at PC = %d", interpreter->program_counter);
	return INSTR_OK;
}

static enum instr_result handle_future(ncm_interpreter_t* interpreter) {
	u32 delay = interpreter->program[interpreter->program_counter].args[0];
	u32 jmp = interpreter->program[interpreter->program_counter].args[1];
	int error;

	debug_print("FUTURE instruction reached at PC = %d with args %d and %d",
			interpreter->program_counter,
			interpreter->program[interpreter->program_counter].args[0],
			interpreter->program[interpreter->program_counter].args[1]);

	if(jmp >= interpreter->program_length) {
		return INSTR_ERROR;
	}

	error = push_future(&interpreter->future_queue, jmp, delay);

	if(error != FUTURE_Q_OK) {
		return INSTR_ERROR;
	}

	return INSTR_OK;
}

static enum instr_result handle_halt(ncm_interpreter_t* interpreter) {
	ncm_future_queue_el_t head;
	int error;
	u64 range;

	debug_print("HALT instruction reached at PC = %d", interpreter->program_counter);

	error = pop_future(&interpreter->future_queue, &head);

	if(error != FUTURE_Q_OK) {
		return INSTR_ERROR;
	}

	range = head.expiry - now_us();

	if(range < 0) {
		return INSTR_ERROR;
	}

	usleep_range(range, range);

	interpreter->program_counter = head.jmp_address;

	return INSTR_CHANGE_PC;
}

static enum instr_result handle_wait(ncm_interpreter_t* interpreter) {
	enum instr_result res;
	res = handle_future(interpreter);

	if(res != INSTR_OK) {
		return res;
	}

	return handle_halt(interpreter);
}

static enum instr_result handle_goto(ncm_interpreter_t* interpreter) {
	u32 jmp = interpreter->program[interpreter->program_counter].args[0];

	debug_print("GOTO instruction reached at PC = %d with arg %d",
			interpreter->program_counter, jmp);

	if(jmp >= interpreter->program_length) {
		return INSTR_ERROR;
	}

	interpreter->program_counter = jmp;

	return INSTR_CHANGE_PC;
}

static enum instr_result handle_end_of_program(ncm_interpreter_t* interpreter) {
	debug_print("END_OF_PROGRAM instruction reached at PC = %d",
			interpreter->program_counter);
	return INSTR_OK;
}

static enum instr_result handle_if(ncm_interpreter_t* interpreter) {
	u32 error;
	u32 guard = interpreter->program[interpreter->program_counter].args[0];
	u32 jmp = interpreter->program[interpreter->program_counter].args[1];
	u32* args = &interpreter->program[interpreter->program_counter].args[2];
	bool result = test_guard(interpreter, guard, args, &error);

	debug_print("IF instruction reached at PC = %d with args %d, %d",
			interpreter->program_counter,
			guard, jmp);

	if(error != GUARD_OK) {
		return INSTR_ERROR;
	}

	if(result) {
		interpreter->program_counter = jmp;
		return INSTR_CHANGE_PC;
	}

	return INSTR_OK;
}

static enum instr_result handle_clear_counter(ncm_interpreter_t* interpreter) {
	u32 counter_id = interpreter->program[interpreter->program_counter].args[0];

	reset_counter(&interpreter->counters, counter_id);

	debug_print("CLEAR_COUNTER instruction reached at PC = %d with arg %d",
			interpreter->program_counter,
			counter_id);

	return INSTR_OK;
}

static enum instr_result handle_add_to_counter(ncm_interpreter_t* interpreter) {
	u32 counter_id = interpreter->program[interpreter->program_counter].args[0];
	u32 amount = interpreter->program[interpreter->program_counter].args[1];

	add_to_counter(&interpreter->counters, counter_id, amount);

	debug_print("ADD_TO_COUNTER instruction reached at PC = %d with args %d, %d",
			interpreter->program_counter,
			counter_id, amount);

	return INSTR_OK;
}

static enum instr_result handle_sub_from_counter(ncm_interpreter_t* interpreter) {
	u32 counter_id = interpreter->program[interpreter->program_counter].args[0];
	u32 amount = interpreter->program[interpreter->program_counter].args[1];

	sub_from_counter(&interpreter->counters, counter_id, amount);

	debug_print("SUB_FROM_COUNTER instruction reached at PC = %d with args %d, %d",
			interpreter->program_counter,
			counter_id, amount);

	return INSTR_OK;
}

static enum instr_result handle_set_counter(ncm_interpreter_t* interpreter) {
	u32 counter_id = interpreter->program[interpreter->program_counter].args[0];
	u32 value = interpreter->program[interpreter->program_counter].args[1];

	set_counter(&interpreter->counters, counter_id, value);

	debug_print("SET_COUNTER instruction reached at PC = %d with args %d, %d",
			interpreter->program_counter,
			counter_id, value);

	return INSTR_OK;
}

// args: variable id, message id
static enum instr_result handle_create(struct interpreter* interpreter) {
	u32 var_id = interpreter->program[interpreter->program_counter].args[0];
	u32 msg_id = interpreter->program[interpreter->program_counter].args[1];

	debug_print("CREATE instruction reached at PC = %d with args %d, %d",
			interpreter->program_counter,
			var_id, msg_id);

	ncm_create_message_from_var(&interpreter->network, &interpreter->variable_space, var_id, msg_id);

	return INSTR_OK;
}

// args: channel id, message id
static enum instr_result handle_send(struct interpreter* interpreter) {
	u32 chan_id = interpreter->program[interpreter->program_counter].args[0];
	u32 msg_id = interpreter->program[interpreter->program_counter].args[1];

	debug_print("SEND instruction reached at PC = %d with args %d, %d",
			interpreter->program_counter,
			chan_id, msg_id);

	if(ncm_send_message(&interpreter->network, chan_id, msg_id) >= 0) {
		return INSTR_OK;
	} else {
		return INSTR_ERROR;
	}
}

// args: channel id, variable id
static enum instr_result handle_receive(struct interpreter* interpreter) {
	u32 chan_id = interpreter->program[interpreter->program_counter].args[0];
	u32 var_id = interpreter->program[interpreter->program_counter].args[1];

	debug_print("RECEIVE instruction reached at PC = %d with args %d, %d",
			interpreter->program_counter,
			chan_id, var_id);

	if(ncm_receive_message_to_var(&interpreter->network, &interpreter->variable_space, chan_id, var_id) >= 0) {
		return INSTR_OK;
	} else {
		return INSTR_ERROR;
	}
}

// TODO: broadcast the sync instead of sending it on a specific channel
// args: SYNC_MASTER, channel OR SYNC_SLAVE, timeout (in some unknown unit?)
static enum instr_result handle_sync(struct interpreter* interpreter) {
	u32 sync_type = interpreter->program[interpreter->program_counter].args[0];
	u32 chan_id = interpreter->program[interpreter->program_counter].args[1];

	debug_print("SYNC instruction reached at PC = %d with args %d, %d",
			interpreter->program_counter,
			sync_type, chan_id);

	if(sync_type == SYNC_MASTER){
		ncm_send_sync(&interpreter->network, chan_id);
	} else {
		ncm_receive_sync(&interpreter->network, chan_id);
	}

	return INSTR_OK;
}
/*****************************************************************************
 * End interpreter instruction handlers
 *****************************************************************************/

/*
 * The interpreter main thread function
 */
static int interpreter_threadfn(void* data) {
	ncm_interpreter_t* interpreter = (ncm_interpreter_t*) data;
	enum instr_result instr_res;
	struct sched_param sparams;

	sparams.sched_priority = MAX_RT_PRIO-1;
	if(sched_setscheduler(interpreter->thread, SCHED_FIFO, &sparams) == -1) {
		return PROGRAM_ERROR;
	}

	debug_print("The interpreter is running!");
	debug_print("The program has %d instructions", interpreter->program_length);

	while(!kthread_should_stop()) {
		/* Handle the next instruction */
		switch( interpreter->program[interpreter->program_counter].type ) {
		case NOP:
			instr_res = handle_nop(interpreter);
			break;
		case GOTO:
			instr_res = handle_goto(interpreter);
			break;
		case FUTURE:
			instr_res = handle_future(interpreter);
			break;
		case HALT:
			instr_res = handle_halt(interpreter);
			break;
		case WAIT:
			instr_res = handle_wait(interpreter);
			break;
		case IF:
			instr_res = handle_if(interpreter);
			break;
		case CLEAR_COUNTER:
			instr_res = handle_clear_counter(interpreter);
			break;
		case ADD_TO_COUNTER:
			instr_res = handle_add_to_counter(interpreter);
			break;
		case SUB_FROM_COUNTER:
			instr_res = handle_sub_from_counter(interpreter);
			break;
		case SET_COUNTER:
			instr_res = handle_set_counter(interpreter);
			break;
		case CREATE:
			instr_res = handle_create(interpreter);
			break;
		case SEND:
			instr_res = handle_send(interpreter);
			break;
		case RECEIVE:
			instr_res = handle_receive(interpreter);
			break;
		case SYNC:
			instr_res = handle_sync(interpreter);
			break;
		case END_OF_PROGRAM:
			instr_res = handle_end_of_program(interpreter);
			return PROGRAM_OK;
		default:
			return INVALID_INSTR;
		}

		/* Handle the result of the instrution that just executed */
		switch(instr_res) {
		case INSTR_OK:
			++interpreter->program_counter;
			break;
		case INSTR_ERROR:
			debug_print("Instruction at PC = %d returned an error. Interpreter terminating.", interpreter->program_counter);
			return PROGRAM_ERROR;
		case INSTR_CHANGE_PC:
			continue;
		}
	}

	return PROGRAM_OK;
}


/*
 * Initializes the interpreter passed in.
 * This will create the thread the interpreter runs in
 * and set the program counter to 0.
 */
int init_interpreter(ncm_interpreter_t* interpreter) {
	interpreter->program_counter = 0;
	init_future_queue(&interpreter->future_queue);
	init_variable_space(&interpreter->variable_space);

	return 0;
}

/*
 * Initializes the interpreter passed in with the given program.
 * This will create and start a thread to run the interpreter in
 * and set the program counter to 0.
 */
int start_interpreter(ncm_interpreter_t* interpreter, ncm_program_t* program, ncm_interp_params_t* params) {
	interpreter->program_counter = 0;
	interpreter->program_length = program->length;
	interpreter->program = program->instructions;

	init_network(&interpreter->network, &params->network);
	interpreter->thread = kthread_run(interpreter_threadfn, (void*) interpreter, "NCM interpreter");

	return 0;
}

/*
 * Stops the interpreter thread and cleans up the interpreter
 */
int stop_interpreter(ncm_interpreter_t* interpreter) {
	destroy_network(&interpreter->network);
	kthread_stop(interpreter->thread);
	interpreter->thread = NULL;
	return 0;
}

bool is_running(ncm_interpreter_t* interpreter) {
	return interpreter->thread != NULL;
}




