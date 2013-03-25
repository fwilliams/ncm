/*
 * interpreter.c
 * The Network Code interpreter
 *
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */

#include "interpreter.h"
#include "guards.h"

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

enum instr_result handle_nop(struct interpreter* interpreter) {
	debug_print("NOP instruction reached at PC = %d\n", interpreter->program_counter);
	return INSTR_OK;
}

enum instr_result handle_future(struct interpreter* interpreter) {
	u32 delay = interpreter->program[interpreter->program_counter].args[0];
	u32 jmp = interpreter->program[interpreter->program_counter].args[1];
	int error;

	debug_print("FUTURE instruction reached at PC = %d with args %d and %d\n",
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

enum instr_result handle_halt(struct interpreter* interpreter) {
	struct future_queue_el head;
	int error;
	u64 range;

	debug_print("HALT instruction reached at PC = %d\n", interpreter->program_counter);

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

enum instr_result handle_wait(struct interpreter* interpreter) {
	enum instr_result res;
	res = handle_future(interpreter);

	if(res != INSTR_OK) {
		return res;
	}

	return handle_halt(interpreter);
}

enum instr_result handle_goto(struct interpreter* interpreter) {
	u32 jmp = interpreter->program[interpreter->program_counter].args[0];

	debug_print("GOTO instruction reached at PC = %d with arg %d\n",
			interpreter->program_counter, jmp);

	if(jmp >= interpreter->program_length) {
		return INSTR_ERROR;
	}

	interpreter->program_counter = jmp;

	return INSTR_CHANGE_PC;
}

enum instr_result handle_end_of_program(struct interpreter* interpreter) {
	debug_print("END_OF_PROGRAM instruction reached at PC = %d\n",
			interpreter->program_counter);
	return INSTR_OK;
}

enum instr_result handle_if(struct interpreter* interpreter) {
	u32 error;
	u32 guard = interpreter->program[interpreter->program_counter].args[0];
	u32 jmp = interpreter->program[interpreter->program_counter].args[1];
	u32* args = &interpreter->program[interpreter->program_counter].args[2];
	bool result = test_guard(interpreter, guard, args, &error);

	debug_print("IF instruction reached at PC = %d with args %d, %d\n",
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

enum instr_result handle_clear_counter(struct interpreter* interpreter) {
	u32 counter_id = interpreter->program[interpreter->program_counter].args[0];

	reset_counter(&interpreter->counters, counter_id);

	debug_print("CLEAR_COUNTER instruction reached at PC = %d with arg %d\n",
			interpreter->program_counter,
			counter_id);

	return INSTR_OK;
}

enum instr_result handle_add_to_counter(struct interpreter* interpreter) {
	u32 counter_id = interpreter->program[interpreter->program_counter].args[0];
	u32 amount = interpreter->program[interpreter->program_counter].args[1];

	add_to_counter(&interpreter->counters, counter_id, amount);

	debug_print("ADD_TO_COUNTER instruction reached at PC = %d with args %d, %d\n",
			interpreter->program_counter,
			counter_id, amount);

	return INSTR_OK;
}

enum instr_result handle_set_counter(struct interpreter* interpreter) {
	u32 counter_id = interpreter->program[interpreter->program_counter].args[0];
	u32 value = interpreter->program[interpreter->program_counter].args[1];

	set_counter(&interpreter->counters, counter_id, value);

	debug_print("SET_COUNTER instruction reached at PC = %d with args %d, %d\n",
			interpreter->program_counter,
			counter_id, value);

	return INSTR_OK;
}

/*****************************************************************************
 * End interpreter instruction handlers
 *****************************************************************************/

/*
 * The interpreter main thread function
 */
int interpreter_threadfn(void* data) {
	struct interpreter* interpreter = (struct interpreter*) data;
	enum instr_result instr_res;

	debug_print("The interpreter is running! \n");
	debug_print("The program has %d instructions\n", interpreter->program_length);

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
		case SET_COUNTER:
			instr_res = handle_set_counter(interpreter);
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
			return PROGRAM_ERROR;
		case INSTR_CHANGE_PC:
			continue;
		}
	}

	return PROGRAM_OK;
}

/*
 * Initializes the interpreter passed in with the given program.
 * This will create and start a thread to run the interpreter in
 * and set the program counter to 0.
 */
int start_interpreter(struct interpreter* interpreter, struct netcode_instr* prog, u8 prog_len) {
	interpreter->program_counter = 0;
	interpreter->program_length = prog_len;
	interpreter->program = prog;
	init_future_queue(&interpreter->future_queue);
	init_variable_space(&interpreter->variable_space);
	init_network(&interpreter->network);
	interpreter->thread = kthread_run(interpreter_threadfn, (void*) interpreter, "NCM interpreter");
	return 0;
}

/*
 * Stops the interpreter thread and cleans up the interpreter
 */
int stop_interpreter(struct interpreter* interpreter) {
	kthread_stop(interpreter->thread);
	destroy_network(&interpreter->network);
	return 0;
}




